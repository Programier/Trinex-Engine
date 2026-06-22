#include <Core/definitions.hpp>

#if !PLATFORM_ANDROID
#include <Core/etl/flat_set.hpp>
#include <Core/etl/span.hpp>
#include <Core/etl/templates.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
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

namespace Trinex
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
			static constexpr usize exclude_scalar          = BIT(0);
			static constexpr usize exclude_vector          = BIT(1);
			static constexpr usize exclude_matrix          = BIT(2);
			static constexpr usize exclude_resource        = BIT(3);
			static constexpr usize exclude_sampler         = BIT(4);
			static constexpr usize exclude_struct          = BIT(5);
			static constexpr usize exclude_constant_buffer = BIT(6);

			String name;
			slang::VariableLayoutReflection* var = nullptr;
			const VarTraceEntry* const prev      = nullptr;
			slang::TypeReflection::Kind kind;
			usize exclude_flags;

			VarTraceEntry(slang::VariableLayoutReflection* const var, const VarTraceEntry* const prev = nullptr)
			    : var(var), prev(prev), kind(var->getTypeLayout()->getKind()), exclude_flags(prev ? prev->exclude_flags : 0)
			{
				name = Strings::make_string(var->getName());

				if (prev && !prev->name.empty())
				{
					name = Strings::format("{}.{}", prev->name, name);
				}
			}

			usize trace_offset(SlangParameterCategory category) const
			{
				const VarTraceEntry* current = this;
				usize result                 = 0;

				while (current)
				{
					result += current->var->getOffset(category);
					current = current->prev;
				}

				return result;
			}

			usize trace_offset(slang::ParameterCategory category) const
			{
				return trace_offset(static_cast<SlangParameterCategory>(category));
			}

			usize trace_space(SlangParameterCategory category) const
			{
				const VarTraceEntry* current = this;
				usize result                 = 0;

				while (current)
				{
					result += current->var->getBindingSpace(category);
					current = current->prev;
				}

				return result;
			}

			usize trace_space(slang::ParameterCategory category) const
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
			inline slang::ParameterCategory category(u32 index) const { return var->getCategoryByIndex(index); }
			inline u32 category_count() const { return var->getCategoryCount(); }
			inline bool is_excluded(usize flags) const { return (exclude_flags & flags) == flags; }
			inline slang::VariableLayoutReflection* operator->() const { return var; }
		};

	private:
		ShaderCompilationResult::Reflection* m_reflection;
		Span<Slang::ComPtr<slang::IMetadata>> m_metadatas;

	public:
		using TypeDetector = RHIShaderParameterType(slang::VariableLayoutReflection*, u32, u32, u32,
		                                            slang::TypeReflection::ScalarType);
		static Vector<TypeDetector*> type_detectors;

		static inline StringView parse_string_attribute(slang::UserAttribute* attribute, u32 index)
		{
			usize size;
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

		static bool find_semantic(String name, RHISemantic& out_semantic)
		{
			name = Strings::to_lower(name);

			static const TreeMap<String, RHISemantic> semantics = {
			        {"position", RHISemantic::Position},        //
			        {"texcoord", RHISemantic::TexCoord0},       //
			        {"color", RHISemantic::Color},              //
			        {"normal", RHISemantic::Normal},            //
			        {"tangent", RHISemantic::Tangent},          //
			        {"bitangent", RHISemantic::Bitangent},      //
			        {"blendweight", RHISemantic::BlendWeight},  //
			        {"blendindices", RHISemantic::BlendIndices},//
			        {"userdata", RHISemantic::UserData}         //
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

		inline bool is_variable_used(const VarTraceEntry& var, u8 index)
		{
			using SPC = SlangParameterCategory;

			for (slang::IMetadata* meta : m_metadatas)
			{
				u32 categories = var.category_count();

				for (u32 i = 0; i < categories; ++i)
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

				for (u32 field_index = 0; field_index < fields_count; ++field_index)
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
				RHIInputAttribute attribute;

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
					const usize max_semantic_index = attribute.semantic == RHISemantic::TexCoord0 ? 3 : 0;

					usize index = var->getSemanticIndex();

					if (index > max_semantic_index)
					{
						error_log("ShaderCompiler", "Unsupported semantic index %zu for semantic %s!", index, semantic_name);
						return false;
					}

					attribute.semantic = static_cast<RHISemantic::Enum>(attribute.semantic.value + index);
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

			for (u32 i = 0, count = refl->getBindingRangeCount(); i < count; ++i)
			{
				slang::BindingType binding_type = refl->getBindingRangeType(i);

				switch (binding_type)
				{
					case slang::BindingType::ConstantBuffer: type |= RHIShaderParameterType::META_UniformBuffer; break;
					case slang::BindingType::CombinedTextureSampler: type |= RHIShaderParameterType::META_Texture;
					case slang::BindingType::Sampler: type |= RHIShaderParameterType::META_Sampler; break;

					case slang::BindingType::MutableTexture: type |= RHIShaderParameterType::META_RW;
					case slang::BindingType::Texture: type |= RHIShaderParameterType::META_Texture; break;

					default: return RHIShaderParameterType::Undefined;
				}
			}

			return type;
		}

		bool register_parameter(const RHIShaderParameterInfo& info, RHIShaderParameterType flags = {})
		{
			auto& param = m_reflection->parameters.emplace_back(info);
			param.type |= RHIShaderParameterType::META_Concrete;
			param.type |= flags;
			return true;
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
				return register_parameter(info);
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
						return register_parameter(object, RHIShaderParameterType::META_2D);
					}
					else if (shape_mask == SLANG_TEXTURE_3D)
					{
						return register_parameter(object, RHIShaderParameterType::META_3D);
					}
					else if (shape_mask == SLANG_TEXTURE_CUBE)
					{
						return register_parameter(object, RHIShaderParameterType::META_Cube);
					}
					else if (shape_mask == SLANG_STRUCTURED_BUFFER)
					{
						return register_parameter(object, RHIShaderParameterType::META_StructuredBuffer);
					}
					else if (shape_mask == SLANG_BYTE_ADDRESS_BUFFER)
					{
						return register_parameter(object, RHIShaderParameterType::META_ByteAddressBuffer);
					}
					else if (param.kind == slang::TypeReflection::Kind::SamplerState)
					{
						return register_parameter(object, RHIShaderParameterType::META_Sampler);
					}
				}
			}
			else if (is_in<slang::TypeReflection::Kind::Struct>(param.kind) && !param.is_excluded(VarTraceEntry::exclude_struct))
			{
				auto layout = param.var->getTypeLayout();
				auto fields = layout->getFieldCount();

				usize additional_exclude = 0;

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

						return register_parameter(info);
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
					return register_parameter(object);
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
				u32 parameter_count = entry_point->getParameterCount();
				for (u32 i = 0; i < parameter_count; i++)
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
		static RHIShaderParameterType primitive(SVLR*, u32 rows, u32 columns, u32 elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == 1);
			return_undefined_if_not(elements == 0);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<RHIShaderParameterType type, u32 len, Scalar required_scalar>
		static RHIShaderParameterType vector(SVLR*, u32 rows, u32 columns, u32 elements, Scalar scalar)
		{
			return_undefined_if_not(rows == 1);
			return_undefined_if_not(columns == len);
			return_undefined_if_not(elements == len);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}

		template<RHIShaderParameterType type, Scalar required_scalar, u32 required_rows, u32 required_columns>
		static RHIShaderParameterType matrix(SVLR* var, u32 rows, u32 columns, u32 elements, Scalar scalar)
		{
			return_undefined_if_not(rows == required_rows);
			return_undefined_if_not(rows == required_rows);
			return_undefined_if_not(columns == required_columns);
			return_undefined_if_not(elements == 0);
			return_undefined_if_not(scalar == required_scalar);
			return type;
		}
	};

	trinex_on_pre_init()
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

	trinex_implement_class_default_init(Trinex::SLANG_ShaderCompiler, 0);
	trinex_implement_class_default_init(Trinex::VULKAN_ShaderCompiler, 0);
	trinex_implement_class_default_init(Trinex::NONE_ShaderCompiler, 0);
	trinex_implement_class_default_init(Trinex::D3D12_ShaderCompiler, 0);

	namespace
	{
		struct SlangShaderPermutation {
			Name name;
			Vector<String> specialization_args;
		};

		static bool append_attribute_argument(String& out, slang::Attribute* attribute, u32 index)
		{
			size_t size = 0;

			if (const char* string_value = attribute->getArgumentValueString(index, &size))
			{
				out.append(string_value, size);
				return true;
			}

			int int_value = 0;
			if (attribute->getArgumentValueInt(index, &int_value) == SLANG_OK)
			{
				out += Strings::format("{}", int_value);
				return true;
			}

			return false;
		}

		static bool parse_specialization_attribute(slang::Attribute* attribute, String& out)
		{
			if (attribute == nullptr || attribute->getArgumentCount() == 0)
				return false;

			return append_attribute_argument(out, attribute, 0);
		}

		static bool collect_permutations_from_decl(slang::DeclReflection* decl, Vector<SlangShaderPermutation>& out)
		{
			if (decl == nullptr)
				return true;

			if (auto* type = decl->getType())
			{
				if (auto* pipeline_attribute = type->findUserAttributeByName("trinex_pipeline"))
				{
					if (pipeline_attribute->getArgumentCount() != 1)
					{
						error_log("ShaderCompiler", "trinex_pipeline attribute must have exactly one argument");
						return false;
					}

					SlangShaderPermutation permutation;
					size_t size      = 0;
					const char* name = pipeline_attribute->getArgumentValueString(0, &size);

					if (name == nullptr)
					{
						error_log("ShaderCompiler", "Failed to parse trinex_pipeline attribute");
						return false;
					}

					permutation.name = StringView(name, size);

					for (unsigned int i = 0, count = type->getUserAttributeCount(); i < count; ++i)
					{
						auto* attribute = type->getUserAttributeByIndex(i);

						if (std::strcmp(attribute->getName(), "trinex_specialize") != 0)
							continue;

						auto& specialization_arg = permutation.specialization_args.emplace_back();
						if (!parse_specialization_attribute(attribute, specialization_arg))
						{
							error_log("ShaderCompiler", "Failed to parse trinex_specialize on permutation '%s'",
							          permutation.name.c_str());
							return false;
						}
					}

					out.push_back(std::move(permutation));
				}
			}

			for (auto* child : decl->getChildren())
			{
				if (!collect_permutations_from_decl(child, out))
					return false;
			}

			return true;
		}

	}// namespace

	SLANG_ShaderCompiler::SLANG_ShaderCompiler()
	{
		flags.set(Flags::StandAlone);
		flags.remove(Flags::IsAvailableForGC);

		if (g_slang_global_session == nullptr)
		{
			trinex_verify(SLANG_SUCCEEDED(slang::createGlobalSession(&g_slang_global_session)));
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
		        rootfs()->native_path("[shaders]:/TrinexEditor"),
		        rootfs()->native_path("[shaders]:/TrinexEngine"),
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

	bool SLANG_ShaderCompiler::compile(const ShaderCompilationEnvironment* env, const CompileCallback& callback)
	{
		StackByteAllocator::Mark mark;
		StackVector<slang::IComponentType*> components;
		StackVector<slang::IModule*> source_modules;
		StackVector<Slang::ComPtr<slang::IMetadata>> metadata;
		StackVector<slang::SpecializationArg> specialization_args;
		Vector<SlangShaderPermutation> permutations;

		components.reserve(env->sources_count() + env->modules_count() + 12);
		source_modules.reserve(env->sources_count());

		///////// STEP ONE: COLLECT COMPONENTS /////////
		{
			// Process sources
			for (usize i = 0, count = env->sources_count(); i < count; ++i)
			{
				const char* source = env->source(i);
				static u32 id      = 0;
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
				source_modules.push_back(module);
			}

			// Process modules
			for (usize i = 0, count = env->modules_count(); i < count; ++i)
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

			components.push_back(m_session->loadModule("trinex/trinex.slang"));
		}

		for (slang::IModule* module : source_modules)
		{
			if (!collect_permutations_from_decl(module->getModuleReflection(), permutations))
				return false;
		}

		if (permutations.empty())
		{
			permutations.push_back({});
			permutations.back().name = "Default";
		}

		for (const SlangShaderPermutation& permutation : permutations)
		{
			metadata.clear();
			specialization_args.clear();

			ShaderCompilationResult result;
			result.permutation = permutation.name;

			ShaderInfo shader_infos[] = {
			        {&result.shaders.vertex, "vertex_main"},                            //
			        {&result.shaders.tessellation_control, "tessellation_control_main"},//
			        {&result.shaders.tessellation, "tessellation_main"},                //
			        {&result.shaders.geometry, "geometry_main"},                        //
			        {&result.shaders.fragment, "fragment_main"},                        //
			        {&result.shaders.compute, "compute_main"},                          //
			        {&result.shaders.mesh, "mesh_main"},                                //
			        {&result.shaders.task, "task_main"},                                //
			        {&result.shaders.raygen, "raygen_main"},                            //
			        {&result.shaders.closest_hit, "closest_hit_main"},                  //
			        {&result.shaders.any_hit, "any_hit_main"},                          //
			        {&result.shaders.miss, "miss_main"},                                //
			};

			StackVector<slang::IComponentType*> permutation_components = components;
			const usize module_count                                   = permutation_components.size();

			for (auto& info : shader_infos)
			{
				for (usize module_index = 0; module_index < module_count; ++module_index)
				{
					auto module = static_cast<slang::IModule*>(permutation_components[module_index]);

					slang::IEntryPoint* entry = nullptr;
					module->findEntryPointByName(info.entry_name, &entry);

					if (!entry)
						continue;

					if (info.entry)
					{
						error_log("ShaderCompiler", "Detected multiple entry points with name '%s'", info.entry_name);
						return false;
					}

					info.index = permutation_components.size() - module_count;
					info.entry = entry;
					permutation_components.push_back(entry);
				}
			}

			Slang::ComPtr<slang::IComponentType> composite;
			{
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult slang_result =
				        m_session->createCompositeComponentType(permutation_components.data(), permutation_components.size(),
				                                                composite.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());

				if (SLANG_FAILED(slang_result))
					return false;
			}

			slang::IComponentType* linked_component = composite;
			Slang::ComPtr<slang::IComponentType> specialized_program;
			Slang::ComPtr<slang::IComponentType> program;

			for (const auto& specialization_arg : permutation.specialization_args)
				specialization_args.push_back(slang::SpecializationArg::fromExpr(specialization_arg.c_str()));

			for (usize i = 0, count = env->specialization_args_count(); i < count; ++i)
				specialization_args.push_back(slang::SpecializationArg::fromExpr(env->specialization_arg(i)));

			if (!specialization_args.empty())
			{
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult slang_result = composite->specialize(specialization_args.data(), specialization_args.size(),
				                                                 specialized_program.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());

				if (SLANG_FAILED(slang_result))
					return false;

				linked_component = specialized_program;
			}

			{
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult slang_result = linked_component->link(program.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());

				if (SLANG_FAILED(slang_result))
					return false;
			}

			for (auto& info : shader_infos)
			{
				if (info.index == -1)
					continue;

				Slang::ComPtr<slang::IBlob> code;
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				SlangResult slang_result =
				        program->getEntryPointCode(info.index, 0, code.writeRef(), diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());

				if (SLANG_FAILED(slang_result))
					return false;

				info.source->resize(code->getBufferSize());
				std::memcpy(info.source->data(), code->getBufferPointer(), info.source->size());
				program->getEntryPointMetadata(info.index, 0, metadata.emplace_back().writeRef(), nullptr);
			}

			{
				Slang::ComPtr<slang::IBlob> diagnostics_blob;
				slang::ProgramLayout* reflection = program->getLayout(0, diagnostics_blob.writeRef());

				if (diagnostics_blob != nullptr)
					error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());

				if (!reflection)
				{
					error_log("ShaderCompiler", "Failed to get shader reflection!");
					return false;
				}

				ReflectionParser parser;
				if (!parser.create_reflection(reflection, &result.reflection, metadata))
					return false;
			}

			if (!callback(result))
				return false;
		}

		return true;
	}

	void NONE_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		trinex_unreachable_msg("Something is wrong! Cannot compile shaders for None API!");
	}

	void VULKAN_ShaderCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_SPIRV;
		session->target_desc.profile = g_slang_global_session->findProfile("spirv_1_3");
		session->add_definition("TRINEX_VULKAN_RHI", "1");
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
}// namespace Trinex

#endif
