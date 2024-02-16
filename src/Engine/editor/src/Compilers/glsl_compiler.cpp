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


namespace Engine
{
    static thread_local MessageList* errors;

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
        const char* shaderStrings[1];
        shaderStrings[0] = text.c_str();
        shader.setStrings(shaderStrings, 1);

        // Set up shader options
        int clientInputSemanticsVersion                 = 100;// default for Vulkan
        glslang::EShTargetClientVersion clientVersion   = glslang::EShTargetVulkan_1_0;
        glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetSpv_1_0;

        // Set up shader environment
        shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, clientInputSemanticsVersion);
        shader.setEnvClient(glslang::EShClientVulkan, clientVersion);
        shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);


        // Preprocess the shader
        if (!shader.parse(GetDefaultResources(), clientInputSemanticsVersion, false, EShMsgDefault))
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
        glslang::SpvOptions spvOptions;
        spvOptions.generateDebugInfo = true;
        spvOptions.disableOptimizer  = false;
        spvOptions.optimizeSize      = true;
        spvOptions.stripDebugInfo    = false;
        spvOptions.validate          = true;
        std::vector<unsigned int> spirv;

        glslang::GlslangToSpv(*program.getIntermediate(lang), spirv, &spvOptions);


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


    struct GLSL_CompiledSource {
        String source;

        GLSL_CompiledSource(const String& source = {}) : source(source)
        {}

        size_t id() const
        {
            return reinterpret_cast<size_t>(this);
        }
    };

    struct GLSL_Input {
        String param;
        byte location;
        MaterialNodeDataType type;
        byte index;
    };

    struct GLSL_Output {
        String param;
        byte location;
        MaterialNodeDataType type;
    };

    struct GLSL_UniformParameter {
        MaterialNodeDataType type;
        String name;
    };


    struct GLSL_CompilerState {
        Map<Identifier, GLSL_CompiledSource*> compiled_pins;
        Vector<GLSL_Input> inputs;
        Vector<GLSL_Output> outputs;

        Vector<String> statements;

        void reset()
        {
            for (auto& [id, source] : compiled_pins)
            {
                delete source;
            }

            inputs.clear();
            outputs.clear();
            statements.clear();
            compiled_pins.clear();
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

        String compile()
        {
            String result = "#version 310 es\nprecision highp float;\n\n";

            for (auto& in : inputs)
            {
                result += Strings::format("layout(location = {}) in {} in_{}{};\n", in.location, glsl_type_of(in.type),
                                          in.param.c_str(), in.index);
            }

            for (auto& out : outputs)
            {
                result += Strings::format("layout(location = {}) out {} out_{};\n", out.location, glsl_type_of(out.type),
                                          out.param.c_str());
            }

            result += "\n\n";
            result += GlobalShaderParameters::shader_code();
            result.push_back(' ');
            result += global_ubo_name;
            result.push_back(';');

            result += "\n\nvoid main()\n{\n";

            for (auto& statement : statements)
            {
                result += "\t";
                result += statement;
                result += ";\n";
            }

            result += "}\n";
            return result;
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

            String code              = reinterpret_value(pin->default_value(), pin->value_type());
            source                   = new GLSL_CompiledSource(code);
            compiled_pins[pin->id()] = source;
            return source;
            ;
        }

        ~GLSL_CompilerState()
        {
            reset();
        }
    };

    enum class CompileStage
    {
        Vertex,
        Fragment,
    };

    class GLSL_Compiler : public ShaderCompiler
    {
        declare_class(GLSL_Compiler, ShaderCompiler);

    public:
        GLSL_CompilerState vertex_state;
        GLSL_CompilerState fragment_state;
        CompileStage stage = CompileStage::Vertex;

        GLSL_Compiler()
        {}

        ~GLSL_Compiler()
        {}

        GLSL_CompilerState& state()
        {
            if (stage == CompileStage::Vertex)
            {
                return vertex_state;
            }

            return fragment_state;
        }

        String cast_primitive_value(const String& in, MaterialNodeDataType in_type, MaterialNodeDataType out_type)
        {
            MaterialDataTypeInfo in_info  = MaterialDataTypeInfo::from(in_type);
            MaterialDataTypeInfo out_info = MaterialDataTypeInfo::from(out_type);

            if (in_info.components_count != 1 || out_info.components_count != 1)
                throw EngineException("Types in not primitive!");

            return Strings::format("{}({})", glsl_type_of(out_type), in);
        }

        String cast_value(const String& in, GLSL_CompiledSource* source, MaterialNodeDataType in_type,
                          MaterialNodeDataType out_type)
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
                    StringView code = in;

                    if (source)
                    {
                        state().create_variable(source, in_type);
                        code = source->source;
                    }
                    else
                    {
                        code = state().create_variable(in, in_type);
                    }

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
            auto source = state().pin_source(pin, this);

            if (pin->refereces_count() > 1 || source->source.length() > max_sentence_len || force_as_var)
            {
                // Compile this source to variable
                state().create_variable(source, pin->value_type());
            }

            return cast_value(source->source, source, pin->node->output_type(pin), out_type);
        }

        String get_pin_source(MaterialInputPin* pin, MaterialNodeDataType out_type = MaterialNodeDataType::Undefined,
                              bool force_as_var = false)
        {
            auto source = state().pin_source(pin, this);

            if (source->source.length() > max_sentence_len || force_as_var)
            {
                // Compile this source to variable
                state().create_variable(source, pin->value_type());
            }

            auto type = pin->value_type();
            return cast_value(source->source, source, type, out_type);
        }

        bool compile(class VisualMaterial* material, MessageList& _errors) override
        {
            errors = &_errors;
            vertex_state.reset();
            fragment_state.reset();
            material->pipeline->has_global_parameters = true;

            stage = CompileStage::Fragment;

            GLSL_Input input;
            input.index    = 0;
            input.location = 0;
            input.param    = "position";
            input.type     = MaterialNodeDataType::Vec2;
            vertex_state.inputs.push_back(input);
            vertex_state.statements.push_back(Strings::format("gl_Position = vec4(vec3(in_position0.xy, 0.0).xyz, 1.0)"));


            material->fragment_node()->compile(this, nullptr);


            material->pipeline->vertex_shader->attributes.clear();
            material->pipeline->vertex_shader->attributes.push_back(VertexShader::Attribute(ColorFormat::R32G32Sfloat));
            material->pipeline->vertex_shader->text_code   = vertex_state.compile();
            material->pipeline->fragment_shader->text_code = fragment_state.compile();


            if (!errors->empty())
                return false;

            if (!errors->empty())
                return false;

            compile_vertex_source(material->pipeline->vertex_shader->text_code, material->pipeline->vertex_shader->binary_code);
            compile_fragment_source(material->pipeline->fragment_shader->text_code,
                                    material->pipeline->fragment_shader->binary_code);
            if (!errors->empty())
                return false;

            //material->postload();
            return true;
        }

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

            MaterialNodeDataType result_type = operator_result_between(t1, t2);

            auto source = (new GLSL_CompiledSource(Strings::format("{}({} + {})", glsl_type_of(result_type),
                                                                   get_pin_source(pin1, result_type),
                                                                   get_pin_source(pin2, result_type))));
            return source->id();
        }

        size_t sub(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = operator_result_between(t1, t2);

            auto source = (new GLSL_CompiledSource(Strings::format("{}({} - {})", glsl_type_of(result_type),
                                                                   get_pin_source(pin1, result_type),
                                                                   get_pin_source(pin2, result_type))));
            return source->id();
        }

        size_t mul(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = operator_result_between(t1, t2);
            auto source                      = (new GLSL_CompiledSource(Strings::format("{}({} * {})", glsl_type_of(result_type),
                                                                                        get_pin_source(pin1, result_type),
                                                                                        get_pin_source(pin2, result_type))));
            return source->id();
        }

        size_t div(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = operator_result_between(t1, t2);
            auto source                      = (new GLSL_CompiledSource(Strings::format("{}({} / {})", glsl_type_of(result_type),
                                                                                        get_pin_source(pin1, result_type),
                                                                                        get_pin_source(pin2, result_type))));
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

            GLSL_CompiledSource* input_source = state().pin_source(pin, this);
            GLSL_CompiledSource* pin_source   = state().find_pin_source(pin->id(), true);

            if (pin_source != input_source)
            {
                pin_source->source = input_source->source;
            }

            pin_source->source = cast_value(pin_source->source, nullptr, input_type, vector_type);
            state().create_variable(pin_source, vector_type);

            static char components[] = "xyzw";
            String out_source        = Strings::format("{}.{}", pin_source->source, components[component_index]);

            return (new GLSL_CompiledSource(out_source))->id();
        }

        /// GLOBALS
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
            return (new GLSL_CompiledSource("gl_FragCoord.xy"))->id();
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


        /// FRAGMENT OUTPUT
        size_t base_color(MaterialInputPin* pin) override
        {
            GLSL_Output out;
            out.location = 0;
            out.type     = MaterialNodeDataType::Vec4;
            out.param    = "color";

            fragment_state.outputs.push_back(std::move(out));

            String source = get_pin_source(pin, MaterialNodeDataType::Vec4);
            state().statements.push_back(Strings::format("out_color = {}", source));
            return 0;
        }


        // TEXTURES

        virtual size_t texture_2d(class Engine::Texture2D* texture, MaterialInputPin* sampler, MaterialInputPin* uv) override
        {
            return 0;
        }
    };

    implement_engine_class_default_init(GLSL_Compiler);
}// namespace Engine
