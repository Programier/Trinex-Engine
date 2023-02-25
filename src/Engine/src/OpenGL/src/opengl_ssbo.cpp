#include <opengl_api.hpp>
#include <opengl_types.hpp>


#define internal_bind_ssbo(ID) glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID)

namespace Engine
{

    struct OpenGL_SSBO : OpenGL_Object {
        OpenGL_SSBO()
        {
            opengl_debug_log("OpenGL: Create new SSBO\n");
            glGenBuffers(1, &_M_instance_id);
        }

        ~OpenGL_SSBO()
        {
            opengl_debug_log("OpenGL: Destroy SSBO\n");
            if (_M_instance_id)
                glDeleteBuffers(1, &_M_instance_id);
        }
    };

    OpenGL& OpenGL::create_ssbo(ObjID& ID)
    {
        if (ID)
            destroy_object(ID);
        ID = get_object_id(new OpenGL_SSBO());
        return *this;
    }

    OpenGL& OpenGL::bind_ssbo(const ObjID& ID, int_t slot)
    {
        check(ID, *this);
        auto ssbo = obj->get_instance_by_type<OpenGL_SSBO>();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, ssbo->_M_instance_id);
        return *this;
    }

    OpenGL& OpenGL::ssbo_data(const ObjID& ID, void* data, std::size_t bytes, BufferUsage usage)
    {
        check(ID, *this);
        auto ssbo = obj->get_instance_by_type<OpenGL_SSBO>();
        internal_bind_ssbo(ssbo->_M_instance_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, data, _M_draw_modes.at(usage));
        internal_bind_ssbo(0);
        return *this;
    }

    OpenGL& OpenGL::update_ssbo_data(const ObjID& ID, void* data, std::size_t bytes, std::size_t offset)
    {
        check(ID, *this);
        auto ssbo = obj->get_instance_by_type<OpenGL_SSBO>();
        internal_bind_ssbo(ssbo->_M_instance_id);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, bytes, data);
        internal_bind_ssbo(0);
        return *this;
    }
}// namespace Engine
