#include <Core/definitions.hpp>

#if !PLATFORM_ANDROID
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/slang_shader_compiler.hpp>
#include <cstring>

#define RETURN_ON_FAIL(code)                                                                                                     \
	if (SLANG_FAILED(code))                                                                                                      \
	return false

#define return_undefined_if_not(cond)                                                                                            \
	if (!(cond))                                                                                                                 \
	return RHIShaderParameterType::Undefined

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
			static constexpr size_t exclude_scalar          = BIT(0);
			static constexpr size_t exclude_vector          = BIT(1);
			static constexpr size_t exclude_matrix          = BIT(2);
			static constexpr size_t exclude_resource        = BIT(3);
			static constexpr size_t exclude_sampler         = BIT(4);
			static constexpr size_t exclude_struct          = BIT(5);
			static constexpr size_t exclude_constant_buffer = BIT(6);

			String name;
			slang::VariableLayoutReflection* var = nullptr;
			const VarTraceEntry* const prev      = nullptr;
			slang::TypeReflection::Kind kind;
			size_t exclude_flags;

			VarTraceEntry(slang::VariableLayoutReflection* const var, const VarTraceEntry* const prev = nullptr)
			    : var(var), prev(prev), kind(var->getTypeLayout()->getKind()), exclude_flags(prev ? prev->exclude_flags : 0)
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

			StringView parameter_name() const
			{
				StringView result;
				if (auto attr = find_attribute("name"))
				{
					result = parse_string_attribute(attr, 0);

					if (!result.empty())
						return result;
				}

				return name;
			}

			inline slang::UserAttribute* find_attribute(const char* attribute) const
			{
				return var->getVariable()->findAttributeByName(global_session(), attribute);
			}

			inline bool has_attribute(const char* attribute) { return find_attribute(attribute) != nullptr; }
			inline slang::ParameterCategory category() const { return var->getCategory(); }
			inline bool is_excluded(size_t flags) const { return (exclude_flags & flags) == flags; }
			inline slang::VariableLayoutReflection* operator->() const { return var; }
		};

	private:
		Pipeline* m_pipeline;

	public:
		using TypeDetector = RHIShaderParameterType(slang::VariableLayoutReflection*, uint_t, uint_t, uint_t,
		                                            slang::TypeReflection::ScalarType);
		static Vector<TypeDetector*> type_detectors;

		static inline StringView parse_string_attribute(slang::UserAttribute* attribute, uint_t index)
		{
			size_t size;
			const char* name = attribute->getArgumentValueString(0, &size);
			if (name)
				return StringView(name, size);
			return StringView();
		}

		static RHIShaderParameterType find_parameter_type_from_attributes(slang::VariableReflection* var)
		{
			static Map<StringView, RHIShaderParameterType::Enum> map = {
			        {"LocalToWorld", RHIShaderParameterType::LocalToWorld},      //
			        {"Surface", RHIShaderParameterType::Surface},                //
			        {"CombinedSurface", RHIShaderParameterType::CombinedSurface},//
			};

			if (auto attrib = var->findAttributeByName(global_session(), "parameter_type"))
			{
				auto it = map.find(parse_string_attribute(attrib, 0));

				if (it != map.end())
					return it->second;
			}

			return RHIShaderParameterType::Undefined;
		}

		static uint32_t find_vertex_stream(slang::VariableReflection* var, uint32_t default_stream)
		{
			auto attrib = var->findAttributeByName(global_session(), "vertex_stream");
			if (attrib && attrib->getArgumentCount() == 1)
			{
				int value = 0;
				if (SLANG_SUCCEEDED(attrib->getArgumentValueInt(0, &value)))
				{
					return static_cast<uint32_t>(value);
				}
			}
			return default_stream;
		}

		static uint32_t find_vertex_offset(slang::VariableReflection* var)
		{
			auto attrib = var->findAttributeByName(global_session(), "vertex_offset");
			if (attrib && attrib->getArgumentCount() == 1)
			{
				int value = 0;
				if (SLANG_SUCCEEDED(attrib->getArgumentValueInt(0, &value)))
				{
					return static_cast<uint32_t>(value);
				}
			}
			return 0;
		}

		static bool find_semantic(String name, RHIVertexBufferSemantic& out_semantic)
		{
			name = Strings::to_lower(name);

			static const TreeMap<String, RHIVertexBufferSemantic> semantics = {
			        {"position", RHIVertexBufferSemantic::Position},       //
			        {"texcoord", RHIVertexBufferSemantic::TexCoord},       //
			        {"color", RHIVertexBufferSemantic::Color},             //
			        {"normal", RHIVertexBufferSemantic::Normal},           //
			        {"tangent", RHIVertexBufferSemantic::Tangent},         //
			        {"bitangent", RHIVertexBufferSemantic::Bitangent},     //
			        {"blendweight", RHIVertexBufferSemantic::BlendWeight}, //
			        {"blendindices", RHIVertexBufferSemantic::BlendIndices}//
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

		static RHIVertexBufferElementType find_vertex_element_type(slang::TypeLayoutReflection* var,
		                                                           RHIVertexBufferSemantic semantic)
		{
			if (var == nullptr)
				return RHIVertexBufferElementType::Undefined;

			auto kind = var->getKind();

			if (kind == slang::TypeReflection::Kind::Scalar)
			{
				switch (var->getScalarType())
				{
					case slang::TypeReflection::ScalarType::Int8: return RHIVertexBufferElementType::Byte1;
					case slang::TypeReflection::ScalarType::UInt8: return RHIVertexBufferElementType::UByte1;
					case slang::TypeReflection::ScalarType::Int16: return RHIVertexBufferElementType::Short1;
					case slang::TypeReflection::ScalarType::UInt16: return RHIVertexBufferElementType::UShort1;
					case slang::TypeReflection::ScalarType::Int32: return RHIVertexBufferElementType::Int1;
					case slang::TypeReflection::ScalarType::UInt32: return RHIVertexBufferElementType::UInt1;
					case slang::TypeReflection::ScalarType::Float32: return RHIVertexBufferElementType::Float1;
					default: return RHIVertexBufferElementType::Undefined;
				}
			}
			else if (kind == slang::TypeReflection::Kind::Vector)
			{
				auto base_type         = find_vertex_element_type(var->getElementTypeLayout(), semantic);
				auto components_offset = var->getElementCount() - 1;

				if (components_offset > 3)
					return RHIVertexBufferElementType::Undefined;

				if (components_offset == 3 && !is_in<RHIVertexBufferElementType::Float1, RHIVertexBufferElementType::Int1,
				                                     RHIVertexBufferElementType::UInt1>(base_type))
				{
					--components_offset;
				}

				RHIVertexBufferElementType result(
				        static_cast<RHIVertexBufferElementType::Enum>(static_cast<EnumerateType>(base_type) + components_offset));

				if (semantic == RHIVertexBufferSemantic::Color && result == RHIVertexBufferElementType::Float4)
					return RHIVertexBufferElementType::Color;
				return result;
			}

			return RHIVertexBufferElementType::Undefined;
		}

		bool parse_vertex_semantic(const VarTraceEntry& var)
		{
			auto category = var.category();

			if (category != slang::ParameterCategory::VaryingInput && category != slang::ParameterCategory::Mixed)
				return true;

			if (var.kind == slang::TypeReflection::Kind::Struct)
			{
				auto layout       = var->getTypeLayout();
				auto fields_count = layout->getFieldCount();

				for (uint32_t field_index = 0; field_index < fields_count; ++field_index)
				{
					VarTraceEntry field(layout->getFieldByIndex(field_index), &var);

					if (!parse_vertex_semantic(field))
					{
						return false;
					}
				}
			}
			else if (var.kind == slang::TypeReflection::Kind::Vector || var.kind == slang::TypeReflection::Kind::Scalar)
			{
				RHIVertexAttribute attribute;

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

				if (is_not_in<RHIVertexBufferSemantic::Position, //
				              RHIVertexBufferSemantic::TexCoord, //
				              RHIVertexBufferSemantic::Color,    //
				              RHIVertexBufferSemantic::Normal,   //
				              RHIVertexBufferSemantic::Tangent,  //
				              RHIVertexBufferSemantic::Bitangent,//
				              RHIVertexBufferSemantic::BlendWeight>(attribute.semantic))
				{
					error_log("ShaderCompiler", "Semantic '%s' doesn't support vector type!", var->getSemanticName());
					return false;
				}

				attribute.semantic_index = var->getSemanticIndex();
				attribute.name           = var.name;
				attribute.rate           = var->getVariable()->findAttributeByName(global_session(), "per_instance")
				                                   ? RHIVertexAttributeInputRate::Instance
				                                   : RHIVertexAttributeInputRate::Vertex;
				attribute.type           = find_vertex_element_type(var->getTypeLayout(), attribute.semantic);
				attribute.location       = var.trace_offset(slang::ParameterCategory::VertexInput);
				attribute.stream_index   = find_vertex_stream(var->getVariable(), attribute.location);
				attribute.offset         = find_vertex_offset(var->getVariable());
				Object::instance_cast<GraphicsPipeline>(m_pipeline)->vertex_shader()->attributes.push_back(attribute);
			}
			else
			{
				error_log("ShaderCompiler", "Unsupported input variable type!");
				return false;
			}

			return true;
		}

		static bool is_scene_view(slang::TypeLayoutReflection* type)
		{
			return_if_false(type->getKind() == slang::TypeReflection::Kind::Struct) false;
			const char* name = type->getName();
			return_if_false(name != nullptr && std::strcmp(name, "SceneView") == 0) false;
			return_if_false(sizeof(GlobalShaderParameters) == type->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM)) false;
			return true;
		}

		static RHIShaderParameterType find_scalar_parameter_type(slang::VariableLayoutReflection* var)
		{
			auto type = find_parameter_type_from_attributes(var->getVariable());

			if (type != RHIShaderParameterType::Undefined)
				return type;

			auto reflection = var->getType();
			auto rows       = reflection->getRowCount();
			auto colums     = reflection->getColumnCount();
			auto elements   = reflection->getElementCount();
			auto scalar     = reflection->getScalarType();

			if (scalar != slang::TypeReflection::ScalarType::None)
			{
				for (auto& detector : type_detectors)
				{
					auto type = detector(var, rows, colums, elements, scalar);
					if (type != RHIShaderParameterType::Undefined)
					{
						return type;
					}
				}
			}

			return RHIShaderParameterType::MemoryBlock;
		}

		bool parse_shader_parameter(const VarTraceEntry& param)
		{
			if (is_in<slang::TypeReflection::Kind::Scalar, slang::TypeReflection::Kind::Vector,
			          slang::TypeReflection::Kind::Matrix>(param.kind))
			{
				if (param.kind == slang::TypeReflection::Kind::Scalar && param.is_excluded(VarTraceEntry::exclude_scalar))
					return true;
				if (param.kind == slang::TypeReflection::Kind::Vector && param.is_excluded(VarTraceEntry::exclude_vector))
					return true;
				if (param.kind == slang::TypeReflection::Kind::Matrix && param.is_excluded(VarTraceEntry::exclude_matrix))
					return true;

				RHIShaderParameterInfo info;
				info.type = find_scalar_parameter_type(param.var);

				if (info.type == RHIShaderParameterType::Undefined)
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

				info.name    = param.parameter_name();
				info.offset  = param.trace_offset(slang::ParameterCategory::Uniform);
				info.binding = param.trace_offset(slang::ParameterCategory::ConstantBuffer);
				return m_pipeline->add_parameter(info);
			}
			else if (is_in<slang::TypeReflection::Kind::Resource>(param.kind) &&
			         !param.is_excluded(VarTraceEntry::exclude_resource))
			{
				auto type = find_parameter_type_from_attributes(param.var->getVariable());

				if (auto type_layout = param.var->getTypeLayout())
				{
					SlangResourceShape shape = type_layout->getResourceShape();

					if (shape == SLANG_TEXTURE_2D)
					{
						auto binding_type = type_layout->getBindingRangeType(0);

						RHIShaderParameterInfo object;
						object.name    = param.parameter_name();
						object.binding = param.trace_offset(param.category());
						object.type    = type;

						if (type == RHIShaderParameterType::Undefined)
						{
							switch (binding_type)
							{
								case slang::BindingType::CombinedTextureSampler:
									object.type = RHIShaderParameterType::Sampler2D;
									break;

								case slang::BindingType::Texture: object.type = RHIShaderParameterType::Texture2D; break;

								case slang::BindingType::MutableTexture: object.type = RHIShaderParameterType::RWTexture2D; break;

								default: return false;
							}
						}

						return m_pipeline->add_parameter(object);
					}
				}
			}
			else if (is_in<slang::TypeReflection::Kind::SamplerState>(param.kind) &&
			         !param.is_excluded(VarTraceEntry::exclude_sampler))
			{
				RHIShaderParameterInfo object;
				object.name    = param.parameter_name();
				object.binding = param.trace_offset(param.category());
				object.type    = RHIShaderParameterType::Sampler;
				m_pipeline->add_parameter(object);
			}
			else if (is_in<slang::TypeReflection::Kind::Struct>(param.kind) && !param.is_excluded(VarTraceEntry::exclude_struct))
			{
				auto layout = param.var->getTypeLayout();
				auto fields = layout->getFieldCount();

				size_t additional_exclude = 0;

				if (auto attr = param.find_attribute("parameter_type"))
				{
					if (parse_string_attribute(attr, 0) == "MemoryBlock")
					{
						additional_exclude |= VarTraceEntry::exclude_scalar;
						additional_exclude |= VarTraceEntry::exclude_vector;
						additional_exclude |= VarTraceEntry::exclude_matrix;

						RHIShaderParameterInfo info;

						if (auto layout = param.var->getTypeLayout())
						{
							info.size = layout->getSize();
						}
						else
						{
							error_log("ShaderCompiler", "Failed to get parameter layout info!");
							return false;
						}

						info.type    = RHIShaderParameterType::MemoryBlock;
						info.name    = param.parameter_name();
						info.offset  = param.trace_offset(slang::ParameterCategory::Uniform);
						info.binding = param.trace_offset(slang::ParameterCategory::ConstantBuffer);
						m_pipeline->add_parameter(info);
					}
				}

				for (decltype(fields) i = 0; i < fields; i++)
				{
					VarTraceEntry var(layout->getFieldByIndex(i), &param);
					var.exclude_flags |= additional_exclude;
					return_if_false(parse_shader_parameter(var)) false;
				}
			}
			else if (is_in<slang::TypeReflection::Kind::ConstantBuffer>(param.kind) &&
			         !param.is_excluded(VarTraceEntry::exclude_constant_buffer))
			{
				auto layout = param.var->getTypeLayout()->getElementTypeLayout();

				if (is_scene_view(layout))
				{
					RHIShaderParameterInfo object;
					object.name    = param.parameter_name();
					object.binding = param.trace_offset(slang::ParameterCategory::DescriptorTableSlot);
					object.type    = RHIShaderParameterType::Globals;
					object.size    = sizeof(GlobalShaderParameters);
					object.offset  = 0;
					return m_pipeline->add_parameter(object);
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
			else
			{
				error_log("ShaderCompiler", "Resource type with name '%s' is not supported as uniform parameters",
				          param.name.c_str());
				return false;
			}

			return true;
		}

		bool create_reflection(slang::ShaderReflection* reflection, Pipeline* out)
		{
			CompileLogHandler log_handler;
			m_pipeline = out;

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

		template<RHIShaderParameterType type, Scalar required_scalar>
		static RHIShaderParameterType primitive(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == 1);
			return_undefined_if_not(elements == 0);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<RHIShaderParameterType type, uint32_t len, Scalar required_scalar>
		static RHIShaderParameterType vector(SVLR*, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == len);
			return_undefined_if_not(elements == len);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<RHIShaderParameterType type, Scalar required_scalar, uint_t required_rows, uint_t required_columns>
		static RHIShaderParameterType matrix(SVLR* var, uint_t rows, uint_t columns, uint_t elements, Scalar scalar)
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
		using MP     = RHIShaderParameterType;

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

	trinex_implement_class_default_init(Engine::SLANG_ShaderCompiler, Refl::Class::IsSingletone);
	trinex_implement_class_default_init(Engine::VULKAN_ShaderCompiler, Refl::Class::IsSingletone);
	trinex_implement_class_default_init(Engine::NONE_ShaderCompiler, Refl::Class::IsSingletone);
	trinex_implement_class_default_init(Engine::D3D12_ShaderCompiler, Refl::Class::IsSingletone);

	class SLANG_CompilationEnv : public ShaderCompilationEnvironment
	{
		SLANG_ShaderCompiler::Context* m_ctx = nullptr;

	public:
		SLANG_CompilationEnv(SLANG_ShaderCompiler::Context* ctx) : m_ctx(ctx) {}

		SLANG_CompilationEnv& add_module(const char* module) override
		{
			slang::IModule* shader_module = m_ctx->compiler->load_module(module);
			if (shader_module)
				m_ctx->component_types.push_back(shader_module);
			return *this;
		}
	};

	SLANG_ShaderCompiler::Context::Context(SLANG_ShaderCompiler* compiler) : compiler(compiler), prev_ctx(compiler->m_ctx)
	{
		compiler->m_ctx = this;
		component_types.reserve(16);
	}

	bool SLANG_ShaderCompiler::Context::initialize(const String& source, Pipeline* pipeline)
	{
		auto& session = compiler->m_session;

		if (!session)
		{
			error_log("ShaderCompiler", "Failed to compile shader, because compiler session is invalid!");
			return false;
		}

		String name = Strings::format("Module<{}> '{}'", static_cast<void*>(pipeline), pipeline->name().c_str());
		Slang::ComPtr<slang::IBlob> diagnostics;
		auto module = session->loadModuleFromSourceString(name.c_str(), name.c_str(), source.data(), diagnostics.writeRef());

		if (diagnostics && diagnostics->getBufferSize() > 0)
		{
			if (module)
			{
				warn_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
				return true;
			}
			else
			{
				error_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
				return false;
			}
		}

		if (module)
		{
			component_types.push_back(module);
			return true;
		}
		return false;
	}

	bool SLANG_ShaderCompiler::Context::compile(ShaderInfo* infos, size_t infos_len, Pipeline* pipeline, CheckStages checker)
	{
		const size_t module_count = component_types.size();

		for (size_t i = 0; i < infos_len; ++i)
		{
			auto& info = infos[i];

			for (size_t module_index = 0; module_index < module_count; ++module_index)
			{
				auto module = static_cast<slang::IModule*>(component_types[module_index]);

				slang::IEntryPoint* entry = nullptr;
				module->findEntryPointByName(info.entry_name, &entry);

				if (entry)
				{
					if (info.entry)
					{
						error_log("ShaderCompiler", "Detected multiple entry points with name '%s'", info.entry_name);
						return false;
					}

					info.index = component_types.size() - module_count;
					info.entry = entry;
					component_types.push_back(entry);
				}
			}
		}

		if (!checker(infos))
			return false;

		Slang::ComPtr<slang::IComponentType> composite;
		{
			Slang::ComPtr<slang::IBlob> diagnostics_blob;
			SlangResult result = compiler->m_session->createCompositeComponentType(
			        component_types.data(), component_types.size(), composite.writeRef(), diagnostics_blob.writeRef());

			if (diagnostics_blob != nullptr)
			{
				error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
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
				error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
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
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
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
					error_log("ShaderCompiler", "Failed to create shader");
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
				error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
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

	bool SLANG_ShaderCompiler::Context::compile_graphics(const String& source, Material* material, RenderPass* pass)
	{
		if (pass == nullptr)
		{
			error_log("ShaderCompiler", "Cannot compile material for undefined pass!\n");
			return false;
		}

		if (pass)
		{
			SLANG_CompilationEnv env(this);
			pass->modify_shader_compilation_env(&env);
		}

		GraphicsPipeline* pipeline = material->remove_pipeline(pass);
		const bool new_pipeline    = pipeline == nullptr;

		if (new_pipeline)
		{
			pipeline = new_instance<GraphicsPipeline>(pass->name());
		}
		else
		{
			pipeline->clear();
		}

		if (compile_graphics(source, pipeline))
		{
			material->add_pipeline(pass, pipeline);
			material->post_compile(pass, pipeline);
			pipeline->init_render_resources();
			return true;
		}

		if (new_pipeline)
		{
			GarbageCollector::destroy(pipeline);
		}

		return false;
	}

	bool SLANG_ShaderCompiler::Context::compile_graphics(const String& source, Pipeline* pipeline, RenderPass* pass)
	{
		SLANG_CompilationEnv env(this);
		pipeline->modify_compilation_env(&env);

		if (!initialize(source, pipeline))
			return false;

		ShaderInfo shader_infos[] = {
		        {ShaderType::Vertex, "vs_main"},              //
		        {ShaderType::TessellationControl, "tsc_main"},//
		        {ShaderType::Tessellation, "ts_main"},        //
		        {ShaderType::Geometry, "gs_main"},            //
		        {ShaderType::Fragment, "fs_main"},            //
		};

		CheckStages checker = [](ShaderInfo* infos) -> bool {
			bool result = infos[0].index != -1 && infos[4].index != -1;
			if (!result)
				error_log("ShaderCompiler", "Graphics pipeline is not valid!");
			return result;
		};

		if (compile(shader_infos, 5, pipeline, checker))
		{
			pipeline->post_compile(pass);
			return true;
		}
		return false;
	}

	bool SLANG_ShaderCompiler::Context::compile_compute(const String& source, Pipeline* pipeline)
	{
		SLANG_CompilationEnv env(this);
		pipeline->modify_compilation_env(&env);

		if (!initialize(source, pipeline))
			return false;

		ShaderInfo shader_infos[] = {{ShaderType::Compute, "cs_main"}};

		CheckStages checker = [](ShaderInfo* infos) -> bool {
			bool result = infos[0].index != -1;
			if (!result)
				error_log("ShaderCompiler", "Compute pipeline is not valid!");
			return result;
		};

		return compile(shader_infos, 1, pipeline, checker);
	}

	SLANG_ShaderCompiler::Context::~Context()
	{
		compiler->m_ctx = prev_ctx;

		for (slang::IComponentType* component : component_types)
		{
			component->release();
		}
	}

	SLANG_ShaderCompiler::SLANG_ShaderCompiler()
	{
		flags(StandAlone, true);
		flags(IsAvailableForGC, false);
	}

	SLANG_ShaderCompiler& SLANG_ShaderCompiler::on_create()
	{
		Super::on_create();

		Path include_directories[] = {
		        rootfs()->native_path(Project::shaders_dir),
		        rootfs()->native_path("[shaders_dir]:/TrinexEditor"),
		        rootfs()->native_path("[shaders_dir]:/TrinexEngine"),
		};

		SessionInitializer desc;

		desc.session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		desc.session_desc.allowGLSLSyntax         = false;
		desc.session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		desc.session_desc.targetCount             = 1;
		desc.session_desc.targets                 = &desc.target_desc;

		for (auto& include_dir : include_directories) desc.add_search_path(include_dir.c_str());
		initialize_context(&desc);

		desc.session_desc.searchPathCount          = desc.search_paths.size();
		desc.session_desc.searchPaths              = desc.search_paths.data();
		desc.session_desc.preprocessorMacroCount   = desc.definitions.size();
		desc.session_desc.preprocessorMacros       = desc.definitions.data();
		desc.session_desc.compilerOptionEntryCount = desc.options.size();
		desc.session_desc.compilerOptionEntries    = desc.options.data();

		desc.target_desc.compilerOptionEntryCount = desc.target_options.size();
		desc.target_desc.compilerOptionEntries    = desc.target_options.data();

		if (SLANG_FAILED(global_session()->createSession(desc.session_desc, m_session.writeRef())))
		{
			error_log("Shader Compiler", "Failed to create session");
			m_session = nullptr;
		}
		return *this;
	}

	void SLANG_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		session->add_option(slang::CompilerOptionName::DisableWarning, "15205");
		session->target_desc.format = SLANG_TARGET_NONE;
		session->add_option(slang::CompilerOptionName::LineDirectiveMode, SLANG_LINE_DIRECTIVE_MODE_NONE);

		if (Settings::debug_shaders)
		{
			session->add_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_NONE);
			session->add_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_STANDARD);
		}
		else
		{
			session->add_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
			session->add_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_NONE);
		}
	}

	bool SLANG_ShaderCompiler::compile(Material* material)
	{
		String source;

		material->remove_all_pipelines();

		if (!material->shader_source(source))
		{
			error_log("ShaderCompiler", "Failed to get shader source");
			return false;
		}

		bool success = true;

		for (auto pass = RenderPass::static_first_pass(); pass && success; pass = pass->next_pass())
		{
			if (pass->is_material_compatible(material))
			{
				success = compile_pass(material, pass, source);
			}
		}

		material->remove_unreferenced_parameters();
		return success;
	}

	void SLANG_ShaderCompiler::submit_source(Shader* shader, const byte* src, size_t size)
	{
		Buffer& out = shader->source_code;
		std::destroy_at(&out);
		new (&out) Buffer(src, src + size);
	}

	bool SLANG_ShaderCompiler::compile_pass(Material* material, RenderPass* pass)
	{
		String source;

		if (!material->shader_source(source))
		{
			error_log("ShaderCompiler", "Failed to get shader source");
			return false;
		}

		bool result = compile_pass(material, pass, source);
		material->remove_unreferenced_parameters();
		return result;
	}

	bool SLANG_ShaderCompiler::compile_pass(Material* material, RenderPass* pass, const String& source)
	{
		Context ctx(this);
		return ctx.compile_graphics(source, material, pass);
	}

	bool SLANG_ShaderCompiler::compile(const String& source, Pipeline* pipeline)
	{
		Context ctx(this);

		if (pipeline->is_instance_of<GraphicsPipeline>())
		{
			if (ctx.compile_graphics(source, pipeline))
			{
				pipeline->init_render_resources();
				return true;
			}
		}
		else if (pipeline->is_instance_of<ComputePipeline>())
		{
			if (ctx.compile_compute(source, pipeline))
			{
				pipeline->init_render_resources();
				return true;
			}
		}
		return false;
	}

	slang::IModule* SLANG_ShaderCompiler::load_module(const char* module)
	{
		return m_session->loadModule(module);
	}

	void NONE_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		throw EngineException("Something is wrong! Cannot compile shaders for None API!");
	}

	void VULKAN_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_SPIRV;
		session->target_desc.profile = global_session()->findProfile("spirv_1_3");
		session->add_definition("TRINEX_VULKAN_RHI", "1");
	}

	void D3D12_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_DXIL;
		session->target_desc.profile = global_session()->findProfile("sm_6_0");

		session->add_definition("TRINEX_INVERT_UV", "1");
		session->add_definition("TRINEX_D3D12_RHI", "1");
		session->add_definition("TRINEX_DIRECT_X_RHI", "1");
	}
}// namespace Engine

#endif
