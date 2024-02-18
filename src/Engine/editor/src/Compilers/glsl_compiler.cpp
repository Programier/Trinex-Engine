#include <Compiler/compiler.hpp>
#include <Core/class.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Graphics/visual_material.hpp>

#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <chrono>

namespace Engine
{
    static thread_local MessageList* errors = nullptr;

    static const StringView variable_prefix        = "tnx_var_";
    static const StringView global_ubo_name        = "global";
    static const StringView local_ubo_name         = "local";
    static const StringView global_variable_prefix = "global.";
    static const StringView local_variable_prefix  = "local.";

    static constexpr inline size_t max_sentence_len = 80;

    static bool compile_shader(const String& text, Buffer& out, EShLanguage lang)
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
        return true;
    }

    static bool compile_fragment_source(const String& text, Buffer& out)
    {
        return compile_shader(text, out, EShLangFragment);
    }

    static bool compile_vertex_source(const String& text, Buffer& out)
    {
        return compile_shader(text, out, EShLangVertex);
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
    };

    struct GLSL_BindingObject {
        String name;
        void* object = nullptr;
        StringView type;
        BindingIndex index;
    };

    enum class CompileStage
    {
        Vertex,
        Fragment,
    };

    class GLSL_BaseCompiler : public ShaderCompiler
    {
    public:
        VisualMaterial* material = nullptr;
        Vector<GLSL_Attribute> input_attribute;
        Vector<GLSL_Attribute> output_attribute;

        Vector<GLSL_BindingObject*> objects;
        Map<Identifier, GLSL_CompiledSource*> compiled_pins;
        Vector<String> statements;
        byte next_binding_index = 1;

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
        GLSL_BindingObject* create_binding_object(void* object, StringView type, bool& created_now)
        {
            created_now = false;
            for (GLSL_BindingObject* glsl_object : objects)
            {
                if (glsl_object->object == object)
                {
                    return glsl_object;
                }
            }

            created_now = true;

            GLSL_BindingObject* new_object = new GLSL_BindingObject();
            new_object->object             = object;
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

        GLSL_CompiledSource* pin_source(MaterialOutputPin* pin, ShaderCompiler* compiler)
        {
            // Try to find it
            GLSL_CompiledSource* source = find_pin_source(pin->id(), false);
            if (source)
                return source;

            source                   = reinterpret_cast<GLSL_CompiledSource*>(pin->node->compile(compiler, pin));
            compiled_pins[pin->id()] = source;
            return source;
        }

        GLSL_CompiledSource* pin_source(MaterialInputPin* pin, ShaderCompiler* compiler)
        {
            if (pin->linked_to)
            {
                return pin_source(pin->linked_to, compiler);
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
            auto source = pin_source(pin, this);

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
            auto source = pin_source(pin, this);

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

        bool build_source()
        {
            String source = "#version 310 es\n"
                            "precision highp float;\n"
                            "\n";

            for (auto& attribute : input_attribute)
            {
                source += Strings::format("layout(location = {}) in {} in_{};\n", attribute.location,
                                          glsl_type_of(attribute.type), attribute.param);
            }

            source += "\n";

            for (auto& attribute : output_attribute)
            {
                source += Strings::format("layout(location = {}) out {} out_{};\n", attribute.location,
                                          glsl_type_of(attribute.type), attribute.param);
            }

            source += "\n";
            source += GlobalShaderParameters::shader_code();
            source.push_back(' ');
            source += global_ubo_name;
            source += ";\n\n";

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


            compile_shader(source, current_shader()->binary_code, shader_type());
            current_shader()->text_code = std::move(source);
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


        /// OPERATORS
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

            GLSL_CompiledSource* input_source = pin_source(pin, this);
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

        virtual size_t sampler(class Engine::Sampler* sampler) override
        {
            errors->push_back(Strings::format("Sampler node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t base_color(MaterialInputPin* pin) override
        {
            errors->push_back(Strings::format("BaseColor node doesn't supported in {} shader!", name()));
            return 0;
        }

        virtual size_t vertex(byte index) override
        {
            errors->push_back(Strings::format("Vertex node doesn't supported in {} shader!", name()));
            return 0;
        }

        size_t position(MaterialInputPin* pin) override
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

        Index create_vertex_attribute(const String& name, bool* created_now = nullptr)
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

        virtual size_t vertex(byte index) override
        {
            String code = Strings::format("position_{}", index);

            bool created_now;
            auto& attribute = input_attribute[create_vertex_attribute(code, &created_now)];

            if (created_now)
            {
                attribute.format = ColorFormat::R32G32B32Sfloat;
                attribute.type   = MaterialNodeDataType::Vec3;
            }

            return (new GLSL_CompiledSource(Strings::format("in_{}", code)))->id();
        }

        size_t position(MaterialInputPin* pin) override
        {
            String source = "";

            if (pin->linked_to)
            {
                source = get_pin_source(pin, MaterialNodeDataType::Vec4);
            }
            else
            {
                auto& attribute  = input_attribute[create_vertex_attribute("position_0")];
                attribute.format = ColorFormat::R32G32B32Sfloat;
                attribute.type   = MaterialNodeDataType::Vec3;
                source           = "global.projview * vec4(in_position_0.xyz, 1.0)";
            }

            statements.push_back(Strings::format("gl_Position = {}", source));
            return 0;
        }
    };

    class GLSL_FragmentCompiler : public GLSL_BaseCompiler
    {
    public:
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

            return GLSL_BaseCompiler::compile(material, errors);
        }


        size_t frag_coord() override
        {
            return (new GLSL_CompiledSource("gl_FragCoord.xy"))->id();
        }

        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            bool created_now;
            GLSL_BindingObject* glsl_texture = create_binding_object(texture, "sampler2D", created_now);

            if (created_now)
            {
                GLSL_CompiledSource* sampler_source       = pin_source(sampler, this);
                CombinedSampler2DMaterialParameter* param = reinterpret_cast<CombinedSampler2DMaterialParameter*>(
                        material->create_parameter(glsl_texture->name, MaterialParameter::Type::CombinedSampler2D));

                param->texture  = texture;
                param->sampler  = reinterpret_cast<class Sampler*>(sampler_source->data);
                param->location = {glsl_texture->index, 0};
                current_shader()->combined_samplers.push_back({glsl_texture->name, {glsl_texture->index, 0}});
            }

            String uv_source = get_pin_source(uv, MaterialNodeDataType::Vec2);

            String result_source = Strings::format("texture({}, {})", glsl_texture->name, uv_source);
            return (new GLSL_CompiledSource(result_source))->id();
        }

        virtual size_t sampler(class Engine::Sampler* sampler) override
        {
            return (new GLSL_CompiledSource("", sampler))->id();
        }

        size_t base_color(MaterialInputPin* pin) override
        {
            String source = get_pin_source(pin, MaterialNodeDataType::Vec4);
            statements.push_back(Strings::format("out_color = {}", source));
            return 0;
        }
    };

#define exec_step(code)                                                                                                          \
    if (execute_next)                                                                                                            \
    {                                                                                                                            \
        execute_next = code;                                                                                                     \
    }
    class GLSL_Compiler : public ShaderCompilerBase
    {
        declare_class(GLSL_Compiler, ShaderCompiler);

    public:
        bool compile(class VisualMaterial* material, MessageList& _errors) override
        {
            errors = &_errors;

            GLSL_VertexCompiler* vertex_compiler     = new GLSL_VertexCompiler();
            GLSL_FragmentCompiler* fragment_compiler = new GLSL_FragmentCompiler();

            bool execute_next = true;
            exec_step(vertex_compiler->compile(material, _errors));
            exec_step(vertex_compiler->build_source());
            exec_step(fragment_compiler->compile(material, _errors));
            exec_step(fragment_compiler->build_source());

            delete vertex_compiler;
            delete fragment_compiler;

            return true;
        }
    };

    implement_engine_class_default_init(GLSL_Compiler);
}// namespace Engine
