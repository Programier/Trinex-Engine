#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Engine
{
    class Shader
    {
        bool _M_done = false;
        unsigned int _M_id = 0;
        unsigned int vertex = 0, fragment = 0, compute = 0;
        void delete_shaders();

    public:
        Shader();
        Shader(const std::string& vertex, const std::string& fragment);
        Shader(const std::string& compute);
        Shader& load(const std::string& vertex, const std::string& fragment);
        Shader& load(const std::string& compute);
        Shader(const Shader&) = delete;
        Shader& use();
        bool loaded();

        Shader& set(const std::string& value_name, float value);
        Shader& set(const std::string& value_name, int value);
        Shader& set(const std::string& value_name, const glm::mat4& value);
        Shader& set(const std::string& value_name, const bool& value);
        Shader& set(const std::string& value_name, const glm::vec2& value);
        Shader& set(const std::string& value_name, const glm::vec3& value);
        ~Shader();
    };

}// namespace Engine
