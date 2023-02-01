#include <glm/gtc/type_ptr.hpp>
#include <opengl_api.hpp>
#include <opengl_headers.hpp>
#include <stdexcept>

#define delete_program(id)                                                                                             \
    if ((id) != 0)                                                                                                     \
    glDeleteProgram(id)
#define delete_shader(id)                                                                                              \
    if ((id) != 0)                                                                                                     \
    glDeleteShader(id)

#define VERTEXT_ID 0
#define FRAGMENT_ID 1
#define COMPUTE_ID 2
#define GEOMENTRY_ID 3

namespace Engine
{

    struct OpenGL_Shader : public OpenGL_Object {
    public:
        bool _M_inided = false;
        GLuint _M_shaders_types_id[4] = {0, 0, 0, 0};

        OpenGL_Shader()
        {}

        ~OpenGL_Shader()
        {
            delete_program(_M_instance_id);
            for (auto id : _M_shaders_types_id)
            {
                delete_shader(id);
            }
        }
    };

    const std::vector<FileBuffer>& get_code(const ShaderParams& params, int index)
    {
        switch (index)
        {
            case 0:
                return params.text.vertex;
            case 1:
                return params.text.fragment;
            case 2:
                return params.text.compute;
            case 3:
                return params.text.geometry;
            default:
                throw std::runtime_error("Undefined index of shader");
        }
    }

    static bool compile_shader(const std::vector<FileBuffer>& shader_code, GLuint& ID, int SHADER_TYPE,
                               const ShaderParams& params, const char* type = "")
    {
        static GLchar log[1024];

        ID = glCreateShader(SHADER_TYPE);

        std::vector<const GLchar*> code;
        code.reserve(shader_code.size());

        for (auto& buffer : shader_code)
        {
            if (!buffer.empty())
                code.push_back(reinterpret_cast<const GLchar*>(buffer.data()));
        }

        glShaderSource(ID, code.size(), code.data(), nullptr);


        glCompileShader(ID);

        GLint succes;
        glGetShaderiv(ID, GL_COMPILE_STATUS, &succes);
        if (!succes)
        {
            glGetShaderInfoLog(ID, 1024, nullptr, log);
            OpenGL::_M_api->_M_current_logger->log("Failed to compile %s shader '%s'\n\n%s\n", type,
                                                   params.name.c_str(), log);
            glDeleteShader(ID);
            ID = 0;
            return false;
        }

        return true;
    }

    static bool link_shader(OpenGL_Shader* shader, const ShaderParams& params)
    {
        static GLchar log[1024];
        shader->_M_instance_id = glCreateProgram();
        for (auto& ell : shader->_M_shaders_types_id) glAttachShader(shader->_M_instance_id, ell);

        glLinkProgram(shader->_M_instance_id);
        GLint succes;
        glGetProgramiv(shader->_M_instance_id, GL_LINK_STATUS, &succes);
        if (!succes)
        {
            glGetProgramInfoLog(shader->_M_instance_id, 1024, nullptr, log);
            OpenGL::_M_api->_M_current_logger->log("Shader: Failed to link shader program '%s'\n\n%s\n",
                                                   params.name.c_str(), log);
            return false;
        }

        return true;
    }


    OpenGL& OpenGL::create_shader(ObjID& ID, const ShaderParams& params)
    {
        if (ID)
            destroy_object(ID);
        auto shader = new OpenGL_Shader();
        ID = get_object_id(shader);

        static const GLint shader_types[4] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER,
                                              GL_GEOMETRY_SHADER};
        static const char* shader_type_names[4] = {"vertex", "fragment", "compute", "geometry"};

        for (int i = 0; i < 4; i++)
        {
            const std::vector<FileBuffer>& buffer = get_code(params, i);
            if (buffer.empty())
            {
                continue;
            }

            if (!compile_shader(buffer, shader->_M_shaders_types_id[i], shader_types[i], params, shader_type_names[i]))
            {
                destroy_object(ID);
                return *this;
            }
        }

        if (!link_shader(shader, params))
        {
            destroy_object(ID);
        }
        return *this;
    }

    OpenGL& OpenGL::use_shader(const ObjID& ID)
    {
        check(ID, *this);
        auto shader = obj->get_instance_by_type<OpenGL_Shader>();
        glUseProgram(shader->_M_instance_id);
        return *this;
    }


#define value_declare(type, suffix, func, ...)                                                                         \
    OpenGL& OpenGL::shader_value(const ObjID& ID, const std::string& name, type value)                                 \
    {                                                                                                                  \
        check(ID, *this);                                                                                              \
        auto shader = obj->get_instance_by_type<OpenGL_Shader>();                                                      \
        GLuint transformLoc = glGetUniformLocation(shader->_M_instance_id, name.c_str());                              \
        glUniform##suffix(transformLoc, __VA_ARGS__ __VA_OPT__(, ) func(value));                                       \
        return *this;                                                                                                  \
    }

    value_declare(int_t, 1i, );
    value_declare(float, 1f, );
    value_declare(const glm::mat3&, Matrix3fv, glm::value_ptr, 1, GL_FALSE);
    value_declare(const glm::mat4&, Matrix4fv, glm::value_ptr, 1, GL_FALSE);
    value_declare(const glm::vec2&, 2fv, glm::value_ptr, 1);
    value_declare(const glm::vec3&, 3fv, glm::value_ptr, 1);
    value_declare(const glm::vec4&, 4fv, glm::value_ptr, 1);

}// namespace Engine
