#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <string>

namespace Engine
{
    CLASS Shader : public ApiObject
    {
        declare_instance_info_hpp(Shader);

    public:
        delete_copy_constructors(Shader);
        constructor_hpp(Shader);
        Shader(const ShaderParams& params);

        Shader& load(const ShaderParams& params);

        const Shader& use() const;
        const Shader& set(const std::string& value_name, float value) const;
        const Shader& set(const std::string& value_name, int value) const;
        const Shader& set(const std::string& value_name, const glm::mat4& value) const;
        const Shader& set(const std::string& value_name, const glm::mat3& value) const;
        const Shader& set(const std::string& value_name, const bool& value) const;
        const Shader& set(const std::string& value_name, const Vector2D& value) const;
        const Shader& set(const std::string& value_name, const Vector3D& value) const;
        const Shader& set(const std::string& value_name, const Vector4D& value) const;
    };

}// namespace Engine
