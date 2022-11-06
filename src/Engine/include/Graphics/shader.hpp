#pragma once

#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <string>

namespace Engine
{
    CLASS Shader : public Object
    {
    public:
        implement_class_hpp(Shader);
        Shader(const ShaderParams& params);
        Shader(const std::string& name, const std::string& vertex, const std::string& fragment, const std::string& compute,
               const std::string& geometry, ShaderSourceType type = ShaderSourceType::Text);

        Shader& load(const ShaderParams& params);
        Shader& load(const std::string& name, const std::string& vertex, const std::string& fragment,
                     const std::string& compute, const std::string& geometry,
                     ShaderSourceType type = ShaderSourceType::Text);

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
