#pragma once

#include <Graphics/shader_mode.hpp>
#include <Core/engine_types.hpp>
#include <string>
#include <Core/export.hpp>

namespace Engine
{
    CLASS Shader
    {
        bool _M_done = false;
        ObjectID _M_id = 0;
        ObjectID vertex = 0, fragment = 0, compute = 0;
        void delete_shaders();

    public:
        Shader();
        Shader(const std::string& vertex_file, const std::string& fragment_file, bool is_files = true);
        Shader(const std::string& compute);
        Shader(Shader&&);
        Shader& operator =(Shader&&);
        Shader& load(const std::string& vertex, const std::string& fragment, bool is_files = true);
        Shader& load(const std::string& compute);
        Shader(const Shader&) = delete;
        const Shader& use() const;
        bool loaded() const;

        const Shader& set(const std::string& value_name, float value) const;
        const Shader& set(const std::string& value_name, int value) const;
        const Shader& set(const std::string& value_name, const glm::mat4& value) const;
        const Shader& set(const std::string& value_name, const glm::mat3& value) const;
        const Shader& set(const std::string& value_name, const bool& value) const;
        const Shader& set(const std::string& value_name, const Vector2D& value) const;
        const Shader& set(const std::string& value_name, const Vector3D& value) const;
        const Shader& set(const std::string& value_name, const Vector4D& value) const;
        const Shader& set(const std::string& value_name, const ShaderMode& mode) const;
        ~Shader();
    };

}// namespace Engine
