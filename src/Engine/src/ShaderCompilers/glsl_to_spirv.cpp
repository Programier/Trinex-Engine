#include <Core/class.hpp>
#include <Core/shader_compiler.hpp>
#include <Core/shader_types.hpp>
#include <Core/string_functions.hpp>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace Engine
{
    using SpvBuffer = std::vector<uint32_t>;

    class GLSLToSPIRV : public ShaderCompiler
    {
    public:
        using Super = ShaderCompiler;

        GLSLToSPIRV()
        {
            glslang::InitializeProcess();
        }

        ~GLSLToSPIRV()
        {
            glslang::FinalizeProcess();
        }

        EShLanguage translate_stage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage::Vertex:
                    return EShLangVertex;

                case ShaderStage::Fragment:
                    return EShLangFragment;

                case ShaderStage::Compute:
                    return EShLangCompute;

                case ShaderStage::Geometry:
                    return EShLangGeometry;

                default:
                    throw EngineException("Undefined stage");
            }
        }


        GLSLToSPIRV& write_log(const String& msg, ErrorList* errors = nullptr)
        {
            if (errors)
            {
                errors->push_back(msg);
            }
            else
            {
                error_log("GLSLToSPIRV", msg.c_str());
            }

            return *this;
        }

        bool compile(const char* code, ShaderStage stage, Buffer& out_binary, bool debug, ErrorList* errors)
        {
            if (code == nullptr)
            {
                write_log("Shader code is empty!", errors);
                return false;
            }

            EShLanguage module_language = translate_stage(stage);
            glslang::TShader shader(module_language);
            const char* shaderStrings[1];
            shaderStrings[0] = code;
            shader.setStrings(shaderStrings, 1);

            int clientInputSemanticsVersion                     = 100;
            glslang::EShTargetClientVersion vulkanClientVersion = glslang::EShTargetVulkan_1_0;
            glslang::EShTargetLanguageVersion targetVersion     = glslang::EShTargetSpv_1_0;

            shader.setEnvInput(glslang::EShSourceGlsl, module_language, glslang::EShClientVulkan,
                               clientInputSemanticsVersion);
            shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
            shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

            auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);


            if (!shader.parse(GetDefaultResources(), 100, false, messages))
            {
                write_log(
                        Strings::format("GLSL parsing failed!\n{}\n{}", shader.getInfoLog(), shader.getInfoDebugLog()),
                        errors);
                return false;
            }

            glslang::TProgram program;

            program.addShader(&shader);

            if (!program.link(messages))
            {
                write_log(
                        Strings::format("GLSL linking failed!\n{}\n{}", shader.getInfoLog(), shader.getInfoDebugLog()),
                        errors);
                return false;
            }

            spv::SpvBuildLogger logger;
            glslang::SpvOptions spv_options;

            if (debug)
            {
                spv_options.generateDebugInfo = true;
                spv_options.disableOptimizer  = true;
                spv_options.optimizeSize      = false;
            }
            else
            {
                spv_options.generateDebugInfo = false;
                spv_options.disableOptimizer  = false;
                spv_options.optimizeSize      = true;
            }


            std::vector<unsigned int> spirv;
            glslang::GlslangToSpv(*program.getIntermediate(module_language), spirv, &logger, &spv_options);

            size_t binary_size = spirv.size() * sizeof(unsigned int);
            out_binary.resize(binary_size);
            std::memcpy(out_binary.data(), spirv.data(), binary_size);

            return true;
        }

        bool compile(const String& code, ShaderStage stage, Buffer& out_binary, bool debug, ErrorList* errors) override
        {
            if (code.empty())
            {
                write_log("Shader code is empty!", errors);
                return false;
            }

            return compile(code.data(), stage, out_binary, debug, errors);
        }

        bool compile(PipelineCreateInfo* info, bool debug, ErrorList* errors) override
        {
            if (info == nullptr)
                return false;

            int processed_count        = 0;
            bool status                = true;
            Buffer* stages_code[4]     = {&info->text.vertex, &info->text.fragment, &info->text.compute,
                                          &info->text.geometry};
            Buffer* out_stages_code[4] = {&info->binaries.vertex, &info->binaries.fragment, &info->binaries.compute,
                                          &info->binaries.geometry};
            ShaderStage stages[4]      = {ShaderStage::Vertex, ShaderStage::Fragment, ShaderStage::Compute,
                                          ShaderStage::Geometry};

            for (int i = 0; i < 4 && status; i++)
            {
                if (!stages_code[i]->empty())
                {
                    processed_count++;
                    status = compile(reinterpret_cast<const char*>(stages_code[i]->data()), stages[i],
                                     *(out_stages_code[i]), debug, errors);
                }
            }

            return status && processed_count > 0 && update_reflection(info);
        }


        GLSLToSPIRV& buffer_to_spirv_buf(const Buffer& buffer, SpvBuffer& out)
        {
            out.clear();
            out.resize(buffer.size() / sizeof(uint32_t));
            std::memcpy(out.data(), buffer.data(), buffer.size());
            return *this;
        }

        GLSLToSPIRV& update_vertex_reflection(PipelineCreateInfo* info)
        {
            if (info->binaries.vertex.empty())
                return *this;

            SpvBuffer buffer;
            buffer_to_spirv_buf(info->binaries.vertex, buffer);
            spirv_cross::CompilerGLSL compiler(buffer);

            auto active                            = compiler.get_active_interface_variables();
            spirv_cross::ShaderResources resources = compiler.get_shader_resources(active);


            auto size = resources.stage_inputs.size();
            info->vertex_info.attributes.resize(size);
            for (decltype(size) i = 0; i < size; i++)
            {
                auto& input                          = resources.stage_inputs[i];
                info->vertex_info.attributes[i].name = input.name;
            }

            return update_uniform_buffers(info, compiler, resources);
        }

        GLSLToSPIRV& update_fragment_reflection(PipelineCreateInfo* info)
        {
            if (info->binaries.fragment.empty())
                return *this;

            SpvBuffer buffer;
            buffer_to_spirv_buf(info->binaries.fragment, buffer);
            spirv_cross::CompilerGLSL compiler(buffer);

            auto active                            = compiler.get_active_interface_variables();
            spirv_cross::ShaderResources resources = compiler.get_shader_resources(active);

            update_uniform_buffers(info, compiler, resources).update_texture_samplers(info, compiler, resources);
            return *this;
        }

        GLSLToSPIRV& update_uniform_buffers(PipelineCreateInfo* info, const spirv_cross::CompilerGLSL& compiler,
                                            const spirv_cross::ShaderResources& resources)
        {
            for (auto& res : resources.uniform_buffers)
            {
                info->uniform_buffers.emplace_back();
                auto& output = info->uniform_buffers.back();

                const auto& type = compiler.get_type(res.type_id);
                output.binding   = compiler.get_decoration(res.id, spv::DecorationBinding);
                output.size      = compiler.get_declared_struct_size(type);
            }

            return *this;
        }

        GLSLToSPIRV& update_texture_samplers(PipelineCreateInfo* info, const spirv_cross::CompilerGLSL& compiler,
                                             const spirv_cross::ShaderResources& resources)
        {
            for (auto& res : resources.sampled_images)
            {
                info->texture_samplers.emplace_back();
                auto& output   = info->texture_samplers.back();
                output.binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            }

            return *this;
        }


        bool update_reflection(PipelineCreateInfo* info) override
        {
            info->uniform_buffers.clear();
            info->texture_samplers.clear();
            update_vertex_reflection(info).update_fragment_reflection(info);
            return true;
        }
    };

    register_class(Engine::GLSLToSPIRV);
}// namespace Engine
