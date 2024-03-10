#include <Core/engine_config.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <ShaderCompiler/compiler.hpp>
#include <cstring>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang-file-system.h>
#include <slang-list.h>
#include <slang.h>


namespace Engine::ShaderCompiler
{

    static const String trinex_shader_globals = R"(struct GlobalParameters
{
    float4x4 projview;
    float4  offset;
};
[vk::binding(0, 0)]
ConstantBuffer<GlobalParameters> globals;
)";

#define RETURN_ON_FAIL(code)                                                                                                     \
    if (SLANG_FAILED(code))                                                                                                      \
    return


    static slang::IGlobalSession* global_session()
    {
        static Slang::ComPtr<slang::IGlobalSession> slang_global_session;
        if (slang_global_session.get() == nullptr)
        {
            if (SLANG_FAILED(slang::createGlobalSession(slang_global_session.writeRef())))
            {
                throw EngineException("Cannot create global session");
            }
        }

        return slang_global_session.get();
    }

    static void diagnose_if_needed(slang::IBlob* diagnostics_blob)
    {
        if (diagnostics_blob != nullptr)
        {
            error_log("ShaderCompiler", "%s", (const char*) diagnostics_blob->getBufferPointer());
        }
    }

    static ShaderReflection create_reflection(slang::ShaderReflection* reflection)
    {
        ShaderReflection out_reflection;
        out_reflection.local_buffer_size    = reflection->getGlobalConstantBufferSize();
        out_reflection.local_buffer_binding = reflection->getGlobalConstantBufferBinding();

        int count = reflection->getParameterCount();

        for (int i = 0; i < count; i++)
        {
            auto param = reflection->getParameterByIndex(i);

            if (param->getCategory() == slang::ParameterCategory::Uniform)
            {
                auto name = param->getName();
                trinex_always_check(name, "Failed to get parameter name!");
                ShaderReflection::UniformMemberInfo info;
                info.name   = name;
                info.offset = param->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

                if (auto layout = param->getTypeLayout())
                {
                    info.size = layout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
                }
                else
                {
                    throw EngineException("Failed to get parameter layout info!");
                }
                out_reflection.uniform_member_infos.push_back(info);
            }
        }

        return out_reflection;
    }


    static void host_setup_request(SlangCompileRequest* request, const Vector<ShaderDefinition>& definitions)
    {
        Path shaders_dir = rootfs()->native_path(engine_config.shaders_dir);
        request->addSearchPath(shaders_dir.c_str());

        for (const auto& definition : definitions)
        {
            request->addPreprocessorDefine(definition.key.c_str(), definition.value.c_str());
        }
    }

    static void compile_shader(const String& source, const Vector<ShaderDefinition>& definitions,
                               const Function<void(SlangCompileRequest*)>& setup_request, ShaderReflection& out_reflection,
                               const Function<void(const byte*, size_t)>& on_vertex_success,
                               const Function<void(const byte*, size_t)>& on_fragment_success)
    {
        using Slang::ComPtr;
        slang::SessionDesc session_desc      = {};
        session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        session_desc.allowGLSLSyntax         = false;

        ComPtr<slang::ISession> session;
        RETURN_ON_FAIL(global_session()->createSession(session_desc, session.writeRef()));
        Slang::List<slang::IComponentType*> component_types = {};

        // Compile current module
        ComPtr<slang::IModule> slang_module;
        int_t vertex_entry_index   = -1;
        int_t fragment_entry_index = -1;

        {
            ComPtr<SlangCompileRequest> request;
            session->createCompileRequest(request.writeRef());

            if (!request)
            {
                error_log("ShaderCompiler", "Failed to create compile request");
                return;
            }

            host_setup_request(request, definitions);
            if (setup_request)
            {
                setup_request(request);
            }

            String full_source = trinex_shader_globals + source;
            auto unit          = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "main_unit");
            spAddTranslationUnitSourceString(request, unit, "main_unit_source", full_source.c_str());

            auto compile_result = spCompile(request);

            if (auto diagnostics = spGetDiagnosticOutput(request))
            {
                if (strlen(diagnostics) > 0)
                {
                    error_log("ShaderCompiler", "%s", diagnostics);
                }
            }

            if (SLANG_FAILED(compile_result))
            {
                return;
            }

            spCompileRequest_getModule(request, unit, slang_module.writeRef());
            component_types.add(slang_module);
        }

        ComPtr<slang::IEntryPoint> vertex_entry_point;
        {
            slang_module->findEntryPointByName("vertex_main", vertex_entry_point.writeRef());

            if (!vertex_entry_point)
            {
                error_log("ShaderCompiler", "Failed to find vertex_main. Skipping compiling vertex code");
            }
            else
            {
                component_types.add(vertex_entry_point);
                vertex_entry_index = 0;
            }
        }

        ComPtr<slang::IEntryPoint> fragment_entry_point;
        {
            slang_module->findEntryPointByName("fragment_main", fragment_entry_point.writeRef());

            if (!fragment_entry_point)
            {
                error_log("ShaderCompiler", "Failed to find fragment_main. Skipping compiling fragment code");
            }
            else
            {
                component_types.add(fragment_entry_point);
                fragment_entry_index = vertex_entry_index + 1;
            }
        }

        if (vertex_entry_point == nullptr && fragment_entry_point == nullptr)
        {
            return;
        }


        ComPtr<slang::IComponentType> program;
        {
            ComPtr<slang::IBlob> diagnostics_blob;
            SlangResult result = session->createCompositeComponentType(component_types.getBuffer(), component_types.getCount(),
                                                                       program.writeRef(), diagnostics_blob.writeRef());
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
                return;
            }

            out_reflection = create_reflection(reflection);
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

                on_vertex_success(reinterpret_cast<const byte*>(result_code->getBufferPointer()), result_code->getBufferSize());
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

                on_fragment_success(reinterpret_cast<const byte*>(result_code->getBufferPointer()), result_code->getBufferSize());
            }
        }
    }


    static void setup_glsl_compile_request(SlangCompileRequest* request)
    {
        request->setCodeGenTarget(SLANG_GLSL);
        request->setMatrixLayoutMode(SLANG_MATRIX_LAYOUT_ROW_MAJOR);
        request->setTargetMatrixLayoutMode(0, SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
        request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);
        request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_FAST);
    }

    GLSL_Source create_glsl_shader(const String& source, const Vector<ShaderDefinition>& definitions)
    {
        GLSL_Source out_source;
        auto vertex_callback = [&](const byte* data, size_t size) {
            out_source.vertex_code = reinterpret_cast<const char*>(data);
        };
        auto fragment_callback = [&](const byte* data, size_t size) {
            out_source.fragment_code = reinterpret_cast<const char*>(data);
        };

        compile_shader(source, definitions, setup_glsl_compile_request, out_source.reflection, vertex_callback,
                       fragment_callback);
        return out_source;
    }

    GLSL_Source create_glsl_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions)
    {
        FileReader file(engine_config.shaders_dir / relative);
        if (file.is_open())
        {
            return create_glsl_shader(file.read_string(), definitions);
        }

        return {};
    }
}// namespace Engine::ShaderCompiler
