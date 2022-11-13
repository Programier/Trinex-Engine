#include <Core/engine_types.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opengl.hpp>
#include <opengl_shader.hpp>
#include <stdexcept>

#define delete_program(id)                                                                                                  \
    if ((id) != 0)                                                                                                          \
    glDeleteProgram(id)
#define delete_shader(id)                                                                                                   \
    if ((id) != 0)                                                                                                          \
    glDeleteShader(id)


void OpenGL_Shader::destroy()
{
    delete_program(_M_shader_id);
    for (auto id : _M_shaders_types_id)
    {
        delete_shader(id);
    }
    DEALLOC_INFO;
}

declare_cpp_destructor(OpenGL_Shader);


static bool compile_shader(const FileBuffer& code, GLuint& ID, int SHADER_TYPE, const ShaderParams& params,
                           const char* type = "")
{
    static GLchar log[1024];

    ID = glCreateShader(SHADER_TYPE);
    const GLchar* c_code = reinterpret_cast<const GLchar*>(code.data());

    if (params.source_type == ShaderSourceType::Text)
    {
        glShaderSource(ID, 1, &c_code, nullptr);
    }
    else if (params.source_type == ShaderSourceType::Binary)
    {
        glShaderBinary(1, &ID, (GLenum) 0, code.data(), code.size());
    }
    else
    {
        return false;
    }

    glCompileShader(ID);

    GLint succes;
    glGetShaderiv(ID, GL_COMPILE_STATUS, &succes);
    if (!succes)
    {
        glGetShaderInfoLog(ID, 1024, nullptr, log);
        external_logger->log("Failed to compile %s shader '%s'\n\n%s\n", type, params.name.c_str(), log);
        glDeleteShader(ID);
        ID = 0;
        return false;
    }

    return true;
}

static bool link_shader(OpenGL_Shader* shader, const ShaderParams& params)
{
    static GLchar log[1024];
    shader->_M_shader_id = glCreateProgram();
    for (auto& ell : shader->_M_shaders_types_id) glAttachShader(shader->_M_shader_id, ell);

    glLinkProgram(shader->_M_shader_id);
    GLint succes;
    glGetProgramiv(shader->_M_shader_id, GL_LINK_STATUS, &succes);
    if (!succes)
    {
        glGetProgramInfoLog(shader->_M_shader_id, 1024, nullptr, log);
        external_logger->log("Shader: Failed to link shader program '%s'\n\n%s\n", params.name.c_str(), log);
        return false;
    }

    return true;
}


const FileBuffer& get_code(const ShaderParams& params, int index)
{
    switch (index)
    {
        case 0:
            return params.vertex;
        case 1:
            return params.fragment;
        case 2:
            return params.compute;
        case 3:
            return params.geometry;
        default:
            throw std::runtime_error("Undefined index of shader");
    }
}


API void api_create_shader(ObjID& ID, const ShaderParams& params)
{
    api_destroy_object_instance(ID);

    OpenGL_Shader* shader = new OpenGL_Shader();
    ID = object_id_of(shader);


    static const GLint shader_types[4] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER, GL_GEOMETRY_SHADER};
    static const char* shader_type_names[4] = {"vertex", "fragment", "compute", "geometry"};

    for (int i = 0; i < 4; i++)
    {
        const FileBuffer& buffer = get_code(params, i);
        if (buffer.empty())
        {
            continue;
        }

        if (!compile_shader(buffer, shader->_M_shaders_types_id[i], shader_types[i], params, shader_type_names[i]))
        {
            api_destroy_object_instance(ID);
            return;
        }
    }

    if (!link_shader(shader, params))
    {
        api_destroy_object_instance(ID);
    }
}

API void api_use_shader(const ObjID& ID)
{
    make_shader(shader, ID);
    check(shader, );
    glUseProgram(shader->_M_shader_id);
}


API void api_set_shader_float_value(const ObjID& ID, const std::string& name, float value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniform1f(transformLoc, value);
}

API void api_set_shader_int_value(const ObjID& ID, const std::string& name, int value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniform1i(transformLoc, value);
}

API void api_set_shader_mat3_float_value(const ObjID& ID, const std::string& name, const glm::mat3& value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniformMatrix3fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

API void api_set_shader_mat4_float_value(const ObjID& ID, const std::string& name, const glm::mat4& value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}


API void api_set_shader_vec4_float_value(const ObjID& ID, const std::string& name, const glm::mat4& value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniform4fv(transformLoc, 1, glm::value_ptr(value));
}

API void api_set_shader_vec3_float_value(const ObjID& ID, const std::string& name, const glm::mat4& value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniform3fv(transformLoc, 1, glm::value_ptr(value));
}

API void api_set_shader_vec2_float_value(const ObjID& ID, const std::string& name, const glm::mat4& value)
{
    make_shader(shader, ID);
    check(shader, );

    GLuint transformLoc = glGetUniformLocation(shader->_M_shader_id, name.c_str());
    glUniform2fv(transformLoc, 1, glm::value_ptr(value));
}
