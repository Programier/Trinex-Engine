#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <SPIRV/GlslangToSpv.h>
#include <ShaderCompiler/compiler.hpp>
#include <cstring>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
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
        return                                                                                                                   \
        {}


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
        auto kind     = var->getType()->getKind();
        auto category = var->getCategory();
        if (category != slang::ParameterCategory::VaryingInput)
            return true;

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

            const char* semantic_name = var->getSemanticName();
            if (semantic_name == nullptr)
            {
                print_error(Strings::format("Cannot find semantic for vertex input '{}'", var->getName()).c_str());
                return false;
            }
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
            out_reflection.local_parameters_info.bind_index(reflection->getGlobalParamsVarLayout()->getBindingIndex());
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
            else if (is_in<slang::TypeReflection::Kind::Resource>(kind))
            {
                if (auto type_layout = param->getTypeLayout())
                {
                    SlangResourceShape shape = type_layout->getResourceShape();
                    if (shape == SLANG_TEXTURE_2D)
                    {
                        auto binding_type = type_layout->getBindingRangeType(0);

                        ShaderCompiler::ShaderReflection::BindingObject object;

                        object.name             = param->getName();
                        object.location.binding = param->getBindingIndex();
                        object.location.set     = 0;
                        object.type             = binding_type == slang::BindingType::CombinedTextureSampler
                                                          ? MaterialParameterType::CombinedImageSampler2D
                                                          : MaterialParameterType::Texture2D;
                        out_reflection.binding_objects.push_back(object);
                    }
                }
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
        return                                                                                                                   \
        {}


    static void submit_compiled_source(Buffer& out_buffer, const void* _data, size_t size)
    {
        std::destroy_at(&out_buffer);
        const byte* data = reinterpret_cast<const byte*>(_data);
        new (&out_buffer) Buffer(data, data + size);
        out_buffer.push_back(0);
    }

    static ShaderSource compile_shader(const String& source, const Vector<ShaderDefinition>& definitions, MessageList* errors,
                                       const Function<void(SlangCompileRequest*)>& setup_request)
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

        ShaderSource out_source;
        using Slang::ComPtr;
        slang::SessionDesc session_desc      = {};
        session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        session_desc.allowGLSLSyntax         = false;

        ComPtr<slang::ISession> session;
        RETURN_ON_FAIL(global_session()->createSession(session_desc, session.writeRef()));
        Slang::List<slang::IComponentType*> component_types = {};

        // Compile current module
        ComPtr<slang::IModule> slang_module;

        int_t current_entry_index = 0;

        int_t vertex_entry_index         = -1;
        int_t tessellation_control_index = -1;
        int_t tessellation_index         = -1;
        int_t geometry_index             = -1;
        int_t fragment_entry_index       = -1;

        {
            ComPtr<SlangCompileRequest> request;
            session->createCompileRequest(request.writeRef());

            if (!request)
            {
                print_error("Failed to create compile request");
                return {};
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
                return {};
            }

            spCompileRequest_getModule(request, unit, slang_module.writeRef());
            component_types.add(slang_module);
        }

        ComPtr<slang::IEntryPoint> vertex_entry_point;
        {
            slang_module->findEntryPointByName("vs_main", vertex_entry_point.writeRef());

            if (!vertex_entry_point)
            {
                print_error("Failed to find vs_main. Skipping!");
            }
            else
            {
                component_types.add(vertex_entry_point);
                vertex_entry_index = current_entry_index++;
            }
        }

        check_compile_errors();

        ComPtr<slang::IEntryPoint> fragment_entry_point;
        {
            slang_module->findEntryPointByName("fs_main", fragment_entry_point.writeRef());

            if (!fragment_entry_point)
            {
                print_error("Failed to find fs_main. Skipping compiling fragment code");
            }
            else
            {
                component_types.add(fragment_entry_point);
                fragment_entry_index = current_entry_index++;
            }
        }

        check_compile_errors();


        ComPtr<slang::IEntryPoint> tessellation_control_entry_point;
        {
            slang_module->findEntryPointByName("tsc_main", tessellation_control_entry_point.writeRef());

            if (tessellation_control_entry_point)
            {
                component_types.add(tessellation_control_entry_point);
                tessellation_control_index = current_entry_index++;
            }
        }

        ComPtr<slang::IEntryPoint> tessellation_entry_point;
        {
            slang_module->findEntryPointByName("ts_main", tessellation_entry_point.writeRef());

            if (tessellation_entry_point)
            {
                component_types.add(tessellation_entry_point);
                tessellation_index = current_entry_index++;
            }
        }

        ComPtr<slang::IEntryPoint> geometry_entry_point;
        {
            slang_module->findEntryPointByName("gs_main", geometry_entry_point.writeRef());

            if (geometry_entry_point)
            {
                component_types.add(geometry_entry_point);
                geometry_index = current_entry_index++;
            }
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
                print_error("Failed to get shader reflection!");
                return {};
            }

            create_reflection(reflection, out_source.reflection, print_error);
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

                submit_compiled_source(out_source.vertex_code, result_code->getBufferPointer(), result_code->getBufferSize());
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

                submit_compiled_source(out_source.fragment_code, result_code->getBufferPointer(), result_code->getBufferSize());
            }
        }

        if (tessellation_control_entry_point)
        {
            ComPtr<slang::IBlob> result_code;
            {
                ComPtr<slang::IBlob> diagnostics_blob;
                SlangResult result = program->getEntryPointCode(tessellation_control_index, 0, result_code.writeRef(),
                                                                diagnostics_blob.writeRef());
                diagnose_if_needed(diagnostics_blob);
                RETURN_ON_FAIL(result);

                submit_compiled_source(out_source.tessellation_control_code, result_code->getBufferPointer(),
                                       result_code->getBufferSize());
            }
        }

        if (tessellation_entry_point)
        {
            ComPtr<slang::IBlob> result_code;
            {
                ComPtr<slang::IBlob> diagnostics_blob;
                SlangResult result =
                        program->getEntryPointCode(tessellation_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
                diagnose_if_needed(diagnostics_blob);
                RETURN_ON_FAIL(result);

                submit_compiled_source(out_source.tessellation_code, result_code->getBufferPointer(),
                                       result_code->getBufferSize());
            }
        }

        if (geometry_entry_point)
        {
            ComPtr<slang::IBlob> result_code;
            {
                ComPtr<slang::IBlob> diagnostics_blob;
                SlangResult result =
                        program->getEntryPointCode(geometry_index, 0, result_code.writeRef(), diagnostics_blob.writeRef());
                diagnose_if_needed(diagnostics_blob);
                RETURN_ON_FAIL(result);

                submit_compiled_source(out_source.geometry_code, result_code->getBufferPointer(), result_code->getBufferSize());
            }
        }

        return out_source;
    }

    static void setup_glsl_base_compile_request(SlangCompileRequest* request)
    {
        request->setCodeGenTarget(SLANG_GLSL);
        request->setMatrixLayoutMode(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setTargetMatrixLayoutMode(0, SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);
        request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_MAXIMAL);
        request->setTargetLineDirectiveMode(0, SLANG_LINE_DIRECTIVE_MODE_NONE);
        request->setTargetFloatingPointMode(0, SLANG_FLOATING_POINT_MODE_FAST);
    }

    static void setup_glsles_compile_request(SlangCompileRequest* request)
    {
        setup_glsl_base_compile_request(request);
        auto profile = global_session()->findProfile("glsl_310_es");
        request->setTargetProfile(0, profile);
    }

    static void setup_glsl_compile_request(SlangCompileRequest* request)
    {
        setup_glsl_base_compile_request(request);
        auto profile = global_session()->findProfile("glsl_440");
        request->setTargetProfile(0, profile);
    }

    static void setup_spriv_compile_request(SlangCompileRequest* request)
    {
        request->setCodeGenTarget(SLANG_GLSL);
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

    static ShaderSource create_opengles_shader(const String& source, const Vector<ShaderDefinition>& definitions,
                                               MessageList* errors)
    {
        auto new_definitions = definitions;
        new_definitions.push_back({"TRINEX_OPENGLES_API", "1"});
        return compile_shader(source, new_definitions, errors, setup_glsles_compile_request);
    }

    static ShaderSource create_opengl_shader(const String& source, const Vector<ShaderDefinition>& definitions,
                                             MessageList* errors)
    {
        auto new_definitions = definitions;
        new_definitions.push_back({"TRINEX_OPENGL_API", "1"});
        return compile_shader(source, new_definitions, errors, setup_glsl_compile_request);
    }

    static ShaderSource create_vulkan_shader(const String& source, const Vector<ShaderDefinition>& definitions,
                                             MessageList* errors)
    {
        auto new_definitions = definitions;
        new_definitions.push_back({"TRINEX_VULKAN_API", "1"});
        return compile_shader(source, new_definitions, errors, setup_spriv_compile_request);
    }

    static ShaderSource create_opengles_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                         MessageList* errors)
    {
        return compile_shader_source_from_file(relative, definitions, errors, create_opengles_shader);
    }

    static ShaderSource create_opengl_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                       MessageList* errors)
    {
        return compile_shader_source_from_file(relative, definitions, errors, create_opengl_shader);
    }

    static ShaderSource create_vulkan_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions,
                                                       MessageList* errors)
    {
        return compile_shader_source_from_file(relative, definitions, errors, create_vulkan_shader);
    }

    implement_class(OpenGLES_Compiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(OpenGLES_Compiler);

    implement_class(OpenGL_Compiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(OpenGL_Compiler);

    implement_class(Vulkan_Compiler, Engine::ShaderCompiler, 0);
    implement_default_initialize_class(Vulkan_Compiler);

    bool OpenGLES_Compiler::compile(Material* material, ShaderSource& out_source, MessageList& errors)
    {
        auto source =
                create_opengles_shader_from_file(material->pipeline->shader_path.str(), material->compile_definitions, &errors);

        if (errors.empty())
        {
            out_source = std::move(source);
            return true;
        }
        return false;
    }

    bool OpenGL_Compiler::compile(Material* material, ShaderSource& out_source, MessageList& errors)
    {
        auto source =
                create_opengl_shader_from_file(material->pipeline->shader_path.str(), material->compile_definitions, &errors);

        if (errors.empty())
        {
            out_source = std::move(source);
            return true;
        }
        return false;
    }

    static bool compile_shader_to_spirv(const char* compiler_type, const Buffer& text, Buffer& out, EShLanguage lang,
                                        MessageList& errors)
    {
        if (text.empty())
        {
            out = {};
            return false;
        }

        glslang::InitializeProcess();

        glslang::TShader shader(lang);
        const char* shader_strings[1];
        shader_strings[0] = reinterpret_cast<const char*>(text.data());

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
            errors.push_back(Strings::format("{}: {}", compiler_type, shader.getInfoLog()));
            errors.push_back(Strings::format("{}: {}", compiler_type, shader.getInfoDebugLog()));
            glslang::FinalizeProcess();
            return false;
        }

        // Create and link shader program
        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
        {
            errors.push_back(Strings::format("{}: {}", compiler_type, shader.getInfoLog()));
            errors.push_back(Strings::format("{}: {}", compiler_type, shader.getInfoDebugLog()));
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

    bool Vulkan_Compiler::compile(Material* material, ShaderSource& out_source, MessageList& errors)
    {
        auto source =
                create_vulkan_shader_from_file(material->pipeline->shader_path.str(), material->compile_definitions, &errors);

        if (!errors.empty())
            return false;

        ShaderSource tmp;
        if (!compile_shader_to_spirv("Vertex Compiler", source.vertex_code, tmp.vertex_code, EShLangVertex, errors))
        {
            return false;
        }

        compile_shader_to_spirv("Tessellation Control Compiler", source.tessellation_control_code, tmp.tessellation_control_code,
                                EShLangTessControl, errors);

        compile_shader_to_spirv("Tessellation Compiler", source.tessellation_code, tmp.tessellation_code, EShLangTessEvaluation,
                                errors);

        compile_shader_to_spirv("Geometry Compiler", source.geometry_code, tmp.geometry_code, EShLangGeometry, errors);

        if (!compile_shader_to_spirv("Fragment Compiler", source.fragment_code, tmp.fragment_code, EShLangFragment, errors))
        {
            return false;
        }

        out_source                           = std::move(source);
        out_source.vertex_code               = std::move(tmp.vertex_code);
        out_source.tessellation_control_code = std::move(tmp.tessellation_control_code);
        out_source.tessellation_code         = std::move(tmp.tessellation_code);
        out_source.geometry_code             = std::move(tmp.geometry_code);
        out_source.fragment_code             = std::move(tmp.fragment_code);

        return errors.empty();
    }
}// namespace Engine::ShaderCompiler
