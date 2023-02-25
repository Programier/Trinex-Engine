#include <algorithm>
#include <opengl_api.hpp>
#include <opengl_types.hpp>
#include <unordered_set>

namespace Engine
{
    struct OpenGL_Mesh : OpenGL_Object {
        GLuint _M_VBO = 0;
        GLuint _M_EBO = 0;
        std::size_t _M_buffer_size;
        GLint _M_indexes_type;
        DrawMode _M_mode;

        OpenGL_Mesh()
        {
            opengl_debug_log("OpenGL: Create new mesh\n");
            glGenBuffers(1, &_M_VBO);
            glGenBuffers(1, &_M_EBO);
        }


        void* instance_address() override
        {
            return reinterpret_cast<void*>(this);
        }

        ~OpenGL_Mesh()
        {
            opengl_debug_log("OpenGL: Destroy mesh\n");

            glDeleteBuffers(1, &_M_VBO);
            glDeleteBuffers(1, &_M_EBO);
            _M_VBO = 0;
            _M_EBO = 0;
        }
    };


    OpenGL& OpenGL::generate_mesh(ObjID& ID)
    {
        if (ID)
            destroy_object(ID);
        ID = get_object_id(new OpenGL_Mesh());
        return *this;
    }

    OpenGL& OpenGL::mesh_data(const ObjID& ID, std::size_t buffer_len, DrawMode mode, void* data)
    {
        check(ID, *this);
        auto mesh = obj->get_instance_by_type<OpenGL_Mesh>();
        glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);

        mesh->_M_buffer_size = buffer_len;
        mesh->_M_mode = mode;
        glBufferData(GL_ARRAY_BUFFER, mesh->_M_buffer_size, data, _M_draw_modes.at(mode));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return *this;
    }

    OpenGL& OpenGL::draw_mesh(const ObjID& ID, Primitive primitive, std::size_t vertices, std::size_t offset)
    {
        if (!_M_current_shader)
            return *this;

        check(ID, *this);
        auto mesh = obj->get_instance_by_type<OpenGL_Mesh>();

        glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_M_EBO);

        apply_shader_vertex_attributes();
        glDrawElements(_M_primitives.at(primitive), vertices, mesh->_M_indexes_type, reinterpret_cast<void*>(offset));

        glBindVertexArray(0);

        return *this;
    }

    OpenGL& OpenGL::update_mesh_data(const ObjID& ID, std::size_t offset, std::size_t size, void* data)
    {
        check(ID, *this);
        auto mesh = obj->get_instance_by_type<OpenGL_Mesh>();

        if (mesh->_M_buffer_size < offset + size)
            return *this;

        glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return *this;
    }

    OpenGL& OpenGL::mesh_indexes_array(const ObjID& ID, std::size_t bytes, const IndexBufferComponent& data_type,
                                       void* data)
    {
        check(ID, *this);
        auto mesh = obj->get_instance_by_type<OpenGL_Mesh>();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_M_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, data, _M_draw_modes.at(mesh->_M_mode));
        mesh->_M_indexes_type = _M_index_buffer_components.at(data_type);
        return *this;
    }

}// namespace Engine
