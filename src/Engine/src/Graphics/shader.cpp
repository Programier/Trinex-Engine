#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <opengl.hpp>
#include <sstream>
#include <vector>

#define delete_program(id)                                                                                                  \
    if ((id) != 0)                                                                                                          \
    glDeleteProgram(id)
#define delete_shader(id)                                                                                                   \
    if ((id) != 0)                                                                                                          \
    glDeleteShader(id)


static void read_file(const std::string& filename, std::string& out)
{
    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
        Engine::logger->log("Shader: Failed to open %s\n", filename.c_str());
        throw -1;
    }

    try
    {
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        out = buffer.str();
    }
    catch (...)
    {
        Engine::logger->log("Shader: Failed to read %s\n", filename.c_str());
    }
}

static void compile_shader(const GLchar* code, unsigned int& ID, int SHADER_TYPE, const char* path)
{
    static GLchar log[1024];
    ID = glCreateShader(SHADER_TYPE);
    glShaderSource(ID, 1, &code, nullptr);
    glCompileShader(ID);
    GLint succes;
    glGetShaderiv(ID, GL_COMPILE_STATUS, &succes);
    if (!succes)
    {
        Engine::logger->log("Shader: Failed to compile shader %s\n", path);
        glGetShaderInfoLog(ID, 1024, nullptr, log);
        Engine::logger->log("%s\n", log);
        glDeleteShader(ID);
        ID = 0;
        throw -1;
    }
}

static void link_shader(unsigned int& ID, const std::vector<unsigned int>& shaders)
{
    static GLchar log[1024];
    ID = glCreateProgram();
    for (auto& ell : shaders) glAttachShader(ID, ell);

    glLinkProgram(ID);
    GLint succes;
    glGetProgramiv(ID, GL_LINK_STATUS, &succes);
    if (!succes)
    {
        glGetProgramInfoLog(ID, 1024, nullptr, log);
        Engine::logger->log("Shader: Failed to link shader program\n%s\n", log);
        glDeleteProgram(ID);
        ID = 0;
        throw -1;
    }
}

namespace Engine
{
    void Shader::delete_shaders()
    {
        delete_program(_M_id);
        delete_shader(vertex);
        delete_shader(fragment);
    }

    Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, bool is_files)
    {
        load(vertex_path, fragment_path, is_files);
    }


    Shader::Shader(Shader&& sh)
    {
        *this = std::move(sh);
    }

    Shader& Shader::operator=(Shader&& sh)
    {
        if (this == &sh)
            return *this;
        delete_shaders();
        _M_id = sh._M_id;
        _M_done = sh._M_done;
        sh._M_id = 0;
        sh._M_done = false;
        return *this;
    }

    const Shader& Shader::use() const
    {
        if (_M_done)
            glUseProgram(_M_id);
        return *this;
    }

    Shader::~Shader()
    {
        delete_shaders();
    }

    bool Shader::loaded() const
    {
        return _M_done;
    }

    const Shader& Shader::set(const std::string& value_name, const ShaderMode& mode) const
    {
        return set(value_name, static_cast<int>(mode));
    }

    const Shader& Shader::set(const std::string& value_name, float value) const
    {
        GLuint transformLoc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1f(transformLoc, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, int value) const
    {
        GLuint transformLoc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1i(transformLoc, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat4& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat3& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const bool& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform1i(loc, static_cast<int>(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec2& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform2fv(loc, 1, glm::value_ptr(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec3& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform3fv(loc, 1, glm::value_ptr(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const Vector4D& value) const
    {
        GLuint loc = glGetUniformLocation(_M_id, value_name.c_str());
        glUniform4fv(loc, 1, glm::value_ptr(value));
        return *this;
    }

    Shader::Shader() = default;

    Shader& Shader::load(const std::string& _vertex, const std::string& _fragment, bool is_files)
    {
        delete_shaders();
        if (is_files)
        {
            logger->log("Loading shaders: %s, %s\n", _vertex.c_str(), _fragment.c_str());
        }

        std::string vertex_code, fragment_code;
        const std::string *out_code_vs = &_vertex, *out_code_fs = &_fragment;
        try
        {
            if (is_files)
            {
                read_file(_vertex, vertex_code);
                read_file(_fragment, fragment_code);
                out_code_vs = &vertex_code;
                out_code_fs = &fragment_code;
            }
            compile_shader(out_code_vs->c_str(), vertex, GL_VERTEX_SHADER, (is_files ? _vertex.c_str() : ""));
            compile_shader(out_code_fs->c_str(), fragment, GL_FRAGMENT_SHADER, (is_files ? _fragment.c_str() : ""));
            link_shader(_M_id, {vertex, fragment});
        }
        catch (...)
        {
            _M_done = false;
            return *this;
        }

        logger->log("Shader: Compilation complete\n");
        _M_done = true;
        return *this;
    }

    Shader& Shader::load(const std::string& compute_path)
    {
        delete_shaders();
        std::string code;
        try
        {
            read_file(compute_path, code);
            compile_shader(code.c_str(), compute, GL_COMPUTE_SHADER, compute_path.c_str());
            link_shader(_M_id, {compute});
        }
        catch (...)
        {
            _M_done = false;
            return *this;
        }

        logger->log("Shader: Compilation complete\n");
        _M_done = true;
        return *this;
    }

    Shader::Shader(const std::string& compute)
    {
        load(compute);
    }
}// namespace Engine
