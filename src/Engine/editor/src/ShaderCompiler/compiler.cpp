#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <ShaderCompiler/compiler.hpp>
#include <cstring>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang-file-system.h>
#include <slang-list.h>
#include <slang.h>

#include <iostream>


namespace Engine::ShaderCompiler
{
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


    ShaderReflection& ShaderReflection::clear()
    {
        global_parameters_info.remove_parameters();
        local_parameters_info.remove_parameters();
        uniform_member_infos.clear();
        attributes.clear();
        return *this;
    }

    static bool find_semantic(String name, VertexBufferSemantic& out_semantic, const Function<void(const char*)>& print_error)
    {
        Strings::to_lower(name);

        static const TreeMap<String, VertexBufferSemantic> semantics = {
                {"position", VertexBufferSemantic::Position},       //
                {"texcoord", VertexBufferSemantic::TexCoord},       //
                {"color", VertexBufferSemantic::Color},             //
                {"normal", VertexBufferSemantic::Normal},           //
                {"tangent", VertexBufferSemantic::Tangent},         //
                {"binormal", VertexBufferSemantic::Binormal},       //
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
            print_error(Strings::format("Failed to find semantic '{}'", name).c_str());
            return false;
        }
    }

    static ColorFormat find_color_format_for_semantic(VertexBufferSemantic semantic)
    {
        switch (semantic)
        {
            case Engine::VertexBufferSemantic::Position:
                return ColorFormat::R32G32B32Sfloat;
            case Engine::VertexBufferSemantic::TexCoord:
                return ColorFormat::R32G32Sfloat;
            case Engine::VertexBufferSemantic::Color:
                return ColorFormat::R8G8B8A8Unorm;
            case Engine::VertexBufferSemantic::Normal:
                return ColorFormat::R32G32B32Sfloat;
            case Engine::VertexBufferSemantic::Tangent:
                return ColorFormat::R32G32B32Sfloat;
            case Engine::VertexBufferSemantic::Binormal:
                return ColorFormat::R32G32B32Sfloat;
            case Engine::VertexBufferSemantic::BlendWeight:
                return ColorFormat::R32G32B32Sfloat;
            case Engine::VertexBufferSemantic::BlendIndices:
                return ColorFormat::R8Sint;
        }
        return ColorFormat::Undefined;
    }

    static byte find_components_count_for_semantic(VertexBufferSemantic semantic)
    {
        switch (semantic)
        {
            case Engine::VertexBufferSemantic::Position:
                return 3;
            case Engine::VertexBufferSemantic::TexCoord:
                return 2;
            case Engine::VertexBufferSemantic::Color:
                return 4;
            case Engine::VertexBufferSemantic::Normal:
                return 3;
            case Engine::VertexBufferSemantic::Tangent:
                return 3;
            case Engine::VertexBufferSemantic::Binormal:
                return 3;
            case Engine::VertexBufferSemantic::BlendWeight:
                return 4;
            case Engine::VertexBufferSemantic::BlendIndices:
                return 1;
        }

        return 0;
    }

    static bool parse_vertex_semantic(slang::VariableLayoutReflection* var, ShaderReflection& out_reflection,
                                      const Function<void(const char*)>& print_error)
    {
        auto kind = var->getType()->getKind();

        if (kind == slang::TypeReflection::Kind::Struct)
        {
            auto layout       = var->getTypeLayout();
            auto fields_count = layout->getFieldCount();

            for (uint32_t field_index = 0; field_index < fields_count; ++field_index)
            {
                auto field = layout->getFieldByIndex(field_index);

                if (!parse_vertex_semantic(field, out_reflection, print_error))
                {
                    out_reflection.clear();
                    return false;
                }
            }
        }
        else if (kind == slang::TypeReflection::Kind::Vector)
        {
            ShaderReflection::VertexAttribute attribute;

            if (!find_semantic(var->getSemanticName(), attribute.semantic, print_error))
            {
                return false;
            }

            if (is_not_in<VertexBufferSemantic::Position,//
                          VertexBufferSemantic::TexCoord,//
                          VertexBufferSemantic::Color,   //
                          VertexBufferSemantic::Normal,  //
                          VertexBufferSemantic::Tangent, //
                          VertexBufferSemantic::Binormal,//
                          VertexBufferSemantic::BlendWeight>(attribute.semantic))
            {
                print_error(Strings::format("Semantic '{}' doesn't support vector type!", var->getSemanticName()).c_str());
                return false;
            }

            attribute.semantic_index = var->getSemanticIndex();
            attribute.name           = var->getName();
            attribute.rate           = VertexAttributeInputRate::Vertex;
            attribute.count          = 1;
            attribute.format         = find_color_format_for_semantic(attribute.semantic);

            auto requred_elements_count = find_components_count_for_semantic(attribute.semantic);

            if (requred_elements_count != var->getType()->getElementCount())
            {
                print_error(
                        Strings::format("Sematic '{}' require vector{}", var->getSemanticName(), requred_elements_count).c_str());
                return false;
            }

            out_reflection.attributes.push_back(attribute);
        }
        else
        {
            print_error("Unsupported input variable type!");
            return false;
        }

        return true;
    }

    static MaterialParameterType find_scalar_parameter_type(slang::TypeReflection* reflection)
    {
        auto rows     = reflection->getRowCount();
        auto colums   = reflection->getColumnCount();
        auto elements = reflection->getElementCount();
        auto scalar   = reflection->getScalarType();

        if (rows > 1 && colums > 1)
        {
            if (rows == colums && scalar == slang::TypeReflection::ScalarType::Float32)
            {
                if (rows == 4)
                {
                    return MaterialParameterType::Mat4;
                }
                else if (rows == 3)
                {
                    return MaterialParameterType::Mat3;
                }
            }
            return MaterialParameterType::Undefined;
        }
        else if (rows == colums && rows == 1)
        {
            switch (scalar)
            {
                case slang::TypeReflection::ScalarType::Bool:
                    return MaterialParameterType::Bool;
                case slang::TypeReflection::ScalarType::Int32:
                    return MaterialParameterType::Int;
                case slang::TypeReflection::ScalarType::UInt32:
                    return MaterialParameterType::UInt;
                case slang::TypeReflection::ScalarType::Float32:
                    return MaterialParameterType::Float;
                default:
                    return MaterialParameterType::Undefined;
            }
        }

        MaterialParameterType type = MaterialParameterType::Undefined;

        switch (scalar)
        {
            case slang::TypeReflection::ScalarType::Bool:
                type = MaterialParameterType::BVec2;
                break;
            case slang::TypeReflection::ScalarType::Int32:
                type = MaterialParameterType::IVec2;
                break;
            case slang::TypeReflection::ScalarType::UInt32:
                type = MaterialParameterType::UVec2;
                break;
            case slang::TypeReflection::ScalarType::Float32:
                type = MaterialParameterType::Vec2;
                break;
            default:
                return MaterialParameterType::Undefined;
        }

        return static_cast<MaterialParameterType>(static_cast<size_t>(type) + elements - 2);
    }

    static void create_reflection(slang::ShaderReflection* reflection, ShaderReflection& out_reflection,
                                  const Function<void(const char*)>& print_error)
    {
        out_reflection.clear();

        out_reflection.global_parameters_info.bind_index(0);
        if (reflection->getGlobalConstantBufferSize() > 0)
        {
            out_reflection.local_parameters_info.bind_index(reflection->getGlobalConstantBufferBinding());
        }

        // Parse vertex attributes
        if (auto entry_point = reflection->getEntryPointByIndex(0))
        {

            uint32_t parameter_count = entry_point->getParameterCount();
            for (uint32_t i = 0; i < parameter_count; i++)
            {
                if (!parse_vertex_semantic(entry_point->getParameterByIndex(i), out_reflection, print_error))
                {
                    out_reflection.clear();
                    return;
                }
            }
        }

        // Parse parameters
        int count = reflection->getParameterCount();

        for (int i = 0; i < count; i++)
        {
            auto param = reflection->getParameterByIndex(i);
            auto kind  = param->getTypeLayout()->getKind();

            if (is_in<slang::TypeReflection::Kind::Scalar, slang::TypeReflection::Kind::Vector,
                      slang::TypeReflection::Kind::Matrix>(kind))
            {
                auto name = param->getName();
                trinex_always_check(name, "Failed to get parameter name!");

                ShaderReflection::UniformMemberInfo info;
                info.type = find_scalar_parameter_type(param->getType());

                if (info.type == MaterialParameterType::Undefined)
                {
                    print_error("Failed to get parameter type!");
                    out_reflection.clear();
                    return;
                }

                info.name   = name;
                info.offset = param->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);

                if (auto layout = param->getTypeLayout())
                {
                    info.size = layout->getSize();
                }
                else
                {
                    print_error("Failed to get parameter layout info!");
                    out_reflection.clear();
                    return;
                }

                out_reflection.uniform_member_infos.push_back(info);
            }
        }
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

#define check_compile_errors()                                                                                                   \
    if (!errors->empty())                                                                                                        \
    return
    static void compile_shader(const String& source, const Vector<ShaderDefinition>& definitions, MessageList* errors,
                               const Function<void(SlangCompileRequest*)>& setup_request, ShaderReflection& out_reflection,
                               const Function<void(const byte*, size_t)>& on_vertex_success,
                               const Function<void(const byte*, size_t)>& on_fragment_success)
    {

        if (errors)
        {
            errors->clear();
        }

        auto print_error = [errors](const char* msg) {
            if (errors)
            {
                errors->push_back(Strings::format("{}", msg));
            }

            error_log("ShaderCompiler", "%s", msg);
        };

        auto diagnose_if_needed = [print_error](slang::IBlob* diagnostics_blob) {
            if (diagnostics_blob != nullptr)
            {
                print_error((const char*) diagnostics_blob->getBufferPointer());
            }
        };


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
                print_error("Failed to create compile request");
                return;
            }

            host_setup_request(request, definitions);
            if (setup_request)
            {
                setup_request(request);
            }
            auto unit = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "main_unit");
            spAddTranslationUnitSourceString(request, unit, "main_unit_source", source.c_str());

            auto compile_result = spCompile(request);

            if (auto diagnostics = spGetDiagnosticOutput(request))
            {
                if (strlen(diagnostics) > 0)
                {
                    print_error(diagnostics);
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
                print_error("Failed to find vertex_main. Skipping!");
            }
            else
            {
                component_types.add(vertex_entry_point);
                vertex_entry_index = 0;
            }
        }

        check_compile_errors();

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

        check_compile_errors();

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
                print_error("Failed to get shader reflection!");
                return;
            }

            create_reflection(reflection, out_reflection, print_error);
            check_compile_errors();
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
        request->setMatrixLayoutMode(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setTargetMatrixLayoutMode(0, SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
        request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);
        request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_FAST);
        auto profile = global_session()->findProfile("glsl_310_es");
        request->setTargetProfile(0, profile);
    }

    static void setup_spriv_compile_request(SlangCompileRequest* request)
    {
        request->setCodeGenTarget(SLANG_SPIRV);
        request->setMatrixLayoutMode(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setTargetMatrixLayoutMode(0, SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
        request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);
        request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_FAST);
        request->setTargetFlags(0, SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY);
        request->setTargetProfile(0, global_session()->findProfile("glsl_440"));
    }


    using CompileFunction = ShaderSource (*)(const String&, const Vector<ShaderDefinition>&, MessageList*);

    static ShaderSource compile_shader_source_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                        MessageList* errors, CompileFunction func)
    {
        auto path = engine_config.shaders_dir / relative;
        FileReader file(path);
        if (file.is_open())
        {
            return func(file.read_string(), definitions, errors);
        }
        else if (errors)
        {
            errors->push_back(Strings::format("Failed to open file '{}'", path.str()));
        }
        return {};
    }

    ShaderSource create_opengl_shader(const String& source, const Vector<ShaderDefinition>& definitions, MessageList* errors)
    {
        auto new_definitions = definitions;
        new_definitions.push_back({"TRINEX_OPENGL_API", ""});

        ShaderSource out_source;
        auto vertex_callback = [&](const byte* data, size_t size) {
            std::destroy_at(&out_source.vertex_code);
            new (&out_source.vertex_code) Buffer(data, data + size);
            out_source.vertex_code.push_back(0);
        };

        auto fragment_callback = [&](const byte* data, size_t size) {
            std::destroy_at(&out_source.fragment_code);
            new (&out_source.fragment_code) Buffer(data, data + size);
            out_source.fragment_code.push_back(0);
        };

        compile_shader(source, new_definitions, errors, setup_glsl_compile_request, out_source.reflection, vertex_callback,
                       fragment_callback);
        return out_source;
    }

    ShaderSource create_vulkan_shader(const String& source, const Vector<ShaderDefinition>& definitions, MessageList* errors)
    {
        auto new_definitions = definitions;
        new_definitions.push_back({"TRINEX_VULKAN_API", ""});

        ShaderSource out_source;
        auto vertex_callback = [&](const byte* data, size_t size) {
            std::destroy_at(&out_source.vertex_code);
            new (&out_source.vertex_code) Buffer(data, data + size);
        };

        auto fragment_callback = [&](const byte* data, size_t size) {
            std::destroy_at(&out_source.fragment_code);
            new (&out_source.fragment_code) Buffer(data, data + size);
        };

        compile_shader(source, new_definitions, errors, setup_spriv_compile_request, out_source.reflection, vertex_callback,
                       fragment_callback);
        return out_source;
    }

    ShaderSource create_opengl_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                MessageList* errors)
    {
        return compile_shader_source_from_file(relative, definitions, errors, create_opengl_shader);
    }

    ShaderSource create_vulkan_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                MessageList* errors)
    {
        return compile_shader_source_from_file(relative, definitions, errors, create_vulkan_shader);
    }


    implement_class(ShaderCompiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(ShaderCompiler);

    implement_class(OpenGL_ShaderCompiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(OpenGL_ShaderCompiler);

    implement_class(Vulkan_ShaderCompiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(Vulkan_ShaderCompiler);

    bool OpenGL_ShaderCompiler::compile(Material* material, ShaderSource& out_source, MessageList& errors)
    {
        auto source =
                create_opengl_shader_from_file(material->pipeline->shader_path.str(), material->compile_definitions, &errors);

        if (errors.empty())
        {
            out_source = source;
            return true;
        }
        return false;
    }

    bool Vulkan_ShaderCompiler::compile(Material* material, ShaderSource& out_source, MessageList& errors)
    {
        auto source =
                create_vulkan_shader_from_file(material->pipeline->shader_path.str(), material->compile_definitions, &errors);

        if (errors.empty())
        {
            out_source = source;
            return true;
        }
        return false;
    }
}// namespace Engine::ShaderCompiler
