#include <Core/definitions.hpp>

#if !PLATFORM_ANDROID
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/shader_compiler.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <cstring>
#include <slang-com-ptr.h>
#include <slang.h>
#include <spirv_glsl.hpp>

#define RETURN_ON_FAIL(code)                                                                                                     \
	if (SLANG_FAILED(code))                                                                                                      \
	return false

#define return_nullptr_if_not(cond)                                                                                              \
	if (!(cond))                                                                                                                 \
	return nullptr

#define check_compile_errors()                                                                                                   \
	if (log_handler.has_error)                                                                                                   \
	return false

#define return_if_false(cond)                                                                                                    \
	if (!(cond))                                                                                                                 \
	return

namespace Engine::ShaderCompiler
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

		~CompileLogHandler()
		{
			Logger::logger = base;
		}

		Logger& log_msg(const char* tag, const char* msg) override
		{
			return base->log_msg(tag, msg);
		}

		Logger& debug_msg(const char* tag, const char* msg) override
		{
			return base->debug_msg(tag, msg);
		}

		Logger& warning_msg(const char* tag, const char* msg) override
		{
			return base->warning_msg(tag, msg);
		}

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
			slang::VariableLayoutReflection* var = nullptr;
			const VarTraceEntry* const prev      = nullptr;

			VarTraceEntry(slang::VariableLayoutReflection* const var, const VarTraceEntry* const prev = nullptr)
				: var(var), prev(prev)
			{}

			size_t trace_offset(SlangParameterCategory category) const
			{
				const VarTraceEntry* current = this;
				size_t result                = 0;

				while (current)
				{
					auto variable = current->var;

					for (unsigned int i = 0, count = variable->getCategoryCount(); i < count; ++i)
					{
						if (variable->getCategoryByIndex(i) == static_cast<slang::ParameterCategory>(category))
						{
							result += current->var->getOffset(category);
							break;
						}
					}

					current = current->prev;
				}

				return result;
			}

			const VarTraceEntry* firts_node() const
			{
				const VarTraceEntry* node = this;
				while (node->prev) node = node->prev;
				return node;
			}

			BindingIndex binding_index() const
			{
				return firts_node()->var->getBindingIndex();
			}

			size_t trace_offset(slang::ParameterCategory category) const
			{
				return trace_offset(static_cast<SlangParameterCategory>(category));
			}
		};


	public:
		using TypeDetector = Refl::Class*(slang::VariableLayoutReflection*, uint_t, uint_t, uint_t,
										  slang::TypeReflection::ScalarType);
		static Vector<TypeDetector*> type_detectors;

		ShaderReflection out;

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

				auto result = static_cast<VertexBufferElementType>(static_cast<EnumerateType>(base_type) + components_offset);

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
				ShaderReflection::VertexAttribute attribute;

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
				out.attributes.push_back(attribute);
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
			return_if_false(std::strcmp(type->getName(), "GlobalParameters") == 0) false;
			return_if_false(sizeof(GlobalShaderParameters) == type->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM)) false;
			return true;
		}

		static Refl::Class* find_scalar_parameter_type(slang::VariableLayoutReflection* var)
		{
			auto reflection = var->getType();
			auto rows       = reflection->getRowCount();
			auto colums     = reflection->getColumnCount();
			auto elements   = reflection->getElementCount();
			auto scalar     = reflection->getScalarType();

			for (auto& detector : type_detectors)
			{
				if (auto type = detector(var, rows, colums, elements, scalar))
				{
					return type;
				}
			}

			return nullptr;
		}

		bool parse_shader_parameter(const VarTraceEntry& param)
		{
			auto kind = param.var->getTypeLayout()->getKind();

			if (is_in<slang::TypeReflection::Kind::Scalar, slang::TypeReflection::Kind::Vector,
					  slang::TypeReflection::Kind::Matrix>(kind))
			{
				auto name = param.var->getName();
				trinex_always_check(name, "Failed to get parameter name!");
				MaterialParameterInfo info;
				info.type = find_scalar_parameter_type(param.var);

				if (info.type == nullptr)
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

				info.name     = name;
				info.offset   = param.trace_offset(SLANG_PARAMETER_CATEGORY_UNIFORM);
				info.location = param.binding_index();
				out.uniform_member_infos.push_back(info);
			}
			else if (is_in<slang::TypeReflection::Kind::Resource>(kind))
			{
				if (auto type_layout = param.var->getTypeLayout())
				{
					SlangResourceShape shape = type_layout->getResourceShape();

					if (shape == SLANG_TEXTURE_2D)
					{
						auto binding_type = type_layout->getBindingRangeType(0);

						MaterialParameterInfo object;
						object.name     = param.var->getName();
						object.location = param.binding_index();
						object.type     = binding_type == slang::BindingType::CombinedTextureSampler
												  ? MaterialParameters::Sampler2D::static_class_instance()
												  : MaterialParameters::Texture2D::static_class_instance();
						out.uniform_member_infos.push_back(object);
					}
				}
			}
			else if (is_in<slang::TypeReflection::Kind::Struct>(kind))
			{
				auto layout = param.var->getTypeLayout();
				auto fields = layout->getFieldCount();

				for (decltype(fields) i = 0; i < fields; i++)
				{
					VarTraceEntry var(layout->getFieldByIndex(i), &param);
					return_if_false(parse_shader_parameter(var)) false;
				}
			}
			else if (is_in<slang::TypeReflection::Kind::ConstantBuffer>(kind))
			{
				auto layout = param.var->getTypeLayout()->getElementTypeLayout();

				if (is_global_parameters(layout))
				{
					MaterialParameterInfo object;
					object.name     = param.var->getName();
					object.location = param.binding_index();
					object.type     = MaterialParameters::Globals::static_class_instance();
					object.size     = sizeof(GlobalShaderParameters);
					object.offset   = 0;
					out.uniform_member_infos.push_back(object);
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

		bool create_reflection(slang::ShaderReflection* reflection)
		{
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
			int count = reflection->getParameterCount();

			for (int i = 0; i < count; i++)
			{
				VarTraceEntry var(reflection->getParameterByIndex(i));
				return_if_false(parse_shader_parameter(var)) false;
			}

			return true;
		}
	};

	Vector<ReflectionParser::TypeDetector*> ReflectionParser::type_detectors;

	struct TypeDetector {
		using Scalar = slang::TypeReflection::ScalarType;
		using SVR    = slang::VariableReflection;
		using SVLR   = slang ::VariableLayoutReflection;

		static bool has_model_attribute(SVR* var)
		{
			return ReflectionParser::has_attribute(var, "is_model");
		}

		template<typename Type, Scalar required_scalar>
		static Refl::Class* primitive(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_nullptr_if_not(rows == 1);
			return_nullptr_if_not(columns == 1);
			return_nullptr_if_not(elements == 0);
			return_nullptr_if_not(scalar == required_scalar);
			return Type::static_class_instance();
		}

		template<typename Type, Scalar required_scalar>
		static Refl::Class* vector(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			auto len = static_cast<uint_t>(decltype(Type::value)::length());
			return_nullptr_if_not(rows == 1);
			return_nullptr_if_not(columns == len);
			return_nullptr_if_not(elements == len);
			return_nullptr_if_not(scalar == required_scalar);
			return Type::static_class_instance();
		}

		template<typename Type, Scalar required_scalar, uint_t required_rows, uint_t required_columns, bool allow_model = false>
		static Refl::Class* matrix(SVLR* var, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_nullptr_if_not(!has_model_attribute(var->getVariable()) || allow_model);
			return_nullptr_if_not(rows == required_rows);
			return_nullptr_if_not(rows == required_rows);
			return_nullptr_if_not(columns == required_columns);
			return_nullptr_if_not(elements == 0);
			return_nullptr_if_not(scalar == required_scalar);
			return Type::static_class_instance();
		}
	};

	static void setup_detectors()
	{
		using T      = TypeDetector;
		using Scalar = slang::TypeReflection::ScalarType;
		namespace MP = MaterialParameters;

		ReflectionParser::type_detectors.push_back(T::primitive<MP::Bool, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::Int, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::UInt, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::primitive<MP::Float, Scalar::Float32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool2, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool3, Scalar::Bool>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Bool4, Scalar::Bool>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Int2, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Int3, Scalar::Int32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Int4, Scalar::Int32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt2, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt3, Scalar::UInt32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::UInt4, Scalar::UInt32>);

		ReflectionParser::type_detectors.push_back(T::vector<MP::Float2, Scalar::Float32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Float3, Scalar::Float32>);
		ReflectionParser::type_detectors.push_back(T::vector<MP::Float4, Scalar::Float32>);

		ReflectionParser::type_detectors.push_back(T::matrix<MP::Float3x3, Scalar::Float32, 3, 3>);
		ReflectionParser::type_detectors.push_back(T::matrix<MP::Float4x4, Scalar::Float32, 4, 4>);
		ReflectionParser::type_detectors.push_back(T::matrix<MP::Model4x4, Scalar::Float32, 4, 4, true>);
	}

	static PreInitializeController preinit(setup_detectors);


	static void host_setup_request(SlangCompileRequest* request, const Vector<ShaderDefinition>& definitions)
	{
		Path shaders_dir = rootfs()->native_path(Project::shaders_dir);

		auto engine_path = rootfs()->native_path("[shaders_dir]:/TrinexEditor");
		auto editor_path = rootfs()->native_path("[shaders_dir]:/TrinexEngine");

		request->addSearchPath(engine_path.c_str());
		request->addSearchPath(editor_path.c_str());
		request->addSearchPath(shaders_dir.c_str());

		for (const auto& definition : definitions)
		{
			request->addPreprocessorDefine(definition.key.c_str(), definition.value.c_str());
		}

		request->setMatrixLayoutMode(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
		request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
	}

	static void submit_compiled_source(Buffer& out_buffer, const void* _data, size_t size)
	{
		std::destroy_at(&out_buffer);
		const byte* data = reinterpret_cast<const byte*>(_data);
		new (&out_buffer) Buffer(data, data + size);
	}

	struct RequestSetupInterface {
		virtual void setup(SlangCompileRequest*) const = 0;
	};

	using SetupRequestFunction = void (*)(SlangCompileRequest*);

	static bool compile_shader(const String& source, const Vector<ShaderDefinition>& definitions, ShaderSource& out_source,
							   const RequestSetupInterface* setup_request)
	{
		CompileLogHandler log_handler;

		auto diagnose_if_needed = [](slang::IBlob* diagnostics_blob) {
			if (diagnostics_blob != nullptr)
			{
				error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
			}
		};

		using Slang::ComPtr;

		slang::SessionDesc session_desc      = {};
		session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		session_desc.allowGLSLSyntax         = false;


		ComPtr<slang::ISession> session;
		RETURN_ON_FAIL(global_session()->createSession(session_desc, session.writeRef()));
		Vector<slang::IComponentType*> component_types = {};

		// Compile current module
		ComPtr<slang::IModule> slang_module;

		int_t current_entry_index = 0;

		int_t vertex_entry_index         = -1;
		int_t tessellation_control_index = -1;
		int_t tessellation_index         = -1;
		int_t geometry_index             = -1;
		int_t fragment_entry_index       = -1;


		{
			ComPtr<SlangCompileRequest> request;
			session->createCompileRequest(request.writeRef());

			if (!request)
			{
				error_log("ShaderCompiler", "Failed to create compile request");
				return {};
			}

			host_setup_request(request, definitions);
			if (setup_request)
			{
				setup_request->setup(request);
			}

			auto unit = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "main_unit");
			request->addTranslationUnitSourceString(unit, "main_unit_source", source.c_str());

			auto compile_result = request->compile();

			if (SLANG_FAILED(compile_result))
			{
				if (auto diagnostics = request->getDiagnosticOutput())
				{
					if (strlen(diagnostics) > 0)
					{
						error_log("ShaderCompiler", diagnostics);
					}
				}

				return {};
			}
			else if (auto diagnostics = request->getDiagnosticOutput())
			{
				if (strlen(diagnostics) > 0)
				{
					warn_log("ShaderCompiler", diagnostics);
				}
			}

			request->getModule(unit, slang_module.writeRef());
			component_types.push_back(slang_module);
		}

		ComPtr<slang::IEntryPoint> vertex_entry_point;
		{
			slang_module->findEntryPointByName("vs_main", vertex_entry_point.writeRef());

			if (!vertex_entry_point)
			{
				error_log("ShaderCompiler", "Failed to find vs_main. Skipping!");
			}
			else
			{
				component_types.push_back(vertex_entry_point);
				vertex_entry_index = current_entry_index++;
			}
		}

		check_compile_errors();

		ComPtr<slang::IEntryPoint> fragment_entry_point;
		{
			slang_module->findEntryPointByName("fs_main", fragment_entry_point.writeRef());

			if (!fragment_entry_point)
			{
				error_log("ShaderCompiler", "Failed to find fs_main. Skipping compiling fragment code");
			}
			else
			{
				component_types.push_back(fragment_entry_point);
				fragment_entry_index = current_entry_index++;
			}
		}

		check_compile_errors();


		ComPtr<slang::IEntryPoint> tessellation_control_entry_point;
		{
			slang_module->findEntryPointByName("tsc_main", tessellation_control_entry_point.writeRef());

			if (tessellation_control_entry_point)
			{
				component_types.push_back(tessellation_control_entry_point);
				tessellation_control_index = current_entry_index++;
			}
		}

		ComPtr<slang::IEntryPoint> tessellation_entry_point;
		{
			slang_module->findEntryPointByName("ts_main", tessellation_entry_point.writeRef());

			if (tessellation_entry_point)
			{
				component_types.push_back(tessellation_entry_point);
				tessellation_index = current_entry_index++;
			}
		}

		ComPtr<slang::IEntryPoint> geometry_entry_point;
		{
			slang_module->findEntryPointByName("gs_main", geometry_entry_point.writeRef());

			if (geometry_entry_point)
			{
				component_types.push_back(geometry_entry_point);
				geometry_index = current_entry_index++;
			}
		}

		ComPtr<slang::IComponentType> composite;
		{
			ComPtr<slang::IBlob> diagnostics_blob;
			SlangResult result = session->createCompositeComponentType(component_types.data(), component_types.size(),
																	   composite.writeRef(), diagnostics_blob.writeRef());
			diagnose_if_needed(diagnostics_blob);
			RETURN_ON_FAIL(result);
		}

		ComPtr<slang::IComponentType> program;
		{
			ComPtr<slang::IBlob> diagnostics_blob;
			SlangResult result = composite->link(program.writeRef(), diagnostics_blob.writeRef());
			diagnose_if_needed(diagnostics_blob);
			RETURN_ON_FAIL(result);
		}

		{
			ComPtr<slang::IBlob> diagnostics_blob;
			slang::ProgramLayout* reflection = program->getLayout(0, diagnostics_blob.writeRef());
			diagnose_if_needed(diagnostics_blob);
			if (!reflection)
			{
				error_log("ShaderCompiler", "Failed to get shader reflection!");
				return {};
			}

			ReflectionParser parser;
			if (!parser.create_reflection(reflection))
				return false;
			out_source.reflection = parser.out;
			check_compile_errors();
		}

		if (vertex_entry_point)
		{
			ComPtr<slang::IBlob> result_code;
			{
				ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result =
						program->getEntryPointCode(vertex_entry_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
				diagnose_if_needed(diagnostics_blob);
				RETURN_ON_FAIL(result);

				submit_compiled_source(out_source.vertex_code, result_code->getBufferPointer(), result_code->getBufferSize());
			}
		}

		if (fragment_entry_point)
		{
			ComPtr<slang::IBlob> result_code;
			{
				ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result =
						program->getEntryPointCode(fragment_entry_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
				diagnose_if_needed(diagnostics_blob);
				RETURN_ON_FAIL(result);

				submit_compiled_source(out_source.fragment_code, result_code->getBufferPointer(), result_code->getBufferSize());
			}
		}

		if (tessellation_control_entry_point)
		{
			ComPtr<slang::IBlob> result_code;
			{
				ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result = program->getEntryPointCode(tessellation_control_index, 0, result_code.writeRef(),
																diagnostics_blob.writeRef());
				diagnose_if_needed(diagnostics_blob);
				RETURN_ON_FAIL(result);

				submit_compiled_source(out_source.tessellation_control_code, result_code->getBufferPointer(),
									   result_code->getBufferSize());
			}
		}

		if (tessellation_entry_point)
		{
			ComPtr<slang::IBlob> result_code;
			{
				ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result =
						program->getEntryPointCode(tessellation_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
				diagnose_if_needed(diagnostics_blob);
				RETURN_ON_FAIL(result);

				submit_compiled_source(out_source.tessellation_code, result_code->getBufferPointer(),
									   result_code->getBufferSize());
			}
		}

		if (geometry_entry_point)
		{
			ComPtr<slang::IBlob> result_code;
			{
				ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result =
						program->getEntryPointCode(geometry_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
				diagnose_if_needed(diagnostics_blob);
				RETURN_ON_FAIL(result);

				submit_compiled_source(out_source.geometry_code, result_code->getBufferPointer(), result_code->getBufferSize());
			}
		}

		return true;
	}

	static std::vector<uint32_t> to_spirv_buffer(const Buffer& spirv)
	{
		const uint32_t* data = reinterpret_cast<const uint32_t*>(spirv.data());
		const uint32_t size  = spirv.size() / 4;
		return std::vector<uint32_t>(data, data + size);
	}

	static void compile_spirv_to_glsl_es(Buffer& code)
	{
		if (code.empty())
			return;

		std::vector<uint32_t> spirv_binary = to_spirv_buffer(code);
		spirv_cross::CompilerGLSL glsl(std::move(spirv_binary));
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
		submit_compiled_source(code, glsl_code.data(), glsl_code.size() + 1);
	}

	struct VulkanRequestSetup : RequestSetupInterface {
		void setup(SlangCompileRequest* request) const override
		{
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
	};

	struct OpenGLRequestSetup : RequestSetupInterface {
		void setup(SlangCompileRequest* request) const override
		{
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
	};

	struct D3D11RequestSetup : RequestSetupInterface {
		void setup(SlangCompileRequest* request) const override
		{
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
	};

	implement_class_default_init(Engine::ShaderCompiler::OPENGL_Compiler, 0);
	implement_class_default_init(Engine::ShaderCompiler::VULKAN_Compiler, 0);
	implement_class_default_init(Engine::ShaderCompiler::NONE_Compiler, 0);
	implement_class_default_init(Engine::ShaderCompiler::D3D11_Compiler, 0);

	bool OPENGL_Compiler::compile(Material* material, const String& slang_source, ShaderSource& out_source)
	{
		OpenGLRequestSetup setup;
		ShaderSource source;

		if (!compile_shader(slang_source, material->compile_definitions, source, &setup))
			return false;

		CompileLogHandler handler;
		compile_spirv_to_glsl_es(source.vertex_code);
		compile_spirv_to_glsl_es(source.tessellation_control_code);
		compile_spirv_to_glsl_es(source.tessellation_code);
		compile_spirv_to_glsl_es(source.geometry_code);
		compile_spirv_to_glsl_es(source.fragment_code);
		compile_spirv_to_glsl_es(source.compute_code);

		if (handler.has_error)
			return false;

		out_source = std::move(source);
		return true;
	}

	bool VULKAN_Compiler::compile(Material* material, const String& slang_source, ShaderSource& out_source)
	{
		VulkanRequestSetup setup;
		return compile_shader(slang_source, material->compile_definitions, out_source, &setup);
	}

	bool NONE_Compiler::compile(Material* material, const String& slang_source, ShaderSource& out_sources)
	{
		return false;
	}

	bool D3D11_Compiler::compile(Material* material, const String& slang_source, ShaderSource& out_source)
	{
		D3D11RequestSetup setup;
		return compile_shader(slang_source, material->compile_definitions, out_source, &setup);
	}
}// namespace Engine::ShaderCompiler

#endif
