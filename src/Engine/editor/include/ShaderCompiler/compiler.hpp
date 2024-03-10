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

    struct ShaderDefinition {
        String key;
        String value;
    };

    struct GLSL_Source {
        String vertex_code;
        String fragment_code;
        Buffer vertex_spirv;
        Buffer fragment_spirv;

        ShaderReflection reflection;
    };

    GLSL_Source create_glsl_shader(const String& source, const Vector<ShaderDefinition>& definitions = {});
    GLSL_Source create_glsl_shader_from_file(const StringView& relative, const Vector<ShaderDefinition>& definitions = {});
}// namespace Engine::ShaderCompiler
