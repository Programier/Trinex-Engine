#pragma once
#include <Core/engine_types.hpp>

namespace Engine::ShaderCompiler
{
    struct ShaderReflection {
        struct UniformMemberInfo {
            String name;
            size_t size;
            size_t offset;
        };

        Vector<UniformMemberInfo> uniform_member_infos;
        BindingIndex local_buffer_binding;
        size_t local_buffer_size;
    };

    struct GLSL_Source {
        String vertex_code;
        String fragment_code;
        Buffer vertex_spirv;
        Buffer fragment_spirv;

        ShaderReflection reflection;
    };

    GLSL_Source string_to_glsl(const String& slang);
    GLSL_Source file_to_glsl(const Path& slang);
}// namespace Engine::ShaderCompiler
