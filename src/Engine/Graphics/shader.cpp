#include "shader.hpp"
#include <GL/glew.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace Engine
{
    Shader::Shader(const std::string& vertex_path, const std::string& fragment_path)
    {
        load(vertex_path, fragment_path);
    }

    Shader& Shader::use()
    {
        if (_M_done)
            glUseProgram(_M_id);
        return *this;
    }
    Shader::~Shader()
    {
        if (_M_done)
        {
            glDeleteProgram(_M_id);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }
    }
    bool Shader::loaded()
    {
        return _M_done;
    }

    Shader& Shader::set(const std::string& value_name, float value)
    {
        GLuint transformLoc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1f(transformLoc, value);
        return *this;
    }

    Shader& Shader::set(const std::string& value_name, int value)
    {
        GLuint transformLoc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1i(transformLoc, value);
        return *this;
    }

    Shader& Shader::set(const std::string& value_name, const glm::mat4& value)
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        return *this;
    }

    Shader& Shader::set(const std::string& value_name, const bool& value)
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1i(loc, static_cast<int>(value));
        return *this;
    }

    Shader& Shader::set(const std::string& value_name, const glm::vec2& value)
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform2fv(loc, 1, glm::value_ptr(value));
        return *this;
    }

    Shader& Shader::set(const std::string& value_name, const glm::vec3& value)
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform3fv(loc, 1, glm::value_ptr(value));
        return *this;
    }

    Shader::Shader() = default;

    Shader& Shader::load(const std::string& vertex_path, const std::string& fragment_path)
    {
        if (_M_done)
        {
            glDeleteProgram(_M_id);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }


        std::string vertex_code, fragment_code;
        std::ifstream vertex_file(vertex_path), fragment_file(fragment_path);
        try
        {
            std::stringstream buffer1, buffer2;
            buffer1 << vertex_file.rdbuf();
            buffer2 << fragment_file.rdbuf();

            vertex_code = buffer1.str();
            fragment_code = buffer2.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << e.what() << std::endl;
            _M_done = false;
            return *this;
        }

        // Compiling shaders
        GLint succes;
        GLchar log[1024];
        const GLchar* v_code = vertex_code.c_str();
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &v_code, nullptr);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &succes);
        if (!succes)
        {
            std::cerr << "Shader: Failed to compile vertex shader" << std::endl;
            glGetShaderInfoLog(vertex, 1024, nullptr, log);
            std::cerr << log << std::endl;
            _M_done = false;
            glDeleteShader(vertex);
            return *this;
        }

        const GLchar* f_code = fragment_code.c_str();

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &f_code, nullptr);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &succes);
        if (!succes)
        {
            std::cerr << "Shader: Failed to compile fragment shader" << std::endl;
            glGetShaderInfoLog(fragment, 1024, nullptr, log);
            std::cerr << log << std::endl;
            _M_done = false;
            glDeleteShader(fragment);
            return *this;
        }

        _M_id = glCreateProgram();
        glAttachShader(_M_id, fragment);
        glAttachShader(_M_id, vertex);
        glLinkProgram(_M_id);
        glGetProgramiv(_M_id, GL_LINK_STATUS, &succes);
        if (!succes)
        {
            glGetProgramInfoLog(_M_id, 1024, nullptr, log);
            std::cerr << "Shader: Failed to link shader program" << std::endl;
            std::cerr << log << std::endl;
            glDeleteProgram(_M_id);
        }

        std::clog << "Shader: Compilation complete" << std::endl;
        _M_done = true;
        return *this;
    }
}// namespace Engine
