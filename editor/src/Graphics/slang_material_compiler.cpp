#include <Core/definitions.hpp>

#if !PLATFORM_ANDROID
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/slang_material_compiler.hpp>
#include <cstring>

#define RETURN_ON_FAIL(code)                                                                                                     \
	if (SLANG_FAILED(code))                                                                                                      \
	return false

#define return_undefined_if_not(cond)                                                                                            \
	if (!(cond))                                                                                                                 \
	return ShaderParameterType::Undefined

#define check_compile_errors()                                                                                                   \
	if (log_handler.has_error)                                                                                                   \
	return false

#define return_if_false(cond)                                                                                                    \
	if (!(cond))                                                                                                                 \
	return

namespace Engine
{
	class CompileLogHandler : public Logger
	{
	public:
		Logger* base   = nullptr;
		bool has_error = false;

		CompileLogHandler()
		{
			base           = Logger::logger;
			Logger::logger = this;
		}

		~CompileLogHandler() { Logger::logger = base; }

		Logger& log_msg(const char* tag, const char* msg) override { return base->log_msg(tag, msg); }

		Logger& debug_msg(const char* tag, const char* msg) override { return base->debug_msg(tag, msg); }

		Logger& warning_msg(const char* tag, const char* msg) override { return base->warning_msg(tag, msg); }

		Logger& error_msg(const char* tag, const char* msg) override
		{
			if (std::strcmp(tag, "ShaderCompiler") == 0)
				has_error = true;
			return base->error_msg(tag, msg);
		}
	};

	static slang::IGlobalSession* global_session()
	{
		static Slang::ComPtr<slang::IGlobalSession> slang_global_session;
		if (slang_global_session.get() == nullptr)
		{
			if (SLANG_FAILED(slang::createGlobalSession(slang_global_session.writeRef())))
			{
				throw EngineException("Cannot create global session");
			}

			DestroyController().push([]() { slang_global_session = nullptr; });
		}

		return slang_global_session.get();
	}

	class ReflectionParser
	{
	private:
		struct VarTraceEntry {
			String name;
			slang::VariableLayoutReflection* var = nullptr;
			const VarTraceEntry* const prev      = nullptr;
			slang::TypeReflection::Kind kind;

			VarTraceEntry(slang::VariableLayoutReflection* const var, const VarTraceEntry* const prev = nullptr)
				: var(var), prev(prev), kind(var->getTypeLayout()->getKind())
			{
				name = Strings::make_string(var->getName());

				if (prev && !prev->name.empty())
				{
					name = Strings::format("{}.{}", prev->name, name);
				}
			}

			size_t trace_offset(SlangParameterCategory category) const
			{
				const VarTraceEntry* current = this;
				size_t result                = 0;

				if (var->getCategory() == slang::ParameterCategory::Uniform)
				{
					while (current)
					{
						result += current->var->getOffset(category);

						if (current->kind == slang::TypeReflection::Kind::ConstantBuffer)
						{
							break;
						}
						current = current->prev;
					}
				}
				else
				{
					while (current)
					{
						result += current->var->getOffset(category);
						current = current->prev;
					}
				}

				return result;
			}

			size_t trace_offset(slang::ParameterCategory category) const
			{
				return trace_offset(static_cast<SlangParameterCategory>(category));
			}

			slang::ParameterCategory category() const { return var->getCategory(); }
		};


	public:
		using TypeDetector = ShaderParameterType(slang::VariableLayoutReflection*, uint_t, uint_t, uint_t,
												 slang::TypeReflection::ScalarType);
		static Vector<TypeDetector*> type_detectors;

		Pipeline* pipeline;

		static bool has_attribute(slang::VariableReflection* var, const char* attribute, size_t args = 0)
		{
			auto count = var->getUserAttributeCount();
			for (unsigned int i = 0; i < count; ++i)
			{
				if (auto attrib = var->getUserAttributeByIndex(i))
				{
					if (std::strcmp(attrib->getName(), attribute) != 0)
						continue;

					if (attrib->getArgumentCount() != args)
						continue;

					return true;
				}
			}

			return false;
		}

		static ShaderParameterType find_parameter_type_from_attributes(slang::VariableReflection* var)
		{
			static Map<StringView, ShaderParameterType::Enum> map = {
					{"LocalToWorld", ShaderParameterType::LocalToWorld},      //
					{"Globals", ShaderParameterType::Globals},                //
					{"Surface", ShaderParameterType::Surface},                //
					{"CombinedSurface", ShaderParameterType::CombinedSurface},//
			};

			auto count = var->getUserAttributeCount();

			for (unsigned int i = 0; i < count; ++i)
			{
				if (auto attrib = var->getUserAttributeByIndex(i))
				{
					if (std::strcmp(attrib->getName(), "parameter_type") != 0)
						continue;

					if (attrib->getArgumentCount() != 1)
						continue;

					size_t size;
					const char* name = attrib->getArgumentValueString(0, &size);
					if (name && size > 2)
					{
						auto it = map.find(StringView(name + 1, size - 2));
						if (it != map.end())
							return it->second;
					}
				}
			}
			return ShaderParameterType::Undefined;
		}

		static bool find_semantic(String name, VertexBufferSemantic& out_semantic)
		{
			Strings::to_lower(name);

			static const TreeMap<String, VertexBufferSemantic> semantics = {
					{"position", VertexBufferSemantic::Position},       //
					{"texcoord", VertexBufferSemantic::TexCoord},       //
					{"color", VertexBufferSemantic::Color},             //
					{"normal", VertexBufferSemantic::Normal},           //
					{"tangent", VertexBufferSemantic::Tangent},         //
					{"bitangent", VertexBufferSemantic::Bitangent},     //
					{"blendweight", VertexBufferSemantic::BlendWeight}, //
					{"blendindices", VertexBufferSemantic::BlendIndices}//
			};

			auto it = semantics.find(name);
			if (it != semantics.end())
			{
				out_semantic = it->second;
				return true;
			}
			else
			{
				error_log("ShaderCompiler", "Failed to find semantic '%s'", name.c_str());
				return false;
			}
		}

		static VertexBufferElementType find_vertex_element_type(slang::TypeLayoutReflection* var, VertexBufferSemantic semantic)
		{
			if (var == nullptr)
				return VertexBufferElementType::Undefined;

			auto kind = var->getKind();

			if (kind == slang::TypeReflection::Kind::Scalar)
			{
				switch (var->getScalarType())
				{
					case slang::TypeReflection::ScalarType::Int8:
						return VertexBufferElementType::Byte1;

					case slang::TypeReflection::ScalarType::UInt8:
						return VertexBufferElementType::UByte1;

					case slang::TypeReflection::ScalarType::Int16:
						return VertexBufferElementType::Short1;

					case slang::TypeReflection::ScalarType::UInt16:
						return VertexBufferElementType::UShort1;

					case slang::TypeReflection::ScalarType::Int32:
						return VertexBufferElementType::Int1;

					case slang::TypeReflection::ScalarType::UInt32:
						return VertexBufferElementType::UInt1;

					case slang::TypeReflection::ScalarType::Float32:
						return VertexBufferElementType::Float1;

					default:
						return VertexBufferElementType::Undefined;
				}
			}
			else if (kind == slang::TypeReflection::Kind::Vector)
			{
				auto base_type         = find_vertex_element_type(var->getElementTypeLayout(), semantic);
				auto components_offset = var->getElementCount() - 1;

				if (components_offset > 3)
					return VertexBufferElementType::Undefined;

				if (components_offset == 3 &&
					!is_in<VertexBufferElementType::Float1, VertexBufferElementType::Int1, VertexBufferElementType::UInt1>(
							base_type))
				{
					--components_offset;
				}

				VertexBufferElementType result(
						static_cast<VertexBufferElementType::Enum>(static_cast<EnumerateType>(base_type) + components_offset));

				if (semantic == VertexBufferSemantic::Color && result == VertexBufferElementType::Float4)
					return VertexBufferElementType::Color;
				return result;
			}

			return VertexBufferElementType::Undefined;
		}

		bool parse_vertex_semantic(slang::VariableLayoutReflection* var)
		{
			auto kind     = var->getType()->getKind();
			auto category = var->getCategory();
			if (category != slang::ParameterCategory::VaryingInput)
				return true;

			if (kind == slang::TypeReflection::Kind::Struct)
			{
				auto layout       = var->getTypeLayout();
				auto fields_count = layout->getFieldCount();

				for (uint32_t field_index = 0; field_index < fields_count; ++field_index)
				{
					auto field = layout->getFieldByIndex(field_index);

					if (!parse_vertex_semantic(field))
					{
						return false;
					}
				}
			}
			else if (kind == slang::TypeReflection::Kind::Vector || kind == slang::TypeReflection::Kind::Scalar)
			{
				VertexAttribute attribute;

				const char* semantic_name = var->getSemanticName();

				if (semantic_name == nullptr)
				{
					error_log("ShaderCompiler", "Cannot find semantic for vertex input '%s'", var->getName());
					return false;
				}

				if (!find_semantic(var->getSemanticName(), attribute.semantic))
				{
					return false;
				}

				if (is_not_in<VertexBufferSemantic::Position, //
							  VertexBufferSemantic::TexCoord, //
							  VertexBufferSemantic::Color,    //
							  VertexBufferSemantic::Normal,   //
							  VertexBufferSemantic::Tangent,  //
							  VertexBufferSemantic::Bitangent,//
							  VertexBufferSemantic::BlendWeight>(attribute.semantic))
				{
					error_log("ShaderCompiler", "Semantic '%s' doesn't support vector type!", var->getSemanticName());
					return false;
				}

				attribute.semantic_index = var->getSemanticIndex();
				attribute.name           = var->getName();
				attribute.rate           = VertexAttributeInputRate::Vertex;
				attribute.type           = find_vertex_element_type(var->getTypeLayout(), attribute.semantic);
				attribute.location       = var->getBindingIndex();
				attribute.stream_index   = attribute.location;
				attribute.offset         = 0;
				Object::instance_cast<GraphicsPipeline>(pipeline)->vertex_shader()->attributes.push_back(attribute);
			}
			else
			{
				error_log("ShaderCompiler", "Unsupported input variable type!");
				return false;
			}

			return true;
		}

		static bool is_global_parameters(slang::TypeLayoutReflection* type)
		{
			return_if_false(type->getKind() == slang::TypeReflection::Kind::Struct) false;
			const char* name = type->getName();
			return_if_false(name != nullptr && std::strcmp(name, "GlobalParameters") == 0) false;
			return_if_false(sizeof(GlobalShaderParameters) == type->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM)) false;
			return true;
		}

		static ShaderParameterType find_scalar_parameter_type(slang::VariableLayoutReflection* var)
		{
			auto type = find_parameter_type_from_attributes(var->getVariable());

			if (type != ShaderParameterType::Undefined)
				return type;

			auto reflection = var->getType();
			auto rows       = reflection->getRowCount();
			auto colums     = reflection->getColumnCount();
			auto elements   = reflection->getElementCount();
			auto scalar     = reflection->getScalarType();

			for (auto& detector : type_detectors)
			{
				auto type = detector(var, rows, colums, elements, scalar);
				if (type != ShaderParameterType::Undefined)
				{
					return type;
				}
			}

			return ShaderParameterType::Undefined;
		}

		bool parse_shader_parameter(const VarTraceEntry& param)
		{
			if (is_in<slang::TypeReflection::Kind::Scalar, slang::TypeReflection::Kind::Vector,
					  slang::TypeReflection::Kind::Matrix>(param.kind))
			{
				ShaderParameterInfo info;
				info.type = find_scalar_parameter_type(param.var);

				if (info.type == ShaderParameterType::Undefined)
				{
					error_log("ShaderCompiler", "Failed to get parameter type!");
					return false;
				}

				if (auto layout = param.var->getTypeLayout())
				{
					info.size = layout->getSize();
				}
				else
				{
					error_log("ShaderCompiler", "Failed to get parameter layout info!");
					return false;
				}

				info.name                       = param.name;
				info.offset                     = param.trace_offset(slang::ParameterCategory::Uniform);
				info.location                   = param.trace_offset(slang::ParameterCategory::ConstantBuffer);
				pipeline->parameters[info.name] = info;
			}
			else if (is_in<slang::TypeReflection::Kind::Resource>(param.kind))
			{
				auto type = find_parameter_type_from_attributes(param.var->getVariable());

				if (auto type_layout = param.var->getTypeLayout())
				{
					SlangResourceShape shape = type_layout->getResourceShape();

					if (shape == SLANG_TEXTURE_2D)
					{
						auto binding_type = type_layout->getBindingRangeType(0);

						ShaderParameterInfo object;
						object.name     = param.name;
						object.location = param.trace_offset(param.category());
						object.type     = type;

						if (type == ShaderParameterType::Undefined)
						{
							switch (binding_type)
							{
								case slang::BindingType::CombinedTextureSampler:
									object.type = ShaderParameterType::Sampler2D;
									break;

								case slang::BindingType::Texture:
									object.type = ShaderParameterType::Texture2D;
									break;

								case slang::BindingType::MutableTexture:
									object.type = ShaderParameterType::RWTexture2D;
									break;

								default:
									return false;
							}
						}

						pipeline->parameters[object.name] = object;
					}
				}
			}
			else if (is_in<slang::TypeReflection::Kind::Struct>(param.kind))
			{
				auto layout = param.var->getTypeLayout();
				auto fields = layout->getFieldCount();

				for (decltype(fields) i = 0; i < fields; i++)
				{
					VarTraceEntry var(layout->getFieldByIndex(i), &param);
					return_if_false(parse_shader_parameter(var)) false;
				}
			}
			else if (is_in<slang::TypeReflection::Kind::ConstantBuffer>(param.kind))
			{
				auto layout = param.var->getTypeLayout()->getElementTypeLayout();

				if (is_global_parameters(layout))
				{
					ShaderParameterInfo object;
					object.name                       = param.name;
					object.location                   = param.trace_offset(slang::ParameterCategory::DescriptorTableSlot);
					object.type                       = ShaderParameterType::Globals;
					object.size                       = sizeof(GlobalShaderParameters);
					object.offset                     = 0;
					pipeline->parameters[object.name] = object;
				}
				else
				{
					auto fields = layout->getFieldCount();

					for (decltype(fields) i = 0; i < fields; i++)
					{
						VarTraceEntry var(layout->getFieldByIndex(i), &param);
						return_if_false(parse_shader_parameter(var)) false;
					}
				}
			}

			return true;
		}

		bool create_reflection(slang::ShaderReflection* reflection, Pipeline* out)
		{
			CompileLogHandler log_handler;
			pipeline = out;

			// Parse vertex attributes
			if (auto entry_point = reflection->findEntryPointByName("vs_main"))
			{
				uint32_t parameter_count = entry_point->getParameterCount();
				for (uint32_t i = 0; i < parameter_count; i++)
				{
					return_if_false(parse_vertex_semantic(entry_point->getParameterByIndex(i))) false;
				}
			}

			// Parse parameters

			if (auto layout = reflection->getGlobalParamsVarLayout())
			{
				VarTraceEntry var(layout);
				return_if_false(parse_shader_parameter(var)) false;
			}

			out = nullptr;
			return log_handler.has_error == false;
		}
	};

	Vector<ReflectionParser::TypeDetector*> ReflectionParser::type_detectors;

	struct TypeDetector {
		using Scalar = slang::TypeReflection::ScalarType;
		using SVR    = slang::VariableReflection;
		using SVLR   = slang ::VariableLayoutReflection;

		template<ShaderParameterType type, Scalar required_scalar>
		static ShaderParameterType primitive(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == 1);
			return_undefined_if_not(elements == 0);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<ShaderParameterType type, uint32_t len, Scalar required_scalar>
		static ShaderParameterType vector(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == len);
			return_undefined_if_not(elements == len);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<ShaderParameterType type, Scalar required_scalar, uint_t required_rows, uint_t required_columns>
		static ShaderParameterType matrix(SVLR* var, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_undefined_if_not(rows == required_rows);
			return_undefined_if_not(rows == required_rows);
			return_undefined_if_not(columns == required_columns);
			return_undefined_if_not(elements == 0);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}
	};

	static void setup_detectors()
	{
		using T      = TypeDetector;
		using Scalar = slang::TypeReflection::ScalarType;
		using MP     = ShaderParameterType;

		ReflectionParser::type_detectors.push_back(T::primitive<MP::Bool, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::Int, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::UInt, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::Float, Scalar::Float32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool2, 2, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool3, 3, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool4, 4, Scalar::Bool>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Int2, 2, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Int3, 3, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Int4, 4, Scalar::Int32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt2, 2, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt3, 3, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt4, 4, Scalar::UInt32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Float2, 2, Scalar::Float32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Float3, 3, Scalar::Float32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Float4, 4, Scalar::Float32>);

		ReflectionParser::type_detectors.push_back(T::matrix<MP::Float3x3, Scalar::Float32, 3, 3>);
		ReflectionParser::type_detectors.push_back(T::matrix<MP::Float4x4, Scalar::Float32, 4, 4>);
	}

	static PreInitializeController preinit(setup_detectors);

	trinex_implement_class_default_init(Engine::SLANG_MaterialCompiler, 0);
	trinex_implement_class_default_init(Engine::OPENGL_MaterialCompiler, 0);
	trinex_implement_class_default_init(Engine::VULKAN_MaterialCompiler, 0);
	trinex_implement_class_default_init(Engine::NONE_MaterialCompiler, 0);
	trinex_implement_class_default_init(Engine::D3D11_MaterialCompiler, 0);

	SLANG_MaterialCompiler::Context::Context(SLANG_MaterialCompiler* compiler) : compiler(compiler), prev_ctx(compiler->m_ctx)
	{
		compiler->m_ctx = this;
	}

	bool SLANG_MaterialCompiler::Context::initialize(const String& source)
	{
		slang::SessionDesc session_desc      = {};
		session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		session_desc.allowGLSLSyntax         = false;
		session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

		if (SLANG_FAILED(global_session()->createSession(session_desc, session.writeRef())))
		{
			throw EngineException("Failed to create slang session");
		}

		if (SLANG_FAILED(session->createCompileRequest(compile_request.writeRef())))
		{
			error_log("MaterialCompiler", "Failed to create slang compile request");
			return false;
		}

		for (auto& include_dir : compiler->m_include_directories)
		{
			compile_request->addSearchPath(include_dir.c_str());
		}

		unit = compile_request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "main_unit");
		compile_request->addTranslationUnitSourceString(unit, "main_unit_source", source.c_str());

		compiler->initialize_context();
		return true;
	}

	bool SLANG_MaterialCompiler::Context::compile(ShaderInfo* infos, size_t infos_len, Pipeline* pipeline, CheckStages checker)
	{
		// Compile source
		auto compile_result = compile_request->compile();

		if (SLANG_FAILED(compile_result))
		{
			if (auto diagnostics = compile_request->getDiagnosticOutput())
			{
				if (strlen(diagnostics) > 0)
				{
					error_log("ShaderCompiler", diagnostics);
				}
			}

			return false;
		}
		else if (auto diagnostics = compile_request->getDiagnosticOutput())
		{
			if (strlen(diagnostics) > 0)
			{
				warn_log("ShaderCompiler", diagnostics);
			}
		}

		Vector<slang::IComponentType*> component_types;
		Slang::ComPtr<slang::IModule> slang_module;

		compile_request->getModule(unit, slang_module.writeRef());
		component_types.push_back(slang_module);

		for (size_t i = 0; i < infos_len; ++i)
		{
			auto& info = infos[i];

			slang_module->findEntryPointByName(info.entry_name, info.entry.writeRef());

			if (info.entry)
			{
				info.index = component_types.size() - 1;
				component_types.push_back(info.entry);
			}
		}

		if (!checker(infos))
			return false;

		Slang::ComPtr<slang::IComponentType> composite;
		{
			Slang::ComPtr<slang::IBlob> diagnostics_blob;
			SlangResult result = session->createCompositeComponentType(component_types.data(), component_types.size(),
																	   composite.writeRef(), diagnostics_blob.writeRef());

			if (diagnostics_blob != nullptr)
			{
				error_log("MaterialCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
			}

			if (SLANG_FAILED(result))
				return false;
		}

		Slang::ComPtr<slang::IComponentType> program;
		{
			Slang::ComPtr<slang::IBlob> diagnostics_blob;
			SlangResult result = composite->link(program.writeRef(), diagnostics_blob.writeRef());

			if (diagnostics_blob != nullptr)
			{
				error_log("MaterialCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
			}

			if (SLANG_FAILED(result))
				return false;
		}

		for (size_t i = 0; i < infos_len; ++i)
		{
			auto& info = infos[i];

			if (info.index != -1)
			{
				Slang::ComPtr<slang::IBlob> result_code;
				Slang::ComPtr<slang::IBlob> diagnostics_blob;

				SlangResult result =
						program->getEntryPointCode(info.index, 0, result_code.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
				{
					error_log("MaterialCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
				}

				if (SLANG_FAILED(result))
					return false;

				if (auto shader = pipeline->shader(info.type, true))
				{
					const byte* src = static_cast<const byte*>(result_code->getBufferPointer());
					compiler->submit_source(shader, src, result_code->getBufferSize());
				}
				else
				{
					error_log("MaterialCompiler", "Failed to create shader");
					return false;
				}
			}
		}

		// Generate reflection
		{
			Slang::ComPtr<slang::IBlob> diagnostics_blob;
			slang::ProgramLayout* reflection = program->getLayout(0, diagnostics_blob.writeRef());

			if (diagnostics_blob != nullptr)
			{
				error_log("MaterialCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
			}

			if (!reflection)
			{
				error_log("ShaderCompiler", "Failed to get shader reflection!");
				return false;
			}

			ReflectionParser parser;

			if (!parser.create_reflection(reflection, pipeline))
				return false;
		}

		return pipeline;
	}

	bool SLANG_MaterialCompiler::Context::compile_graphics(Material* material, Refl::RenderPassInfo* pass)
	{
		if (material)
		{
			for (auto& definition : material->compile_definitions)
			{
				add_definition(definition);
			}
		}

		if (pass)
		{
			for (auto& definition : pass->shader_definitions())
			{
				add_definition(definition);
			}
		}

		Pipeline* pipeline      = material->remove_pipeline(pass);
		const bool new_pipeline = pipeline == nullptr;

		if (new_pipeline)
		{
			String name = pass ? pass->name() : "Default";
			pipeline    = new_instance<GraphicsPipeline>(name);
		}
		else
		{
			pipeline->clear();
		}

		if (compile_graphics(pipeline))
		{
			material->add_pipeline(pass, pipeline);
			pipeline->init_render_resources();
			return true;
		}

		if (new_pipeline)
		{
			GarbageCollector::destroy(pipeline);
		}

		return false;
	}

	bool SLANG_MaterialCompiler::Context::compile_graphics(Pipeline* pipeline)
	{
		ShaderInfo shader_infos[] = {
				{ShaderType::Vertex, "vs_main", {}, -1},              //
				{ShaderType::TessellationControl, "tsc_main", {}, -1},//
				{ShaderType::Tessellation, "ts_main", {}, -1},        //
				{ShaderType::Geometry, "gs_main", {}, -1},            //
				{ShaderType::Fragment, "fs_main", {}, -1},            //
		};

		CheckStages checker = [](ShaderInfo* infos) -> bool {
			bool result = infos[0].index != -1 && infos[4].index != -1;
			if (!result)
				error_log("MaterialCompiler", "Graphics pipeline is not valid!");
			return result;
		};

		return compile(shader_infos, 5, pipeline, checker);
	}

	bool SLANG_MaterialCompiler::Context::compile_compute(Pipeline* pipeline)
	{
		ShaderInfo shader_infos[] = {{ShaderType::Compute, "cs_main", {}, -1}};

		CheckStages checker = [](ShaderInfo* infos) -> bool {
			bool result = infos[0].index != -1;
			if (!result)
				error_log("MaterialCompiler", "Compute pipeline is not valid!");
			return result;
		};

		return compile(shader_infos, 1, pipeline, checker);
	}

	SLANG_MaterialCompiler::Context::~Context()
	{
		compiler->m_ctx = prev_ctx;
	}

	SLANG_MaterialCompiler::SLANG_MaterialCompiler()
	{
		flags(StandAlone, true);
		flags(IsAvailableForGC, false);

		m_include_directories = {
				rootfs()->native_path(Project::shaders_dir),
				rootfs()->native_path("[shaders_dir]:/TrinexEditor"),
				rootfs()->native_path("[shaders_dir]:/TrinexEngine"),
		};
	}

	void SLANG_MaterialCompiler::initialize_context()
	{
		m_ctx->compile_request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
	}

	bool SLANG_MaterialCompiler::compile(Material* material)
	{
		String source;

		material->remove_all_pipelines();

		if (!material->shader_source(source))
		{
			error_log("MaterialCompiler", "Failed to get shader source");
			return false;
		}

		bool success = true;

		if (!(material->options & MaterialOptions::DisableDefaultPass))
			success = compile_pass(material, nullptr, source);

		for (auto pass = Refl::RenderPassInfo::first_pass(); pass && success; pass = pass->next_pass())
		{
			if (pass->is_material_compatible(material))
			{
				success = compile_pass(material, pass, source);
			}
		}

		material->remove_unreferenced_parameters();
		return success;
	}

	void SLANG_MaterialCompiler::submit_source(Shader* shader, const byte* src, size_t size)
	{
		Buffer& out = shader->source_code;
		std::destroy_at(&out);
		new (&out) Buffer(src, src + size);
	}

	bool SLANG_MaterialCompiler::compile_pass(Material* material, Refl::RenderPassInfo* pass)
	{
		String source;

		if (!material->shader_source(source))
		{
			error_log("MaterialCompiler", "Failed to get shader source");
			return false;
		}

		bool result = compile_pass(material, pass, source);
		material->remove_unreferenced_parameters();
		return result;
	}

	bool SLANG_MaterialCompiler::compile_pass(Material* material, Refl::RenderPassInfo* pass, const String& source)
	{
		Context ctx(this);

		if (!ctx.initialize(source))
			return false;

		return ctx.compile_graphics(material, pass);
	}

	bool SLANG_MaterialCompiler::compile(const String& source, Pipeline* pipeline)
	{
		Context ctx(this);

		if (!ctx.initialize(source))
			return false;

		if (pipeline->is_instance_of<GraphicsPipeline>())
		{
			if (ctx.compile_graphics(pipeline))
			{
				pipeline->init_render_resources();
				return true;
			}
		}
		else if (pipeline->is_instance_of<ComputePipeline>())
		{
			if (ctx.compile_compute(pipeline))
			{
				pipeline->init_render_resources();
				return true;
			}
		}
		return false;
	}

	void NONE_MaterialCompiler::initialize_context()
	{
		Super::initialize_context();
	}

	void OPENGL_MaterialCompiler::initialize_context()
	{
		Super::initialize_context();
		auto& request = m_ctx->compile_request;

		request->setCodeGenTarget(SLANG_SPIRV);
		request->addPreprocessorDefine("TRINEX_OPENGL_RHI", "1");

		request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
		request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_NONE);
		request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);

		auto profile = global_session()->findProfile("glsl_330");
		request->setTargetProfile(0, profile);

		const char* argument = "-emit-spirv-via-glsl";
		request->processCommandLineArguments(&argument, 1);// TODO: Maybe it can be optimized to avoid parsing arguments?
	}

	void OPENGL_MaterialCompiler::submit_source(Shader* shader, const byte* src, size_t size)
	{
		spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t*>(src), size / 4);
		spirv_cross::ShaderResources resources = glsl.get_shader_resources();

		for (auto& resource : resources.sampled_images)
		{
			unsigned set     = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
			unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
			glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
			glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
		}

		// Set some options.
		spirv_cross::CompilerGLSL::Options options;
		options.version = 310;
		options.es      = true;
		glsl.set_common_options(options);

		String glsl_code = glsl.compile();
		Super::submit_source(shader, reinterpret_cast<const byte*>(glsl_code.data()), glsl_code.size() + 1);
	}

	void VULKAN_MaterialCompiler::initialize_context()
	{
		Super::initialize_context();
		auto& request = m_ctx->compile_request;

		request->setCodeGenTarget(SLANG_SPIRV);
		request->addPreprocessorDefine("TRINEX_VULKAN_RHI", "1");

		Vector<const char*> arguments;

		if (Settings::debug_shaders)
		{
			request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);
			request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_NONE);
			request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_STANDARD);

			auto profile = global_session()->findProfile("spirv_1_3");
			request->setTargetProfile(0, profile);

			arguments.push_back("-emit-spirv-directly");
		}
		else
		{
			request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
			request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_NONE);
			request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);

			auto profile = global_session()->findProfile("glsl_330");
			request->setTargetProfile(0, profile);

			arguments.push_back("-emit-spirv-via-glsl");
		}

		request->processCommandLineArguments(arguments.data(),
											 arguments.size());// TODO: Maybe it can be optimized to avoid parsing arguments?
	}

	void D3D11_MaterialCompiler::initialize_context()
	{
		Super::initialize_context();
		auto& request = m_ctx->compile_request;

		request->setCodeGenTarget(SLANG_DXBC);
		request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);
		request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);

		if (Settings::debug_shaders)
			request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
		else
			request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_NONE);

		auto profile = global_session()->findProfile("sm_5_0");
		request->setTargetProfile(0, profile);
		request->addPreprocessorDefine("TRINEX_INVERT_UV", "1");
		request->addPreprocessorDefine("TRINEX_D3D11_RHI", "1");
		request->addPreprocessorDefine("TRINEX_DIRECT_X_RHI", "1");
	}
}// namespace Engine

#endif
