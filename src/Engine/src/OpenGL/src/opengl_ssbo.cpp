#include <opengl_ssbo.hpp>
#include <opengl_types.hpp>

void OpenGL_SSBO::destroy()
{
    if(_M_ID)
        glDeleteBuffers(1, &_M_ID);
}

declare_cpp_destructor(OpenGL_SSBO);


API void api_create_ssbo(ObjID& ID)
{
    if(ID)
        api_destroy_object_instance(ID);

    OpenGL_SSBO* ssbo = new OpenGL_SSBO();
    ID = reinterpret_cast<ObjID>(ssbo);
    glGenBuffers(1, &ssbo->_M_ID);
}

#define internal_bind_ssbo(ID) glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ID)

API void api_bind_ssbo(const ObjID& ID, unsigned int slot)
{
    make_ssbo(ssbo, ID);
    check(ssbo, );

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, ssbo->_M_ID);
}

API void api_set_ssbo_data(const ObjID& ID, void* data, std::size_t bytes, BufferUsage mode)
{
    make_ssbo(ssbo, ID);
    check(ssbo, );
    internal_bind_ssbo(ssbo->_M_ID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, data, _M_draw_modes.at(mode));
    internal_bind_ssbo(0);
}

API void api_update_ssbo_data(const ObjID& ID, void* data, std::size_t bytes, std::size_t offset)
{
    make_ssbo(ssbo, ID);
    check(ssbo, );
    internal_bind_ssbo(ssbo->_M_ID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, bytes, data);
    internal_bind_ssbo(0);
}
