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

			inline slang::UserAttribute* find_attribute(const char* attribute) const
			{
				return var->getVariable()->findAttributeByName(global_session(), attribute);
			}

			inline bool has_attribute(const char* attribute) { return find_attribute(attribute) != nullptr; }
			inline slang::ParameterCategory category() const { return var->getCategory(); }
			inline bool is_excluded(size_t flags) const { return (exclude_flags & flags) == flags; }
		};


	public:
		using TypeDetector = ShaderParameterType(slang::VariableLayoutReflection*, uint_t, uint_t, uint_t,
												 slang::TypeReflection::ScalarType);
		static Vector<TypeDetector*> type_detectors;

		Pipeline* pipeline;

		static inline StringView parse_string_attribute(slang::UserAttribute* attribute, uint index)
		{
			size_t size;
			const char* name = attribute->getArgumentValueString(0, &size);
			if (name && size > 2)
				return StringView(name + 1, size - 2);
			return StringView();
		}


		static ShaderParameterType find_parameter_type_from_attributes(slang::VariableReflection* var)
		{
			static Map<StringView, ShaderParameterType::Enum> map = {
					{"LocalToWorld", ShaderParameterType::LocalToWorld},      //
					{"Globals", ShaderParameterType::Globals},                //
					{"Surface", ShaderParameterType::Surface},                //
					{"CombinedSurface", ShaderParameterType::CombinedSurface},//
			};

			if (auto attrib = var->findAttributeByName(global_session(), "parameter_type"))
			{
				auto it = map.find(parse_string_attribute(attrib, 0));

				if (it != map.end())
					return it->second;
			}

			return ShaderParameterType::Undefined;
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
			if (category != slang::ParameterCategory::VaryingInput && category != slang::ParameterCategory::Mixed)
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
				attribute.rate           = var->getVariable()->findAttributeByName(global_session(), "per_instance")
												   ? VertexAttributeInputRate::Instance
												   : VertexAttributeInputRate::Vertex;
				attribute.type           = find_vertex_element_type(var->getTypeLayout(), attribute.semantic);
				attribute.location       = var->getOffset(slang::ParameterCategory::VertexInput);
				attribute.stream_index   = find_vertex_stream(var->getVariable(), attribute.location);
				attribute.offset         = find_vertex_offset(var->getVariable());
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

			if (scalar != slang::TypeReflection::ScalarType::None)
			{
				for (auto& detector : type_detectors)
				{
					auto type = detector(var, rows, colums, elements, scalar);
					if (type != ShaderParameterType::Undefined)
					{
						return type;
					}
				}
			}

			return ShaderParameterType::MemoryBlock;
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
			else if (is_in<slang::TypeReflection::Kind::SamplerState>(param.kind) &&
					 !param.is_excluded(VarTraceEntry::exclude_sampler))
			{
				ShaderParameterInfo object;
				object.name                       = param.name;
				object.location                   = param.trace_offset(param.category());
				object.type                       = ShaderParameterType::Sampler;
				pipeline->parameters[object.name] = object;
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

						ShaderParameterInfo info;

						if (auto layout = param.var->getTypeLayout())
						{
							info.size = layout->getSize();
						}
						else
						{
							error_log("ShaderCompiler", "Failed to get parameter layout info!");
							return false;
						}

						info.type                       = ShaderParameterType::MemoryBlock;
						info.name                       = param.name;
						info.offset                     = param.trace_offset(slang::ParameterCategory::Uniform);
						info.location                   = param.trace_offset(slang::ParameterCategory::ConstantBuffer);
						pipeline->parameters[info.name] = info;
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


	class SLANG_CompilationEnv : public ShaderCompilationEnvironment
	{
		SLANG_MaterialCompiler::Context* m_ctx = nullptr;

	public:
		SLANG_CompilationEnv(SLANG_MaterialCompiler::Context* ctx) : m_ctx(ctx) {}

		static const char* copy_str(const char* str)
		{
			auto len   = std::strlen(str) + 1;
			char* copy = FrameAllocator<char>().allocate(len);
			std::memcpy(copy, str, len);
			return copy;
		}

		SLANG_CompilationEnv& add_definition(const char* key, const char* value) override
		{
			m_ctx->definitions.push_back({copy_str(key), copy_str(value)});
			return *this;
		}

		SLANG_CompilationEnv& add_definition_nocopy(const char* key, const char* value) override
		{
			m_ctx->definitions.push_back({key, value});
			return *this;
		}
	};

	SLANG_MaterialCompiler::Context::Context(SLANG_MaterialCompiler* compiler) : compiler(compiler), prev_ctx(compiler->m_ctx)
	{
		compiler->m_ctx = this;
		definitions.reserve(64);
	}

	size_t SLANG_MaterialCompiler::Context::calculate_source_len(const String& source)
	{
		size_t len = source.size();

		for (auto& definition : definitions)
		{
			len += definition.key.size();
			len += definition.value.size();
			len += 10;// "<#define >{key}< >{value}<\n>", 10 is sum of string lengths in <>
		}
		return len;
	}

	char* SLANG_MaterialCompiler::Context::initialize_definitions(char* dst)
	{
		for (auto& definition : definitions)
		{
			std::memcpy(dst, "#define ", 8);
			dst += 8;

			std::memcpy(dst, definition.key.data(), definition.key.size());
			dst += definition.key.size();

			*(dst++) = ' ';

			std::memcpy(dst, definition.value.data(), definition.value.size());
			dst += definition.value.size();

			*(dst++) = '\n';
		}
		return dst;
	}

	bool SLANG_MaterialCompiler::Context::initialize(const String& source, Pipeline* pipeline)
	{
		auto& session = compiler->m_session;

		if (!session)
		{
			error_log("ShaderCompiler", "Failed to compile shader, because compiler session is invalid!");
			return false;
		}

		const char* source_ptr = source.data();

		if (definitions.size() > 0)
		{
			size_t len = calculate_source_len(source);
			char* dst  = FrameAllocator<char>().allocate(len + 1);
			source_ptr = dst;

			dst = initialize_definitions(dst);
			std::memcpy(dst, source.c_str(), source.size());
			dst[source.size()] = '\0';
		}

		String name = Strings::format("Module<{}> '{}'", static_cast<void*>(pipeline), pipeline->name().c_str());
		Slang::ComPtr<slang::IBlob> diagnostics;
		module = session->loadModuleFromSourceString(name.c_str(), name.c_str(), source_ptr, diagnostics.writeRef());

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

		return module != nullptr;
	}

	bool SLANG_MaterialCompiler::Context::compile(ShaderInfo* infos, size_t infos_len, Pipeline* pipeline, CheckStages checker)
	{
		Containers::Vector<slang::IComponentType*, FrameAllocator<slang::IComponentType*>> component_types;
		component_types.push_back(module);

		for (size_t i = 0; i < infos_len; ++i)
		{
			auto& info = infos[i];

			module->findEntryPointByName(info.entry_name, info.entry.writeRef());

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
			SlangResult result = compiler->m_session->createCompositeComponentType(
					component_types.data(), component_types.size(), composite.writeRef(), diagnostics_blob.writeRef());

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

	bool SLANG_MaterialCompiler::Context::compile_graphics(const String& source, Material* material, Refl::RenderPassInfo* pass)
	{
		if (material)
		{
			for (auto& definition : material->compile_definitions)
			{
				definitions.push_back(definition);
			}
		}

		if (pass)
		{
			SLANG_CompilationEnv env(this);
			pass->modify_shader_compilation_env(&env);
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

		if (compile_graphics(source, pipeline))
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

	bool SLANG_MaterialCompiler::Context::compile_graphics(const String& source, Pipeline* pipeline)
	{
		SLANG_CompilationEnv env(this);
		pipeline->modify_compilation_env(&env);

		if (!initialize(source, pipeline))
			return false;

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

	bool SLANG_MaterialCompiler::Context::compile_compute(const String& source, Pipeline* pipeline)
	{
		SLANG_CompilationEnv env(this);
		pipeline->modify_compilation_env(&env);

		if (!initialize(source, pipeline))
			return false;

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
	}

	SLANG_MaterialCompiler& SLANG_MaterialCompiler::on_create()
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

	void SLANG_MaterialCompiler::initialize_context(SessionInitializer* session)
	{
		session->add_option(slang::CompilerOptionName::DisableWarning, "15205");
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
		return ctx.compile_graphics(source, material, pass);
	}

	bool SLANG_MaterialCompiler::compile(const String& source, Pipeline* pipeline)
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

	void NONE_MaterialCompiler::initialize_context(SessionInitializer* session)
	{
		throw EngineException("Something is wrong! Cannot compile shaders for None API!");
	}

	void OPENGL_MaterialCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.profile = global_session()->findProfile("glsl_330");
		session->target_desc.format  = SLANG_SPIRV;
		session->add_definition("TRINEX_OPENGL_RHI", "1");

		session->add_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
		session->add_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_NONE);
		session->add_option(slang::CompilerOptionName::LineDirectiveMode, SLANG_LINE_DIRECTIVE_MODE_NONE);
		session->add_option(slang::CompilerOptionName::EmitSpirvViaGLSL, true);
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

	void VULKAN_MaterialCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format = SLANG_SPIRV;
		session->add_definition("TRINEX_VULKAN_RHI", "1");

		if (Settings::debug_shaders)
		{
			session->add_target_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_NONE);
			session->add_target_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_STANDARD);
			session->add_target_option(slang::CompilerOptionName::LineDirectiveMode, SLANG_LINE_DIRECTIVE_MODE_STANDARD);
			session->add_target_option(slang::CompilerOptionName::EmitSpirvDirectly, true);

			session->target_desc.profile = global_session()->findProfile("spirv_1_3");
		}
		else
		{
			session->add_target_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
			session->add_target_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_NONE);
			session->add_target_option(slang::CompilerOptionName::LineDirectiveMode, SLANG_LINE_DIRECTIVE_MODE_NONE);
			session->add_target_option(slang::CompilerOptionName::EmitSpirvViaGLSL, true);

			session->target_desc.profile = global_session()->findProfile("glsl_330");
		}
	}

	void D3D11_MaterialCompiler::initialize_context(SessionInitializer* session)
	{
		Super::initialize_context(session);

		session->target_desc.format  = SLANG_DXBC;
		session->target_desc.profile = global_session()->findProfile("sm_5_0");

		session->add_definition("TRINEX_INVERT_UV", "1");
		session->add_definition("TRINEX_D3D11_RHI", "1");
		session->add_definition("TRINEX_DIRECT_X_RHI", "1");

		session->add_option(slang::CompilerOptionName::LineDirectiveMode, SLANG_LINE_DIRECTIVE_MODE_NONE);
		session->add_option(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_MAXIMAL);

		if (Settings::debug_shaders)
			session->add_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
		else
			session->add_option(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_NONE);
	}
}// namespace Engine

#endif
