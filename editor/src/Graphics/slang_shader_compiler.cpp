#include <Core/definitions.hpp>

#if !PLATFORM_ANDROID
#include <Core/etl/flat_set.hpp>
#include <Core/etl/span.hpp>
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

	static slang::IGlobalSession* g_slang_global_session = nullptr;

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

				while (current)
				{
					result += current->var->getOffset(category);
					current = current->prev;
				}

				return result;
			}

			size_t trace_offset(slang::ParameterCategory category) const
			{
				return trace_offset(static_cast<SlangParameterCategory>(category));
			}

			size_t trace_space(SlangParameterCategory category) const
			{
				const VarTraceEntry* current = this;
				size_t result                = 0;

				while (current)
				{
					result += current->var->getBindingSpace(category);
					current = current->prev;
				}

				return result;
			}

			size_t trace_space(slang::ParameterCategory category) const
			{
				return trace_space(static_cast<SlangParameterCategory>(category));
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
				return var->getVariable()->findAttributeByName(g_slang_global_session, attribute);
			}

			inline bool has_attribute(const char* attribute) { return find_attribute(attribute) != nullptr; }
			inline slang::ParameterCategory category() const { return var->getCategory(); }
			inline slang::ParameterCategory category(uint_t index) const { return var->getCategoryByIndex(index); }
			inline uint_t category_count() const { return var->getCategoryCount(); }
			inline bool is_excluded(size_t flags) const { return (exclude_flags & flags) == flags; }
			inline slang::VariableLayoutReflection* operator->() const { return var; }
		};

	private:
		ShaderCompilationResult::Reflection* m_reflection;
		Span<Slang::ComPtr<slang::IMetadata>> m_metadatas;

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

		static RHIShaderParameterType find_parameter_type(slang::VariableReflection* var)
		{
			if (auto attrib = var->findAttributeByName(g_slang_global_session, "parameter_type"))
			{
				if (attrib->getArgumentCount() == 1)
				{
					int value = 0;

					if (attrib->getArgumentValueInt(0, &value) == SLANG_OK)
					{
						return static_cast<RHIShaderParameterType>(value);
					}
				}
			}

			return RHIShaderParameterType::Undefined;
		}

		static bool find_semantic(String name, RHIVertexSemantic& out_semantic)
		{
			name = Strings::to_lower(name);

			static const TreeMap<String, RHIVertexSemantic> semantics = {
			        {"position", RHIVertexSemantic::Position},        //
			        {"texcoord", RHIVertexSemantic::TexCoord0},       //
			        {"color", RHIVertexSemantic::Color},              //
			        {"normal", RHIVertexSemantic::Normal},            //
			        {"tangent", RHIVertexSemantic::Tangent},          //
			        {"bitangent", RHIVertexSemantic::Bitangent},      //
			        {"blendweight", RHIVertexSemantic::BlendWeight},  //
			        {"blendindices", RHIVertexSemantic::BlendIndices},//
			        {"userdata", RHIVertexSemantic::UserData}         //
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

		inline bool is_variable_used(const VarTraceEntry& var, byte index)
		{
			using SPC = SlangParameterCategory;

			for (slang::IMetadata* meta : m_metadatas)
			{
				uint_t categories = var.category_count();

				for (uint_t i = 0; i < categories; ++i)
				{
					slang::ParameterCategory category = var->getCategory();

					bool is_used       = false;
					SlangResult result = meta->isParameterLocationUsed(static_cast<SPC>(category), 0, index, is_used);

					if (result == SLANG_OK && is_used)
						return true;
				}
			}

			if (var.prev)
				return is_variable_used(*var.prev, index);
			return false;
		}

		bool parse_vertex_attribute(const VarTraceEntry& var)
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

					if (!parse_vertex_attribute(field))
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

				if (!find_semantic(semantic_name, attribute.semantic))
				{
					return false;
				}

				{
					const size_t max_semantic_index = attribute.semantic == RHIVertexSemantic::TexCoord0 ? 3 : 0;

					size_t index = var->getSemanticIndex();

					if (index > max_semantic_index)
					{
						error_log("ShaderCompiler", "Unsupported semantic index %zu for semantic %s!", index, semantic_name);
						return false;
					}

					attribute.semantic = static_cast<RHIVertexSemantic::Enum>(attribute.semantic.value + index);
				}

				attribute.binding = var.trace_offset(category);
				m_reflection->vertex_attributes.push_back(attribute);
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
			auto type = find_parameter_type(var->getVariable());

			auto reflection = var->getType();
			auto rows       = reflection->getRowCount();
			auto colums     = reflection->getColumnCount();
			auto elements   = reflection->getElementCount();
			auto scalar     = reflection->getScalarType();

			if (scalar != slang::TypeReflection::ScalarType::None)
			{
				for (auto& detector : type_detectors)
				{
					auto detected_type = detector(var, rows, colums, elements, scalar);
					if (detected_type != RHIShaderParameterType::Undefined)
					{
						return detected_type | type;
					}
				}
			}

			return RHIShaderParameterType::UniformBuffer;
		}

		static RHIShaderParameterType parse_binding_type(slang::TypeLayoutReflection* refl)
		{
			RHIShaderParameterType type = RHIShaderParameterType::Undefined;

			//  ParameterBlock = SLANG_BINDING_TYPE_PARAMETER_BLOCK,
			//  TypedBuffer = SLANG_BINDING_TYPE_TYPED_BUFFER,
			//  RawBuffer = SLANG_BINDING_TYPE_RAW_BUFFER,
			//  InputRenderTarget = SLANG_BINDING_TYPE_INPUT_RENDER_TARGET,
			//  InlineUniformData = SLANG_BINDING_TYPE_INLINE_UNIFORM_DATA,
			//  RayTracingAccelerationStructure = SLANG_BINDING_TYPE_RAY_TRACING_ACCELERATION_STRUCTURE,
			//  VaryingInput = SLANG_BINDING_TYPE_VARYING_INPUT,
			//  VaryingOutput = SLANG_BINDING_TYPE_VARYING_OUTPUT,
			//  ExistentialValue = SLANG_BINDING_TYPE_EXISTENTIAL_VALUE,
			//  PushConstant = SLANG_BINDING_TYPE_PUSH_CONSTANT,
			//  MutableTypedBuffer = SLANG_BINDING_TYPE_MUTABLE_TYPED_BUFFER,
			//  MutableRawBuffer = SLANG_BINDING_TYPE_MUTABLE_RAW_BUFFER,

			for (uint_t i = 0, count = refl->getBindingRangeCount(); i < count; ++i)
			{
				slang::BindingType binding_type = refl->getBindingRangeType(i);

				switch (binding_type)
				{
					case slang::BindingType::ConstantBuffer: type |= RHIShaderParameterType::META_UniformBuffer; break;
					case slang::BindingType::CombinedTextureSampler: type |= RHIShaderParameterType::META_Texture;
					case slang::BindingType::Sampler: type |= RHIShaderParameterType::META_Sampler; break;

					case slang::BindingType::MutableTexture: type |= RHIShaderParameterType::META_RW;
					case slang::BindingType::Texture: type |= RHIShaderParameterType::META_Texture; break;

					default: return false;
				}
			}

			return type;
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
				info.binding = param.trace_offset(slang::ParameterCategory::ConstantBuffer);

				if (!is_variable_used(param, info.binding))
					return true;

				info.offset = param.trace_offset(slang::ParameterCategory::Uniform);
				info.name   = param.parameter_name();
				info.type   = find_scalar_parameter_type(param.var);

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

				m_reflection->parameters.push_back(info);
				return true;
			}
			else if (is_in<slang::TypeReflection::Kind::Resource, slang::TypeReflection::Kind::SamplerState>(param.kind) &&
			         !param.is_excluded(VarTraceEntry::exclude_resource))
			{
				RHIShaderParameterInfo object;
				object.binding = param.trace_offset(param.category());

				if (!is_variable_used(param, object.binding))
					return true;

				object.name = param.parameter_name();
				object.type = find_parameter_type(param.var->getVariable());

				if (auto type_layout = param.var->getTypeLayout())
				{
					SlangResourceShape shape      = type_layout->getResourceShape();
					SlangResourceShape shape_mask = static_cast<SlangResourceShape>(shape & SLANG_RESOURCE_BASE_SHAPE_MASK);

					object.type |= parse_binding_type(type_layout);

					if (shape_mask & SLANG_TEXTURE_ARRAY_FLAG)
					{
						object.type |= RHIShaderParameterType::META_Array;
					}

					if (shape_mask & SLANG_TEXTURE_COMBINED_FLAG)
					{
						object.type |= RHIShaderParameterType::META_Texture;
						object.type |= RHIShaderParameterType::META_Sampler;
					}

					if (shape_mask & SLANG_TEXTURE_ARRAY_FLAG)
					{
						object.type |= RHIShaderParameterType::META_Array;
					}

					if (shape_mask == SLANG_TEXTURE_2D)
					{
						object.type |= RHIShaderParameterType::META_2D;
						m_reflection->parameters.push_back(object);
						return true;
					}
					else if (shape_mask == SLANG_TEXTURE_3D)
					{
						object.type |= RHIShaderParameterType::META_2D;
						m_reflection->parameters.push_back(object);
						return true;
					}
					else if (shape_mask == SLANG_TEXTURE_CUBE)
					{
						object.type |= RHIShaderParameterType::META_Cube;
						m_reflection->parameters.push_back(object);
						return true;
					}
					else if (shape_mask == SLANG_STRUCTURED_BUFFER)
					{
						object.type |= RHIShaderParameterType::META_StructuredBuffer;
						m_reflection->parameters.push_back(object);
						return true;
					}
					else if (shape_mask == SLANG_BYTE_ADDRESS_BUFFER)
					{
						object.type |= RHIShaderParameterType::META_ByteAddressBuffer;
						m_reflection->parameters.push_back(object);
						return true;
					}
					else if (param.kind == slang::TypeReflection::Kind::SamplerState)
					{
						object.type |= RHIShaderParameterType::META_Sampler;
						m_reflection->parameters.push_back(object);
						return true;
					}
				}
			}
			else if (is_in<slang::TypeReflection::Kind::Struct>(param.kind) && !param.is_excluded(VarTraceEntry::exclude_struct))
			{
				auto layout = param.var->getTypeLayout();
				auto fields = layout->getFieldCount();

				size_t additional_exclude = 0;

				RHIShaderParameterType flags = find_parameter_type(param.var->getVariable());

				if (flags & RHIShaderParameterType::META_UniformBuffer)
				{
					additional_exclude |= VarTraceEntry::exclude_scalar;
					additional_exclude |= VarTraceEntry::exclude_vector;
					additional_exclude |= VarTraceEntry::exclude_matrix;

					RHIShaderParameterInfo info;
					info.binding = param.trace_offset(slang::ParameterCategory::ConstantBuffer);

					if (is_variable_used(param, info.binding))
					{
						if (auto layout = param.var->getTypeLayout())
						{
							info.size = layout->getSize();
						}
						else
						{
							error_log("ShaderCompiler", "Failed to get parameter layout info!");
							return false;
						}

						info.type   = flags;
						info.name   = param.parameter_name();
						info.offset = param.trace_offset(slang::ParameterCategory::Uniform);

						m_reflection->parameters.push_back(info);
						return true;
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
					object.binding = param.trace_offset(param.category());

					if (!is_variable_used(param, object.binding))
						return true;

					object.name   = param.parameter_name();
					object.type   = RHIShaderParameterType::Globals;
					object.size   = sizeof(GlobalShaderParameters);
					object.offset = 0;
					m_reflection->parameters.push_back(object);
					return true;
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

		bool create_reflection(slang::ShaderReflection* reflection, ShaderCompilationResult::Reflection* out,
		                       const Span<Slang::ComPtr<slang::IMetadata>>& metadatas)
		{
			CompileLogHandler log_handler;
			m_reflection = out;
			m_metadatas  = metadatas;

			// Parse vertex attributes
			if (auto entry_point = reflection->findEntryPointByName("vertex_main"))
			{
				uint32_t parameter_count = entry_point->getParameterCount();
				for (uint32_t i = 0; i < parameter_count; i++)
				{
					return_if_false(parse_vertex_attribute(entry_point->getParameterByIndex(i))) false;
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

	SLANG_ShaderCompiler::SLANG_ShaderCompiler()
	{
		flags(StandAlone, true);
		flags(IsAvailableForGC, false);

		if (g_slang_global_session == nullptr)
		{
			if (SLANG_FAILED(slang::createGlobalSession(&g_slang_global_session)))
			{
				throw EngineException("Cannot create global session");
			}
		}
		else
		{
			g_slang_global_session->AddRef();
		}
	}

	SLANG_ShaderCompiler::~SLANG_ShaderCompiler()
	{
		if (g_slang_global_session->Release() == 0)
		{
			g_slang_global_session = nullptr;
		}
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
		desc.session_desc.targetCount = 1;
		desc.session_desc.targets     = &desc.target_desc;

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

		if (SLANG_FAILED(g_slang_global_session->createSession(desc.session_desc, m_session.writeRef())))
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

	bool SLANG_ShaderCompiler::submit_result(ShaderCompilationResult& result)
	{
		return true;
	}

	bool SLANG_ShaderCompiler::compile(const ShaderCompilationEnvironment* env, ShaderCompilationResult& result)
	{
		StackByteAllocator::Mark mark;
		StackVector<slang::IComponentType*> components;
		StackVector<Slang::ComPtr<slang::IMetadata>> metadata;
		Slang::ComPtr<slang::IComponentType> program;

		ShaderInfo shader_infos[] = {
		        {&result.shaders.vertex, "vertex_main"},                            // vertex shader
		        {&result.shaders.tessellation_control, "tessellation_control_main"},// tess control shader
		        {&result.shaders.tessellation, "tessellation_main"},                // tess evaluation shader
		        {&result.shaders.geometry, "geometry_main"},                        // geometry shader
		        {&result.shaders.fragment, "fragment_main"},                        // fragment shader
		        {&result.shaders.compute, "compute_main"},                          // compute shader
		        {&result.shaders.mesh, "mesh_main"},                                // mesh shader
		        {&result.shaders.task, "task_main"},                                // task shader
		        {&result.shaders.raygen, "raygen_main"},                            // ray generation shader
		        {&result.shaders.closest_hit, "closest_hit_main"},                  // closest hit shader
		        {&result.shaders.any_hit, "any_hit_main"},                          // any hit shader
		        {&result.shaders.miss, "miss_main"},                                // miss shader
		};

		components.reserve(env->sources_count() + env->modules_count() + 12);
		metadata.reserve(12);

		///////// STEP ONE: COLLECT COMPONENTS /////////
		{
			// Process sources
			for (size_t i = 0, count = env->sources_count(); i < count; ++i)
			{
				const char* source = env->source(i);
				static uint_t id   = 0;
				String name        = Strings::format("Unnamed module {}", ++id);

				Slang::ComPtr<slang::IBlob> diagnostics;
				slang::IModule* module =
				        m_session->loadModuleFromSourceString(name.c_str(), name.c_str(), source, diagnostics.writeRef());

				if (diagnostics && diagnostics->getBufferSize() > 0)
				{
					if (module)
					{
						warn_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
					}
					else
					{
						error_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
						return false;
					}
				}

				components.push_back(module);
			}

			// Process modules
			for (size_t i = 0, count = env->modules_count(); i < count; ++i)
			{
				Slang::ComPtr<slang::IBlob> diagnostics;
				slang::IModule* module = m_session->loadModule(env->module(i), diagnostics.writeRef());

				if (diagnostics && diagnostics->getBufferSize() > 0)
				{
					if (module)
					{
						warn_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
					}
					else
					{
						error_log("ShaderCompiler", reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
						return false;
					}
				}

				components.push_back(module);
			}

			components.push_back(load_module("trinex/trinex.slang"));
		}

		///////// STEP TWO: COLLECT ENTRY POINTS  /////////
		{
			const size_t module_count = components.size();
			for (auto& info : shader_infos)
			{
				for (size_t module_index = 0; module_index < module_count; ++module_index)
				{
					auto module = static_cast<slang::IModule*>(components[module_index]);

					slang::IEntryPoint* entry = nullptr;
					module->findEntryPointByName(info.entry_name, &entry);

					if (entry)
					{
						if (info.entry)
						{
							error_log("ShaderCompiler", "Detected multiple entry points with name '%s'", info.entry_name);
							return false;
						}

						info.index = components.size() - module_count;
						info.entry = entry;
						components.push_back(entry);
					}
				}
			}
		}

		///////// STEP THREE: COMPILE COMPONENTS TO COMPOSITE PROGRAM  /////////
		{
			Slang::ComPtr<slang::IComponentType> composite;
			{
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult result = m_session->createCompositeComponentType(components.data(), components.size(),
				                                                             composite.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
				{
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
				}

				if (SLANG_FAILED(result))
					return false;
			}

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
		}

		///////// STEP FOUR: COLLECT COMPILED SHADER SOURCE  /////////
		for (auto& info : shader_infos)
		{
			if (info.index != -1)
			{
				Slang::ComPtr<slang::IBlob> code;
				Slang::ComPtr<slang::IBlob> diagnostics_blob;

				SlangResult result = program->getEntryPointCode(info.index, 0, code.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
				{
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
				}

				if (SLANG_FAILED(result))
					return false;

				info.source->resize(code->getBufferSize());
				std::memcpy(info.source->data(), code->getBufferPointer(), info.source->size());
				program->getEntryPointMetadata(info.index, 0, metadata.emplace_back().writeRef(), nullptr);
			}
		}

		///////// STEP FIVE: GENERATE REFLECTION  /////////
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

			if (!parser.create_reflection(reflection, &result.reflection, metadata))
				return false;
		}

		return submit_result(result);
	}

	void NONE_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		throw EngineException("Something is wrong! Cannot compile shaders for None API!");
	}

	bool VULKAN_ShaderCompiler::strip_vertex_inputs(const uint32_t* spirv, const uint32_t words,
	                                                Vector<RHIVertexAttribute>& attributes)
	{
		if (words < 5)
			return false;

		StackByteAllocator::Mark mark;

		FlatSet<uint32_t, Less<uint32_t>, StackAllocator<uint32_t>> inputs;
		FlatSet<uint32_t, Less<uint32_t>, StackAllocator<uint32_t>> locations;
		inputs.reserve(attributes.size());
		locations.reserve(attributes.size());

		// Collect all vertex inputs
		{
			size_t i = 5;
			while (i < words)
			{
				uint16_t wc = SPIRV::wordcount(spirv[i]);
				uint16_t op = SPIRV::opcode(spirv[i]);

				if (wc == 0 || i + wc > words)
					return false;

				if (op == SPIRV::OpVariable && wc >= 4)
				{
					uint32_t target  = spirv[i + 2];
					uint32_t storage = spirv[i + 3];

					if (storage == SPIRV::StorageClass_Input)
						inputs.insert(target);
				}

				i += wc;
			}
		}

		if (inputs.empty())
		{
			attributes.clear();
			attributes.shrink_to_fit();
			return true;
		}

		// Collect all vertex locations
		{
			size_t i = 5;
			while (i < words && inputs.size() != locations.size())
			{
				uint32_t first = spirv[i];
				uint16_t wc    = SPIRV::wordcount(first);
				uint16_t op    = SPIRV::opcode(first);

				if (wc == 0 || i + wc > words)
					return false;

				if (op == SPIRV::OpDecorate)
				{
					uint32_t target     = spirv[i + 1];
					uint32_t decoration = spirv[i + 2];

					if (decoration == SPIRV::Decoration_Location && inputs.contains(target))
						locations.insert(spirv[i + 3]);
				}

				i += wc;
			}
		}

		{
			size_t index = 0;
			size_t count = attributes.size();

			while (index < count)
			{
				if (locations.contains(attributes[index].binding))
				{
					++index;
				}
				else
				{
					attributes.erase(attributes.begin() + index);
					--count;
				}
			}
		}

		return true;
	}

	void VULKAN_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_SPIRV;
		session->target_desc.profile = g_slang_global_session->findProfile("spirv_1_3");
		session->add_definition("TRINEX_VULKAN_RHI", "1");
	}

	bool VULKAN_ShaderCompiler::submit_result(ShaderCompilationResult& result)
	{
		bool status = Super::submit_result(result);

		if (status && !result.reflection.vertex_attributes.empty())
		{
			const uint32_t* spirv = reinterpret_cast<uint32_t*>(result.shaders.vertex.data());
			const uint32_t words  = result.shaders.vertex.size() / 4;

			status = strip_vertex_inputs(spirv, words, result.reflection.vertex_attributes);
		}

		return status;
	}

	void D3D12_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_DXIL;
		session->target_desc.profile = g_slang_global_session->findProfile("sm_6_0");

		session->add_definition("TRINEX_INVERT_UV", "1");
		session->add_definition("TRINEX_D3D12_RHI", "1");
		session->add_definition("TRINEX_DIRECT_X_RHI", "1");
	}
}// namespace Engine

#endif
