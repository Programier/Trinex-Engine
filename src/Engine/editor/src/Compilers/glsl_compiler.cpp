#include <Compiler/compiler.hpp>
#include <Core/class.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Graphics/visual_material.hpp>

#include <spirv_glsl.hpp>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <iostream>

namespace Engine
{
    static thread_local MessageList* errors = nullptr;

    static const StringView variable_prefix        = "tnx_var_";
    static const StringView global_ubo_name        = "global";
    static const StringView global_variable_prefix = "global.";
    static const StringView local_variable_prefix  = "local.";

    static constexpr inline size_t max_sentence_len = 80;

    static bool compile_shader(const String& text, Buffer& out, EShLanguage lang, std::vector<unsigned int>* uint_out = nullptr)
    {
        glslang::InitializeProcess();

        glslang::TShader shader(lang);
        const char* shader_strings[3];

        String version = text.substr(0, text.find('\n'));

        shader_strings[0] = version.c_str();
        shader_strings[1] = "\n#define SPIRV_VULKAN\n";
        shader_strings[2] = text.c_str() + version.length() + 1;

        shader.setStrings(shader_strings, sizeof(shader_strings) / sizeof(const char*));

        // Set up shader options
        int client_input_semantics_version               = 100;// default for Vulkan
        glslang::EShTargetClientVersion client_version   = glslang::EShTargetVulkan_1_0;
        glslang::EShTargetLanguageVersion target_version = glslang::EShTargetSpv_1_0;

        // Set up shader environment
        shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, client_input_semantics_version);
        shader.setEnvClient(glslang::EShClientVulkan, client_version);
        shader.setEnvTarget(glslang::EShTargetSpv, target_version);


        // Preprocess the shader
        if (!shader.parse(GetDefaultResources(), client_input_semantics_version, false, EShMsgDefault))
        {
            errors->push_back(Strings::format("Compiler: {}", shader.getInfoLog()));
            errors->push_back(Strings::format("Compiler: {}", shader.getInfoDebugLog()));
            glslang::FinalizeProcess();
            return false;
        }

        // Create and link shader program
        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
        {
            errors->push_back(Strings::format("Compiler: {}", shader.getInfoLog()));
            errors->push_back(Strings::format("Compiler: {}", shader.getInfoDebugLog()));
            glslang::FinalizeProcess();
            return false;
        }


        // Translate to SPIR-V
        glslang::SpvOptions spv_options;
        spv_options.generateDebugInfo = true;
        spv_options.disableOptimizer  = false;
        spv_options.optimizeSize      = true;
        spv_options.stripDebugInfo    = false;
        spv_options.validate          = true;
        std::vector<unsigned int> spirv;

        glslang::GlslangToSpv(*program.getIntermediate(lang), spirv, &spv_options);


        out.resize(spirv.size() * sizeof(unsigned int));
        std::memcpy(out.data(), spirv.data(), out.size());

        // Clean up
        glslang::FinalizeProcess();

        if (uint_out)
        {
            *uint_out = std::move(spirv);
        }
        return true;
    }

    static bool is_variable(const StringView& code)
    {
        return code.starts_with(variable_prefix) || code.starts_with(global_variable_prefix) ||
               code.starts_with(local_variable_prefix);
    }

    static const char* default_value_of_base_type(MaterialBaseDataType type, bool is_zero = true)
    {
        if (is_zero)
        {
            switch (type)
            {
                case MaterialBaseDataType::Void:
                    return "";
                case MaterialBaseDataType::Bool:
                    return "false";
                case MaterialBaseDataType::Int:
                    return "0";
                case MaterialBaseDataType::UInt:
                    return "0";
                case MaterialBaseDataType::Float:
                    return "0.0";
                case MaterialBaseDataType::Color:
                    return "0.0";
                default:
                    return "";
            }
        }
        else
        {
            switch (type)
            {
                case MaterialBaseDataType::Void:
                    return "";
                case MaterialBaseDataType::Bool:
                    return "true";
                case MaterialBaseDataType::Int:
                    return "1";
                case MaterialBaseDataType::UInt:
                    return "1";
                case MaterialBaseDataType::Float:
                    return "1.0";
                case MaterialBaseDataType::Color:
                    return "1.0";
                default:
                    return "";
            }
        }
    }

    static String reinterpret_value(void* data, MaterialNodeDataType type)
    {
        if (data == nullptr)
        {
            errors->push_back("Pin of node is empty!");
            return "";
        }

        switch (type)
        {
            case MaterialNodeDataType::Bool:
                return (*reinterpret_cast<bool*>(data)) ? "true" : "false";
            case MaterialNodeDataType::Int:
                return Strings::format("{}", *reinterpret_cast<int_t*>(data));
            case MaterialNodeDataType::UInt:
                return Strings::format("{}", *reinterpret_cast<uint_t*>(data));
            case MaterialNodeDataType::Float:
                return Strings::format("{:f}", *reinterpret_cast<float*>(data));

            case MaterialNodeDataType::BVec2:
                break;
            case MaterialNodeDataType::BVec3:
                break;
            case MaterialNodeDataType::BVec4:
                break;

            case MaterialNodeDataType::IVec2:
            {
                IntVector2D* vec = reinterpret_cast<IntVector2D*>(data);
                return Strings::format("ivec2({}, {})", vec->x, vec->y);
            }

            case MaterialNodeDataType::IVec3:
            {
                IntVector3D* vec = reinterpret_cast<IntVector3D*>(data);
                return Strings::format("ivec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case MaterialNodeDataType::IVec4:
            {
                IntVector4D* vec = reinterpret_cast<IntVector4D*>(data);
                return Strings::format("ivec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case MaterialNodeDataType::UVec2:
            {
                UIntVector2D* vec = reinterpret_cast<UIntVector2D*>(data);
                return Strings::format("uvec2({}, {})", vec->x, vec->y);
            }

            case MaterialNodeDataType::UVec3:
            {
                UIntVector3D* vec = reinterpret_cast<UIntVector3D*>(data);
                return Strings::format("uvec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case MaterialNodeDataType::UVec4:
            {
                UIntVector4D* vec = reinterpret_cast<UIntVector4D*>(data);
                return Strings::format("uvec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case MaterialNodeDataType::Vec2:
            {
                Vector2D* vec = reinterpret_cast<Vector2D*>(data);
                return Strings::format("vec2({}, {})", vec->x, vec->y);
            }

            case MaterialNodeDataType::Vec3:
            {
                Vector3D* vec = reinterpret_cast<Vector3D*>(data);
                return Strings::format("vec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case MaterialNodeDataType::Vec4:
            {
                Vector4D* vec = reinterpret_cast<Vector4D*>(data);
                return Strings::format("vec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case MaterialNodeDataType::Color3:
            {
                Vector3D* vec = reinterpret_cast<Vector3D*>(data);
                return Strings::format("vec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case MaterialNodeDataType::Color4:
            {
                Vector4D* vec = reinterpret_cast<Vector4D*>(data);
                return Strings::format("vec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }
            default:
                throw EngineException("Undefined type");
        }

        return "";
    }

    static const char* glsl_type_of(MaterialNodeDataType type)
    {
        switch (type)
        {
            case MaterialNodeDataType::Bool:
                return "bool";
            case MaterialNodeDataType::Int:
                return "int";
            case MaterialNodeDataType::UInt:
                return "uint";
            case MaterialNodeDataType::Float:
                return "float";
            case MaterialNodeDataType::BVec2:
                return "bvec2";
            case MaterialNodeDataType::BVec3:
                return "bvec3";
            case MaterialNodeDataType::BVec4:
                return "bvec4";
            case MaterialNodeDataType::IVec2:
                return "ivec2";
            case MaterialNodeDataType::IVec3:
                return "ivec3";
            case MaterialNodeDataType::IVec4:
                return "ivec4";
            case MaterialNodeDataType::UVec2:
                return "uvec2";
            case MaterialNodeDataType::UVec3:
                return "uvec3";
            case MaterialNodeDataType::UVec4:
                return "uvec4";
            case MaterialNodeDataType::Vec2:
                return "vec2";
            case MaterialNodeDataType::Vec3:
                return "vec3";
            case MaterialNodeDataType::Vec4:
                return "vec4";
            case MaterialNodeDataType::Color3:
                return "vec3";
            case MaterialNodeDataType::Color4:
                return "vec4";
            case MaterialNodeDataType::Mat3:
                return "mat3";
            case MaterialNodeDataType::Mat4:
                return "mat4";
            default:
            {
                errors->push_back("Failed to get type of node!");
                return "undefined_type";
            }
        }
    }

    static bool need_extens_type(MaterialNodeDataType in_type, MaterialNodeDataType out_type)
    {
        MaterialDataTypeInfo in_info  = MaterialDataTypeInfo::from(in_type);
        MaterialDataTypeInfo out_info = MaterialDataTypeInfo::from(out_type);

        if (in_info.is_matrix() || out_info.is_matrix())
            return false;

        return in_info.components_count != out_info.components_count;
    }

    static String cast_primitive_value(const String& in, MaterialNodeDataType in_type, MaterialNodeDataType out_type)
    {
        MaterialDataTypeInfo in_info  = MaterialDataTypeInfo::from(in_type);
        MaterialDataTypeInfo out_info = MaterialDataTypeInfo::from(out_type);

        if (in_info.components_count != 1 || out_info.components_count != 1)
            throw EngineException("Types in not primitive!");

        return Strings::format("{}({})", glsl_type_of(out_type), in);
    }


    struct GLSL_CompiledSource {
        String source;
        void* data = nullptr;

        GLSL_CompiledSource(const String& source = {}, void* data = nullptr) : source(source), data(data)
        {}

        size_t id() const
        {
            return reinterpret_cast<size_t>(this);
        }
    };

    struct GLSL_Attribute {
        String param;
        byte location;
        MaterialNodeDataType type;
        ColorFormat format;

        VertexBufferSemantic semantic;
        byte semantic_index = 0;
        bool with_prefix    = true;
    };

    struct GLSL_LocalParameter {
        String param;
        MaterialNodeDataType type;
        MaterialParameter::Type material_parameter_type;
        void* data = nullptr;
    };

    struct GLSL_BindingObject {
        String name;
        void* texture = nullptr;
        void* sampler = nullptr;
        StringView type;
        BindingIndex index;
    };

    class GLSL_BaseCompiler : public ShaderCompiler
    {
    public:
        VisualMaterial* material = nullptr;
        Vector<GLSL_Attribute> input_attribute;
        Vector<GLSL_Attribute> output_attribute;

        Vector<GLSL_LocalParameter> local_parameters;

        Vector<GLSL_BindingObject*> objects;
        Map<Identifier, GLSL_CompiledSource*> compiled_pins;
        Vector<String> statements;
        byte next_binding_index = 2;

        String text_source;
        Buffer binary_source;
        std::vector<unsigned int> spirv;

        List<Function<void()>> on_success_command_list;

        ~GLSL_BaseCompiler()
        {
            for (auto& ell : objects)
            {
                delete ell;
            }

            for (auto& [name, source] : compiled_pins)
            {
                delete source;
            }
        }

        // Helpers

        GLSL_Attribute* find_input_attribute(const StringView& name)
        {
            for (auto& input : input_attribute)
            {
                if (input.param == name)
                    return &input;
            }
            return nullptr;
        }

        GLSL_Attribute* find_output_attribute(const StringView& name)
        {
            for (auto& output : output_attribute)
            {
                if (output.param == name)
                    return &output;
            }
            return nullptr;
        }

        Index create_input_attribute(const String& name, bool* created_now = nullptr)
        {
            if (created_now)
                *created_now = false;

            Index index = 0;
            for (auto& input : input_attribute)
            {
                if (input.param == name)
                    return index;
                ++index;
            }

            if (created_now)
                *created_now = true;

            input_attribute.emplace_back();
            auto& attrib    = input_attribute.back();
            attrib.param    = name;
            attrib.location = static_cast<byte>(index);

            return index;
        }

        Index create_output_attribute(const String& name, bool* created_now = nullptr, bool with_refix = true)
        {
            if (created_now)
                *created_now = false;

            Index index = 0;
            for (auto& output : output_attribute)
            {
                if (output.param == name)
                    return index;
                ++index;
            }

            if (created_now)
                *created_now = true;

            output_attribute.emplace_back();
            auto& attrib       = output_attribute.back();
            attrib.param       = name;
            attrib.with_prefix = with_refix;
            attrib.location    = static_cast<byte>(index);

            return index;
        }

        GLSL_LocalParameter* find_local_parameter(const StringView& name)
        {
            for (auto& local : local_parameters)
            {
                if (local.param == name)
                {
                    return &local;
                }
            }

            return nullptr;
        }

        Index create_local_parameter(const String& name, MaterialNodeDataType type, bool* created_now = nullptr)
        {
            if (created_now)
                *created_now = false;

            Index index = 0;
            for (auto& local : local_parameters)
            {
                if (local.param == name)
                {
                    if (local.type != type)
                    {
                        errors->push_back(
                                Strings::format("Parameter with name {} already exist, but have different type!", name));
                    }
                    return index;
                }
                ++index;
            }

            if (created_now)
                *created_now = true;

            local_parameters.emplace_back();
            auto& param = local_parameters.back();
            param.param = name;
            param.type  = type;

            return index;
        }

        GLSL_BindingObject* create_binding_object(void* texture, void* sampler, StringView type, bool& created_now)
        {
            created_now = false;
            for (GLSL_BindingObject* glsl_object : objects)
            {
                if (glsl_object->texture == texture && glsl_object->sampler == sampler)
                {
                    return glsl_object;
                }
            }

            created_now = true;

            GLSL_BindingObject* new_object = new GLSL_BindingObject();
            new_object->texture            = texture;
            new_object->sampler            = sampler;
            new_object->type               = type;
            new_object->index              = next_binding_index;
            new_object->name               = Strings::format("tnx_object_{}", objects.size());
            objects.push_back(new_object);

            ++next_binding_index;
            return new_object;
        }

        void create_variable(GLSL_CompiledSource* source, MaterialNodeDataType type)
        {
            if (!is_variable(source->source))
            {
                String variable_name = Strings::format("{}{}", variable_prefix, statements.size());
                String new_source    = Strings::format("{} {} = {}", glsl_type_of(type), variable_name, source->source);
                statements.push_back(new_source);
                source->source = std::move(variable_name);
            }
        }

        StringView create_variable(const StringView& code, MaterialNodeDataType type)
        {
            if (!is_variable(code))
            {
                String variable_name  = Strings::format("{}{}", variable_prefix, statements.size());
                const char* type_name = glsl_type_of(type);
                String new_source     = Strings::format("{} {} = {}", type_name, variable_name, code);
                statements.push_back(new_source);

                const char* var_start = statements.back().c_str() + std::strlen(type_name) + 1;
                return StringView(var_start, variable_name.length());
            }

            return code;
        }

        GLSL_CompiledSource* find_pin_source(Identifier id, bool create = false)
        {
            auto it = compiled_pins.find(id);
            if (it != compiled_pins.end())
                return it->second;

            if (create)
            {
                GLSL_CompiledSource* new_source = new GLSL_CompiledSource();
                compiled_pins[id]               = new_source;
                return new_source;
            }
            return nullptr;
        }

        GLSL_CompiledSource* pin_source(MaterialOutputPin* pin)
        {
            // Try to find it
            GLSL_CompiledSource* source = find_pin_source(pin->id(), false);
            if (source)
                return source;

            source                   = reinterpret_cast<GLSL_CompiledSource*>(pin->node->compile(this, pin));
            compiled_pins[pin->id()] = source;
            return source;
        }

        GLSL_CompiledSource* pin_source(MaterialInputPin* pin)
        {
            if (pin->linked_to)
            {
                return pin_source(pin->linked_to);
            }

            // Try to find it
            GLSL_CompiledSource* source = find_pin_source(pin->id(), false);
            if (source)
                return source;

            void* default_value = pin->default_value();
            if (!default_value)
                return nullptr;

            String code              = reinterpret_value(default_value, pin->value_type());
            source                   = new GLSL_CompiledSource(code);
            compiled_pins[pin->id()] = source;
            return source;
        }

        GLSL_CompiledSource* sampler_source(MaterialInputPin* pin)
        {
            return pin_source(pin);
        }

        String uv_source(MaterialInputPin* pin)
        {
            return get_pin_source(pin, MaterialNodeDataType::Vec2);
        }

        String cast_value(const String& in, MaterialNodeDataType in_type, MaterialNodeDataType out_type)
        {
            if (is_equal_types(in_type, out_type) || out_type == MaterialNodeDataType::Undefined)
                return in;

            MaterialDataTypeInfo in_info  = MaterialDataTypeInfo::from(in_type);
            MaterialDataTypeInfo out_info = MaterialDataTypeInfo::from(out_type);

            if (in_info.components_count == 1 && out_info.components_count == 1)
            {
                return cast_primitive_value(in, in_type, out_type);
            }

            if (need_extens_type(in_type, out_type))
            {
                String out = Strings::format("{}(", glsl_type_of(out_type));

                if (in_info.components_count == 1)
                {
                    StringView code = create_variable(in, in_type);

                    out += code;
                    for (size_t i = 1, j = glm::min(out_info.components_count, static_cast<size_t>(3)); i < j; ++i)
                    {
                        out = Strings::format("{}, {}", out, code);
                    }

                    if (out_info.components_count == 4)
                        out = Strings::format("{}, {}", out, default_value_of_base_type(out_info.base_type, false));
                }
                else
                {
                    char components[]                                                             = ".xyzw";
                    components[glm::min(in_info.components_count, out_info.components_count) + 1] = 0;

                    // Maybe compiled source already have swizzling?
                    if (in.ends_with(components))
                    {
                        out += in;
                    }
                    else
                    {
                        const char* in_type_name = glsl_type_of(in_type);
                        bool is_typed_variable   = (in.starts_with(in_type_name) && in.ends_with(')')) || is_variable(in);

                        if (is_typed_variable)
                        {
                            out += Strings::format("{}{}", in, components);
                        }
                        else
                        {
                            out += Strings::format("{}({}){}", in_type_name, in, components);
                        }
                    }

                    for (size_t i = in_info.components_count; i < out_info.components_count; ++i)
                    {
                        out = Strings::format("{}, {}", out, default_value_of_base_type(out_info.base_type, i < 3));
                    }
                }

                out.push_back(')');
                return out;
            }

            // will be used for matrices
            return Strings::format("{}({})", glsl_type_of(out_type), in);
        }

        String get_pin_source(MaterialOutputPin* pin, MaterialNodeDataType out_type = MaterialNodeDataType::Undefined,
                              bool force_as_var = false)
        {
            auto source = pin_source(pin);

            if (pin->refereces_count() > 1 || source->source.length() > max_sentence_len || force_as_var)
            {
                // Compile this source to variable
                create_variable(source, pin->value_type());
            }

            return cast_value(source->source, pin->node->output_type(pin), out_type);
        }

        String get_pin_source(MaterialInputPin* pin, MaterialNodeDataType out_type = MaterialNodeDataType::Undefined,
                              bool force_as_var = false)
        {
            auto source = pin_source(pin);

            if (source->source.length() > max_sentence_len || force_as_var)
            {
                // Compile this source to variable
                create_variable(source, pin->value_type());
            }

            auto type = pin->value_type();
            return cast_value(source->source, type, out_type);
        }

        bool compile(VisualMaterial* material, MessageList& list) override
        {
            this->material = material;
            root_node()->compile(this, nullptr);
            return errors->empty();
        }

        void parse_reflection()
        {
            spirv_cross::CompilerGLSL compiler(spirv);
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();


            for (const auto& uniform : resources.uniform_buffers)
            {
                auto& type = compiler.get_type(uniform.base_type_id);
                if (compiler.get_name(type.self) != "_Local")
                    continue;

                for (uint32_t i = 0; i < type.member_types.size(); ++i)
                {
                    String member_name     = compiler.get_member_name(uniform.base_type_id, i);
                    uint32_t member_offset = compiler.get_member_decoration(uniform.base_type_id, i, spv::DecorationOffset);

                    GLSL_LocalParameter* parameter = find_local_parameter(member_name);

                    if (parameter)
                    {
                        Name name = member_name;
                        MaterialParameter* material_parameter =
                                material->create_parameter(name, parameter->material_parameter_type);
                        material->pipeline->local_parameters.update(name, member_offset);

                        void* dest  = material_parameter->data();
                        void* src   = parameter->data;
                        size_t size = material_parameter->size();

                        if (dest && src && size > 0)
                        {
                            std::memcpy(dest, src, size);
                        }
                    }
                }
            }
        }

        bool submit_source()
        {
            for (auto& func : on_success_command_list)
            {
                func();
            }

            current_shader()->text_code   = std::move(text_source);
            current_shader()->binary_code = std::move(binary_source);
            return true;
        }

        bool build_source()
        {
            String source = "#version 310 es\n"
                            "precision highp float;\n"
                            "\n";

            for (auto& attribute : input_attribute)
            {
                if (attribute.with_prefix)
                {
                    source += Strings::format("layout(location = {}) in {} in_{};\n", attribute.location,
                                              glsl_type_of(attribute.type), attribute.param);
                }
                else
                {
                    source += Strings::format("layout(location = {}) in {} {};\n", attribute.location,
                                              glsl_type_of(attribute.type), attribute.param);
                }
            }

            source += "\n";

            for (auto& attribute : output_attribute)
            {
                if (attribute.with_prefix)
                {
                    source += Strings::format("layout(location = {}) out {} out_{};\n", attribute.location,
                                              glsl_type_of(attribute.type), attribute.param);
                }
                else
                {
                    source += Strings::format("layout(location = {}) out {} {};\n", attribute.location,
                                              glsl_type_of(attribute.type), attribute.param);
                }
            }

            source += "\n";
            source += GlobalShaderParameters::shader_code();
            source.push_back(' ');
            source += global_ubo_name;
            source += ";\n\n";

            if (!local_parameters.empty())
            {
                source += "layout(binding = 1, std140) uniform _Local\n{\n";

                for (auto& local : local_parameters)
                {
                    source.push_back('\t');
                    source += Strings::format("{} {};\n", glsl_type_of(local.type), local.param);
                }

                source += "} local;\n\n";
            }

            for (auto& object : objects)
            {
                source += Strings::format("layout(binding = {}) uniform {} {};\n", object->index, object->type, object->name);
            }

            source += "\n\nvoid main()\n{\n";

            for (auto& statement : statements)
            {
                source += "\t";
                source += statement;
                source += ";\n";
            }

            source += "}\n";

            compile_shader(source, binary_source, shader_type(), &spirv);
            text_source = std::move(source);
            return errors->empty();
        }

        // HELPRES END


        /// GLOBALS

        size_t projection() override
        {
            return (new GLSL_CompiledSource("global.projection"))->id();
        }

        size_t view() override
        {
            return (new GLSL_CompiledSource("global.view"))->id();
        }

        size_t projview() override
        {
            return (new GLSL_CompiledSource("global.projview"))->id();
        }

        size_t inv_projview() override
        {
            return (new GLSL_CompiledSource("global.inv_projview"))->id();
        }

        size_t model() override
        {
            bool created_now;
            Index index                = create_local_parameter("model_matrix", MaterialNodeDataType::Mat4, &created_now);
            GLSL_LocalParameter& param = local_parameters[index];

            if (created_now)
            {
                param.data                    = nullptr;
                param.material_parameter_type = MaterialParameter::Type::ModelMatrix;
            }

            return (new GLSL_CompiledSource("local.model_matrix"))->id();
        }

        size_t camera_location() override
        {
            return (new GLSL_CompiledSource("global.camera_location"))->id();
        }

        size_t time() override
        {
            return (new GLSL_CompiledSource("global.time"))->id();
        }

        size_t gamma() override
        {
            return (new GLSL_CompiledSource("global.gamma"))->id();
        }

        size_t delta_time() override
        {
            return (new GLSL_CompiledSource("global.delta_time"))->id();
        }

        size_t fov() override
        {
            return (new GLSL_CompiledSource("global.fov"))->id();
        }

        size_t ortho_width() override
        {
            return (new GLSL_CompiledSource("global.ortho_width"))->id();
        }

        size_t ortho_height() override
        {
            return (new GLSL_CompiledSource("global.ortho_height"))->id();
        }

        size_t near_clip_plane() override
        {
            return (new GLSL_CompiledSource("global.near_clip_plane"))->id();
        }

        size_t far_clip_plane() override
        {
            return (new GLSL_CompiledSource("global.far_clip_plane"))->id();
        }

        size_t aspect_ratio() override
        {
            return (new GLSL_CompiledSource("global.aspect_ratio"))->id();
        }

        size_t camera_projection_mode() override
        {
            return (new GLSL_CompiledSource("global.camera_projection_mode"))->id();
        }

        size_t frag_coord() override
        {
            errors->push_back(Strings::format("FragCoord node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t render_target_size() override
        {
            return (new GLSL_CompiledSource("global.size"))->id();
        }

        /// CONSTANTS
#define declare_constant_value_method(name, type)                                                                                \
    size_t name##_constant(void* value) override                                                                                 \
    {                                                                                                                            \
        return (new GLSL_CompiledSource(reinterpret_value(value, MaterialNodeDataType::type)))->id();                            \
    }
        declare_constant_value_method(bool, Bool);
        declare_constant_value_method(int, Int);
        declare_constant_value_method(uint, UInt);
        declare_constant_value_method(float, Float);
        declare_constant_value_method(bvec2, BVec2);
        declare_constant_value_method(bvec3, BVec3);
        declare_constant_value_method(bvec4, BVec4);
        declare_constant_value_method(ivec2, IVec2);
        declare_constant_value_method(ivec3, IVec3);
        declare_constant_value_method(ivec4, IVec4);
        declare_constant_value_method(uvec2, UVec2);
        declare_constant_value_method(uvec3, UVec3);
        declare_constant_value_method(uvec4, UVec4);
        declare_constant_value_method(vec2, Vec2);
        declare_constant_value_method(vec3, Vec3);
        declare_constant_value_method(vec4, Vec4);
        declare_constant_value_method(color3, Color3);
        declare_constant_value_method(color4, Color4);

        /// DYNAMIC PARAMETERS

#define declare_dynamic_parameter(param, type)                                                                                   \
    size_t param##_parameter(const String& name, void* data) override                                                            \
    {                                                                                                                            \
        GLSL_LocalParameter& parameter    = local_parameters[create_local_parameter(name, MaterialNodeDataType::type)];          \
        parameter.data                    = data;                                                                                \
        parameter.material_parameter_type = MaterialParameter::Type::type;                                                       \
        return (new GLSL_CompiledSource(Strings::format("{}{}", local_variable_prefix, name)))->id();                            \
    }

        declare_dynamic_parameter(bool, Bool);
        declare_dynamic_parameter(int, Int);
        declare_dynamic_parameter(uint, UInt);
        declare_dynamic_parameter(float, Float);
        declare_dynamic_parameter(bvec2, BVec2);
        declare_dynamic_parameter(bvec3, BVec3);
        declare_dynamic_parameter(bvec4, BVec4);
        declare_dynamic_parameter(ivec2, IVec2);
        declare_dynamic_parameter(ivec3, IVec3);
        declare_dynamic_parameter(ivec4, IVec4);
        declare_dynamic_parameter(uvec2, UVec2);
        declare_dynamic_parameter(uvec3, UVec3);
        declare_dynamic_parameter(uvec4, UVec4);
        declare_dynamic_parameter(vec2, Vec2);
        declare_dynamic_parameter(vec3, Vec3);
        declare_dynamic_parameter(vec4, Vec4);
        declare_dynamic_parameter(color3, Vec3);
        declare_dynamic_parameter(color4, Vec4);


        /// MATH
        size_t sin(MaterialInputPin* pin) override
        {
            return (new GLSL_CompiledSource(Strings::format("sin({})", get_pin_source(pin))))->id();
        }

        size_t cos(MaterialInputPin* pin) override
        {
            return (new GLSL_CompiledSource(Strings::format("cos({})", get_pin_source(pin))))->id();
        }

        size_t tan(MaterialInputPin* pin) override
        {
            return (new GLSL_CompiledSource(Strings::format("tan({})", get_pin_source(pin))))->id();
        }

        size_t dot(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            String source = Strings::format("dot({}, {})", get_pin_source(pin1, MaterialNodeDataType::Vec3),
                                            get_pin_source(pin2, MaterialNodeDataType::Vec3));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t normalize(MaterialInputPin* pin) override
        {
            String source = Strings::format("normalize({})", get_pin_source(pin));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t pow(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            String source = Strings::format("pow({}, {})", get_pin_source(pin1), get_pin_source(pin2, pin1->value_type()));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t floor(MaterialInputPin* pin1) override
        {
            String source = Strings::format("floor({})", get_pin_source(pin1));
            return (new GLSL_CompiledSource(source))->id();
        }


        /// OPERATORS
#define declare_type_operator(name, type)                                                                                        \
    size_t name##_op(MaterialInputPin* pin) override                                                                             \
    {                                                                                                                            \
        String source = get_pin_source(pin, MaterialNodeDataType::type);                                                         \
        return (new GLSL_CompiledSource(source))->id();                                                                          \
    }

        declare_type_operator(bool, Bool);
        declare_type_operator(int, Int);
        declare_type_operator(uint, UInt);
        declare_type_operator(float, Float);
        declare_type_operator(bvec2, BVec2);
        declare_type_operator(bvec3, BVec3);
        declare_type_operator(bvec4, BVec4);
        declare_type_operator(ivec2, IVec2);
        declare_type_operator(ivec3, IVec3);
        declare_type_operator(ivec4, IVec4);
        declare_type_operator(uvec2, UVec2);
        declare_type_operator(uvec3, UVec3);
        declare_type_operator(uvec4, UVec4);
        declare_type_operator(vec2, Vec2);
        declare_type_operator(vec3, Vec3);
        declare_type_operator(vec4, Vec4);
        declare_type_operator(color3, Color3);
        declare_type_operator(color4, Color4);

        size_t add(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = calculate_operator_types(t1, t2);

            auto source = (new GLSL_CompiledSource(Strings::format("{}({} + {})", glsl_type_of(result_type),
                                                                   get_pin_source(pin1, t1), get_pin_source(pin2, t2))));
            return source->id();
        }

        size_t sub(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = calculate_operator_types(t1, t2);

            auto source = (new GLSL_CompiledSource(Strings::format("{}({} - {})", glsl_type_of(result_type),
                                                                   get_pin_source(pin1, t1), get_pin_source(pin2, t2))));
            return source->id();
        }

        size_t mul(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin2->value_type();

            MaterialNodeDataType result_type = calculate_operator_types(t1, t2);
            auto source                      = (new GLSL_CompiledSource(Strings::format("{}({} * {})", glsl_type_of(result_type),
                                                                                        get_pin_source(pin1, t1), get_pin_source(pin2, t2))));
            return source->id();
        }

        size_t div(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = calculate_operator_types(t1, t2);
            auto source                      = (new GLSL_CompiledSource(Strings::format("{}({} / {})", glsl_type_of(result_type),
                                                                                        get_pin_source(pin1, t1), get_pin_source(pin2, t2))));
            return source->id();
        }

        size_t construct_vec2(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto source = Strings::format("vec2({}, {})", get_pin_source(pin1, MaterialNodeDataType::Float),
                                          get_pin_source(pin2, MaterialNodeDataType::Float));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t construct_vec3(MaterialInputPin* pin1, MaterialInputPin* pin2, MaterialInputPin* pin3) override
        {
            auto source = Strings::format("vec3({}, {}, {})", get_pin_source(pin1, MaterialNodeDataType::Float),
                                          get_pin_source(pin2, MaterialNodeDataType::Float),
                                          get_pin_source(pin3, MaterialNodeDataType::Float));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t construct_vec4(MaterialInputPin* pin1, MaterialInputPin* pin2, MaterialInputPin* pin3,
                              MaterialInputPin* pin4) override
        {
            auto source = Strings::format("vec4({}, {}, {}, {})", get_pin_source(pin1, MaterialNodeDataType::Float),
                                          get_pin_source(pin2, MaterialNodeDataType::Float),
                                          get_pin_source(pin3, MaterialNodeDataType::Float),
                                          get_pin_source(pin4, MaterialNodeDataType::Float));
            return (new GLSL_CompiledSource(source))->id();
        }

        size_t decompose_vec(MaterialInputPin* pin, DecomposeVectorComponent component) override
        {
            Index component_index                = static_cast<Index>(component);
            MaterialNodeDataType input_type      = pin->value_type();
            MaterialDataTypeInfo input_type_info = MaterialDataTypeInfo::from(input_type);

            MaterialNodeDataType vector_type =
                    static_cast<MaterialNodeDataType>(material_type_value(input_type_info.base_type, 4));

            GLSL_CompiledSource* input_source = pin_source(pin);
            GLSL_CompiledSource* pin_source   = find_pin_source(pin->id(), true);

            if (pin_source != input_source)
            {
                pin_source->source = input_source->source;
            }

            pin_source->source = cast_value(pin_source->source, input_type, vector_type);
            create_variable(pin_source, vector_type);

            static char components[] = "xyzw";
            String out_source        = Strings::format("{}.{}", pin_source->source, components[component_index]);

            return (new GLSL_CompiledSource(out_source))->id();
        }


        // TEXTURES
        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t base_color_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t position_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t normal_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t emissive_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t data_buffer_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t scene_output_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            errors->push_back(Strings::format("Texture2D node doesn't supported in {} shader!", name()));
            return 0;
        }


        virtual size_t sampler(class Engine::Sampler* sampler) override
        {
            errors->push_back(Strings::format("Sampler node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_base_color(MaterialInputPin* pin) override
        {
            errors->push_back(Strings::format("BaseColor pin doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_metalic(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Metalic node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_specular(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Specular doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_roughness(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Roughness pin doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_emissive(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Emissive pin doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_opacity(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Opacity pin doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_position(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position pin doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t fragment_output_normal(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Normal pin doesn't supported in {} shader!", name()));
            return 0;
        }

        virtual void submit_vertex_attribute(const GLSL_Attribute& attribute)
        {}

        virtual size_t create_vertex_attribute(const StringView& name, byte index, ColorFormat format, MaterialNodeDataType type,
                                               VertexBufferSemantic semantic)
        {
            String code = Strings::format("{}_{}", name, index);

            bool created_now;
            auto& in_attribute = input_attribute[create_input_attribute(code, &created_now)];

            if (created_now)
            {
                in_attribute.format         = format;
                in_attribute.type           = type;
                in_attribute.semantic       = semantic;
                in_attribute.semantic_index = index;

                submit_vertex_attribute(in_attribute);
            }

            return (new GLSL_CompiledSource(Strings::format("in_{}", code)))->id();
        }

        virtual size_t vertex_position_attribute(byte index) override
        {
            return create_vertex_attribute("position", index, ColorFormat::R32G32B32Sfloat, MaterialNodeDataType::Vec3,
                                           VertexBufferSemantic::Position);
        }

        virtual size_t vertex_normal_attribute(byte index) override
        {
            return create_vertex_attribute("normal", index, ColorFormat::R32G32B32Sfloat, MaterialNodeDataType::Vec3,
                                           VertexBufferSemantic::Normal);
        }

        virtual size_t vertex_uv_attribute(byte index) override
        {
            return create_vertex_attribute("tex_coord", index, ColorFormat::R32G32Sfloat, MaterialNodeDataType::Vec2,
                                           VertexBufferSemantic::TexCoord);
        }

        size_t vertex_output_screen_space_position(MaterialInputPin* pin) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_world_position(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_uv0(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_uv1(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_world_normal(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_world_tangent(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_world_bitangent(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t vertex_output_color(MaterialInputPin*) override
        {
            errors->push_back(Strings::format("Position node doesn't supported in {} shader!", name()));
            return 0;
        }

        virtual const char* name() const        = 0;
        virtual Shader* current_shader() const  = 0;
        virtual MaterialNode* root_node() const = 0;
        virtual EShLanguage shader_type() const = 0;
    };

    class GLSL_VertexCompiler : public GLSL_BaseCompiler
    {
    public:
        const char* name() const override
        {
            return "Vertex Compiler";
        }

        VertexShader* current_shader() const override
        {
            return material->pipeline->vertex_shader;
        }

        MaterialNode* root_node() const override
        {
            return material->vertex_node();
        }

        EShLanguage shader_type() const override
        {
            return EShLangVertex;
        }

        bool compile(VisualMaterial* material, MessageList& errors) override
        {
            RenderPassType type = material->pipeline->render_pass;

            if (type == RenderPassType::Undefined)
            {
                errors.push_back("Undefined render pass type! Please, select render pass type!");
                return false;
            }


            if (!GLSL_BaseCompiler::compile(material, errors))
                return false;

            on_success_command_list.push_back([this]() {
                current_shader()->attributes.clear();

                for (auto& input : input_attribute)
                {
                    current_shader()->attributes.push_back(VertexShader::Attribute(input.format, VertexAttributeInputRate::Vertex,
                                                                                   input.semantic, input.semantic_index, 1,
                                                                                   input.param));
                }
            });

            return true;
        }

        void submit_vertex_attribute(const GLSL_Attribute& attribute) override
        {
            bool created_now;
            auto& out_attribute = output_attribute[create_output_attribute(attribute.param, &created_now)];

            if (created_now)
            {
                out_attribute.format = attribute.format;
                out_attribute.type   = attribute.type;
            }

            statements.push_back(Strings::format("out_{} = in_{}", attribute.param, attribute.param));
        }

        size_t vertex_output_screen_space_position(MaterialInputPin* pin) override
        {
            String source = "";

            if (pin->linked_to)
            {
                source = get_pin_source(pin, MaterialNodeDataType::Vec4);
            }
            else
            {
                GLSL_CompiledSource* model_source       = reinterpret_cast<GLSL_CompiledSource*>(model());
                GLSL_CompiledSource* compiled_attribute = reinterpret_cast<GLSL_CompiledSource*>(vertex_position_attribute(0));
                source =
                        Strings::format("global.projview * {} * vec4({}, 1.0)", model_source->source, compiled_attribute->source);
            }

            statements.push_back(Strings::format("gl_Position = {}", source));
            return 0;
        }

        size_t vertex_output_world_position(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_world_position", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32B32Sfloat;
                output.type   = MaterialNodeDataType::Vec3;
            }

            statements.push_back(Strings::format("vertex_world_position = {}", get_pin_source(pin, MaterialNodeDataType::Vec3)));
            return 0;
        }

        size_t vertex_output_uv0(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_uv0", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32Sfloat;
                output.type   = MaterialNodeDataType::Vec2;
            }

            statements.push_back(Strings::format("vertex_uv0 = {}", get_pin_source(pin, MaterialNodeDataType::Vec2)));
            return 0;
        }

        size_t vertex_output_uv1(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_uv1", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32Sfloat;
                output.type   = MaterialNodeDataType::Vec2;
            }

            statements.push_back(Strings::format("vertex_uv1 = {}", get_pin_source(pin, MaterialNodeDataType::Vec2)));
            return 0;
        }

        size_t vertex_output_world_normal(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_world_normal", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32B32Sfloat;
                output.type   = MaterialNodeDataType::Vec3;
            }

            statements.push_back(Strings::format("vertex_world_normal = {}", get_pin_source(pin, MaterialNodeDataType::Vec3)));
            return 0;
        }

        size_t vertex_output_world_tangent(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_world_tangent", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32B32Sfloat;
                output.type   = MaterialNodeDataType::Vec3;
            }

            statements.push_back(Strings::format("vertex_world_tangent = {}", get_pin_source(pin, MaterialNodeDataType::Vec3)));
            return 0;
        }

        size_t vertex_output_world_bitangent(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_world_bitangent", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32B32Sfloat;
                output.type   = MaterialNodeDataType::Vec3;
            }

            statements.push_back(Strings::format("vertex_world_bitangent = {}", get_pin_source(pin, MaterialNodeDataType::Vec3)));
            return 0;
        }

        size_t vertex_output_color(MaterialInputPin* pin) override
        {
            bool created_now;
            auto& output = output_attribute[create_output_attribute("vertex_color", &created_now, false)];

            if (created_now)
            {
                output.format = ColorFormat::R32G32B32Sfloat;
                output.type   = MaterialNodeDataType::Vec3;
            }

            statements.push_back(Strings::format("vertex_color = {}", get_pin_source(pin, MaterialNodeDataType::Vec3)));
            return 0;
        }
    };

    class GLSL_FragmentCompiler : public GLSL_BaseCompiler
    {
        GLSL_VertexCompiler* vertex_compiler = nullptr;

    public:
        GLSL_FragmentCompiler(GLSL_VertexCompiler* vertex_compiler) : vertex_compiler(vertex_compiler)
        {}

        const char* name() const override
        {
            return "Fragment Compiler";
        }

        Shader* current_shader() const override
        {
            return material->pipeline->fragment_shader;
        }

        MaterialNode* root_node() const override
        {
            return material->fragment_node();
        }

        EShLanguage shader_type() const override
        {
            return EShLangFragment;
        }

        void sync(GLSL_VertexCompiler* vertex_compiler)
        {
            next_binding_index = vertex_compiler->next_binding_index;

            for (auto& output : vertex_compiler->output_attribute)
            {
                input_attribute.push_back(output);
            }
        }

        bool compile(VisualMaterial* material, MessageList& errors) override
        {
            RenderPassType type = material->pipeline->render_pass;

            if (type == RenderPassType::Undefined)
            {
                errors.push_back("Undefined render pass type! Please, select render pass type!");
                return false;
            }

            GLSL_Attribute attribute;
            attribute.location = 0;
            attribute.param    = "color";
            attribute.type     = MaterialNodeDataType::Color4;

            output_attribute.push_back(attribute);

            if (type == RenderPassType::GBuffer)
            {
                attribute.location = 1;
                attribute.param    = "position";
                attribute.type     = MaterialNodeDataType::Color4;
                output_attribute.push_back(attribute);

                attribute.location = 2;
                attribute.param    = "normal";
                attribute.type     = MaterialNodeDataType::Color4;
                output_attribute.push_back(attribute);

                attribute.location = 3;
                attribute.param    = "emissive";
                attribute.type     = MaterialNodeDataType::Color4;
                output_attribute.push_back(attribute);

                attribute.location = 4;
                attribute.param    = "data_buffer";
                attribute.type     = MaterialNodeDataType::Color4;
                output_attribute.push_back(attribute);
            }


            return GLSL_BaseCompiler::compile(material, errors);
        }

        size_t frag_coord() override
        {
            return (new GLSL_CompiledSource("gl_FragCoord.xy"))->id();
        }

        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            bool created_now;
            GLSL_CompiledSource* sampler_src = sampler_source(sampler);
            String uv_src                    = uv_source(uv);

            GLSL_BindingObject* glsl_texture = create_binding_object(texture, sampler_src->data, "sampler2D", created_now);

            if (created_now)
            {
                on_success_command_list.push_back([this, sampler_src, glsl_texture, texture]() {
                    CombinedTexture2DMaterialParameter* param = reinterpret_cast<CombinedTexture2DMaterialParameter*>(
                            material->create_parameter(glsl_texture->name, MaterialParameter::Type::CombinedTexture2D));
                    param->texture  = texture;
                    param->sampler  = reinterpret_cast<class Sampler*>(sampler_src->data);
                    param->location = {glsl_texture->index, 0};
                    current_shader()->combined_samplers.push_back({glsl_texture->name, {glsl_texture->index, 0}});
                });
            }


            String result_source = Strings::format("texture({}, {})", glsl_texture->name, uv_src);
            return (new GLSL_CompiledSource(result_source))->id();
        }

#define declare_render_target_texture(func_name, type)                                                                           \
    size_t func_name##_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override                                         \
    {                                                                                                                            \
        bool created_now;                                                                                                        \
        GLSL_CompiledSource* sampler_src = sampler_source(sampler);                                                              \
        String uv_src                    = uv_source(uv);                                                                        \
                                                                                                                                 \
        GLSL_BindingObject* glsl_texture = create_binding_object(nullptr, sampler_src->data, "sampler2D", created_now);          \
                                                                                                                                 \
        if (created_now)                                                                                                         \
        {                                                                                                                        \
            on_success_command_list.push_back([this, sampler_src, glsl_texture]() {                                              \
                type##TextureMaterialParameter* param = reinterpret_cast<type##TextureMaterialParameter*>(                       \
                        material->create_parameter(glsl_texture->name, MaterialParameter::Type::type##Texture));                 \
                param->sampler  = reinterpret_cast<class Sampler*>(sampler_src->data);                                           \
                param->location = {glsl_texture->index, 0};                                                                      \
                current_shader()->combined_samplers.push_back({glsl_texture->name, {glsl_texture->index, 0}});                   \
            });                                                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
                                                                                                                                 \
        String result_source = Strings::format("texture({}, {})", glsl_texture->name, uv_src);                                   \
        return (new GLSL_CompiledSource(result_source))->id();                                                                   \
    }

        size_t base_color_texture(MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            bool created_now;
            GLSL_CompiledSource* sampler_src = sampler_source(sampler);
            String uv_src                    = uv_source(uv);

            GLSL_BindingObject* glsl_texture = create_binding_object(nullptr, sampler_src->data, "sampler2D", created_now);

            if (created_now)
            {
                on_success_command_list.push_back([this, sampler_src, glsl_texture]() {
                    BaseColorTextureMaterialParameter* param = reinterpret_cast<BaseColorTextureMaterialParameter*>(
                            material->create_parameter(glsl_texture->name, MaterialParameter::Type::BaseColorTexture));
                    param->sampler  = reinterpret_cast<class Sampler*>(sampler_src->data);
                    param->location = {glsl_texture->index, 0};
                    current_shader()->combined_samplers.push_back({glsl_texture->name, {glsl_texture->index, 0}});
                });
            }


            String result_source = Strings::format("texture({}, {})", glsl_texture->name, uv_src);
            return (new GLSL_CompiledSource(result_source))->id();
        }

        //declare_render_target_texture(base_color, BaseColor);
        declare_render_target_texture(position, Position);
        declare_render_target_texture(normal, Normal);
        declare_render_target_texture(emissive, Emissive);
        declare_render_target_texture(data_buffer, DataBuffer);
        declare_render_target_texture(scene_output, SceneOutput);

        virtual size_t sampler(class Engine::Sampler* sampler) override
        {
            return (new GLSL_CompiledSource("", sampler))->id();
        }

        size_t fragment_output_base_color(MaterialInputPin* pin) override
        {
            String source = get_pin_source(pin, MaterialNodeDataType::Vec3);
            statements.push_back(Strings::format("out_color.rgb = {}", source));
            return 0;
        }

        size_t fragment_output_metalic(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                String source = get_pin_source(pin, MaterialNodeDataType::Float);
                statements.push_back(Strings::format("out_data_buffer.r = {}", source));
            }
            return 0;
        }

        size_t fragment_output_specular(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                String source = get_pin_source(pin, MaterialNodeDataType::Float);
                statements.push_back(Strings::format("out_data_buffer.g = {}", source));
            }
            return 0;
        }

        size_t fragment_output_roughness(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                String source = get_pin_source(pin, MaterialNodeDataType::Float);
                statements.push_back(Strings::format("out_data_buffer.b = {}", source));
            }
            return 0;
        }

        size_t fragment_output_emissive(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                String source = get_pin_source(pin, MaterialNodeDataType::Vec3);
                statements.push_back(Strings::format("out_emissive = vec4({}, 1.0)", source));
            }
            return 0;
        }

        size_t fragment_output_opacity(MaterialInputPin* pin) override
        {
            String source = get_pin_source(pin, MaterialNodeDataType::Float);
            statements.push_back(Strings::format("out_color.a = {}", source));
            return 0;
        }

        size_t fragment_output_position(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                if (pin->linked_to)
                {
                    String source = get_pin_source(pin, MaterialNodeDataType::Vec3);
                    statements.push_back(Strings::format("out_position = vec4({}, 1.0)", source));
                }
                else
                {
                    statements.push_back(Strings::format("out_position = vec4(vertex_world_position, 1.0)"));
                }
            }
            return 0;
        }

        size_t fragment_output_normal(MaterialInputPin* pin) override
        {
            if (material->pipeline->render_pass == RenderPassType::GBuffer)
            {
                if (pin->linked_to)
                {
                    String source = get_pin_source(pin, MaterialNodeDataType::Vec3);
                    statements.push_back(Strings::format("out_normal = vec4({}, 1.0)", source));
                }
                else
                {
                    statements.push_back(Strings::format("out_normal = vec4(vertex_world_normal, 1.0)"));
                }
            }
            return 0;
        }

        void submit_vertex_attribute(const GLSL_Attribute& attribute) override
        {
            GLSL_Attribute new_attribute = attribute;
            new_attribute.location       = static_cast<byte>(vertex_compiler->input_attribute.size());
            vertex_compiler->input_attribute.push_back(new_attribute);
            vertex_compiler->submit_vertex_attribute(attribute);
        }
    };

#define exec_step(code)                                                                                                          \
    if (execute_next)                                                                                                            \
    {                                                                                                                            \
        execute_next = code;                                                                                                     \
    }
    class GLSL_Compiler : public ShaderCompilerBase
    {
        declare_class(GLSL_Compiler, ShaderCompilerBase);

    public:
        bool compile(class VisualMaterial* material, MessageList& _errors) override
        {
            errors = &_errors;

            GLSL_VertexCompiler* vertex_compiler     = new GLSL_VertexCompiler();
            GLSL_FragmentCompiler* fragment_compiler = new GLSL_FragmentCompiler(vertex_compiler);

            bool execute_next = true;
            exec_step(vertex_compiler->compile(material, _errors));

            fragment_compiler->local_parameters = std::move(vertex_compiler->local_parameters);
            fragment_compiler->sync(vertex_compiler);
            exec_step(fragment_compiler->compile(material, _errors));

            vertex_compiler->local_parameters = fragment_compiler->local_parameters;
            exec_step(vertex_compiler->build_source());
            exec_step(fragment_compiler->build_source());


            if (execute_next)
            {
                material->clear_parameters();
                material->pipeline->has_global_parameters = true;
                fragment_compiler->parse_reflection();
            }

            exec_step(vertex_compiler->submit_source());
            exec_step(fragment_compiler->submit_source());

            delete vertex_compiler;
            delete fragment_compiler;

            return true;
        }
    };

    implement_engine_class_default_init(GLSL_Compiler);
}// namespace Engine
