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
            errors->push_back(shader.getInfoLog());
            errors->push_back(shader.getInfoDebugLog());
            glslang::FinalizeProcess();
            return false;
        }

        // Create and link shader program
        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
        {
            errors->push_back(program.getInfoLog());
            errors->push_back(program.getInfoDebugLog());
            glslang::FinalizeProcess();
            return false;
        }

        // Translate to SPIR-V
        glslang::SpvOptions spvOptions;
        spvOptions.generateDebugInfo = true;
        spvOptions.disableOptimizer  = false;
        spvOptions.optimizeSize      = false;
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
                return Strings::format("int({})", *reinterpret_cast<int_t*>(data));
            case MaterialNodeDataType::UInt:
                return Strings::format("uint({})", *reinterpret_cast<uint_t*>(data));
            case MaterialNodeDataType::Float:
                return Strings::format("float({})", *reinterpret_cast<float*>(data));

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
                return nullptr;
        }
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

        GLSL_CompiledSource* pin_source(MaterialOutputPin* pin, ShaderCompiler* compiler)
        {
            // Try to find it
            auto it = compiled_pins.find(pin->id());
            if (it != compiled_pins.end())
                return it->second;

            GLSL_CompiledSource* source = reinterpret_cast<GLSL_CompiledSource*>(pin->node->compile(compiler, pin));
            compiled_pins[pin->id()]    = source;
            return source;
        }

        GLSL_CompiledSource* push_source(MaterialOutputPin* pin, const String& code)
        {
            GLSL_CompiledSource* source = new GLSL_CompiledSource();
            source->source              = code;
            compiled_pins[pin->id()]    = source;
            return source;
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

        GLSL_CompilerState& state()
        {
            if (stage == CompileStage::Vertex)
            {
                return vertex_state;
            }

            return fragment_state;
        }

        static String validate_type(String value, MaterialNodeDataType in_type, MaterialNodeDataType out_type)
        {
            if ((Flags<MaterialNodeDataType>(out_type) & Flags<MaterialNodeDataType>(in_type)) !=
                        Flags<MaterialNodeDataType>(out_type) &&
                out_type != MaterialNodeDataType::Undefined)
            {
                return Strings::format("{}({})", glsl_type_of(out_type), value);
            }
            return value;
        }

        String get_pin_source(MaterialOutputPin* pin, MaterialNodeDataType out_type = MaterialNodeDataType::Undefined)
        {
            return validate_type(state().pin_source(pin, this)->source, pin->node->output_type(pin), out_type);
        }

        String get_pin_source(MaterialInputPin* pin, MaterialNodeDataType out_type = MaterialNodeDataType::Undefined)
        {
            if (pin->linked_to)
            {
                return get_pin_source(pin->linked_to, out_type);
            }

            auto type = pin->value_type();
            return validate_type(reinterpret_value(pin->default_value(), type), type, out_type);
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


            compile_vertex_source(material->pipeline->vertex_shader->text_code, material->pipeline->vertex_shader->binary_code);
            compile_fragment_source(material->pipeline->fragment_shader->text_code,
                                    material->pipeline->fragment_shader->binary_code);
            material->postload();
            return false;
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


        size_t add(MaterialInputPin* pin1, MaterialInputPin* pin2) override
        {
            auto t1 = pin1->value_type();
            auto t2 = pin1->value_type();

            MaterialNodeDataType result_type = operator_result_between(t1, t2);

            auto source = (new GLSL_CompiledSource(
                    Strings::format("{}({} + {})", glsl_type_of(result_type), get_pin_source(pin1, result_type), get_pin_source(pin2, result_type))));
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


        /// COMMON
        size_t time() override
        {
            return (new GLSL_CompiledSource(Strings::format("global.time")))->id();
        }

        /// CONSTANTS
        size_t float_constant(float value) override
        {
            return (new GLSL_CompiledSource(reinterpret_value(&value, MaterialNodeDataType::Float)))->id();
        }


        /// FRAGMENT OUTPUT

        size_t base_color(MaterialInputPin* pin) override
        {
            GLSL_Output out;
            out.location = 0;
            out.type     = MaterialNodeDataType::Vec4;
            out.param    = "color";

            fragment_state.outputs.push_back(std::move(out));

            String source = get_pin_source(pin);
            auto pin_type = pin->value_type();

            if (pin_type != MaterialNodeDataType::Vec4)
            {
                source = Strings::format("vec4({}, 1.0)", validate_type(source, pin_type, MaterialNodeDataType::Vec3));
            }

            state().statements.push_back(Strings::format("out_color = {}", source));
            return 0;
        }
    };

    implement_engine_class_default_init(GLSL_Compiler);
}// namespace Engine
