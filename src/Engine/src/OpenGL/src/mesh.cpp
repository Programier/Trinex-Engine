#include <cstdio>
#include <opengl_mesh.hpp>
#include <opengl_object.hpp>


using namespace Engine;


void OpenGL_Mesh::destroy()
{
    glDeleteBuffers(1, &_M_VBO);
    glDeleteVertexArrays(1, &_M_VAO);
    _M_VAO = _M_VBO = 0;
}

API void api_generate_mesh(ObjID& ID)
{
    if (ID)
        api_destroy_object_instance(ID);

    OpenGL_Mesh* mesh = new OpenGL_Mesh();

    glGenVertexArrays(1, &mesh->_M_VAO);
    glGenBuffers(1, &mesh->_M_VBO);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ID = object_id_of(mesh);
}

API void api_set_mesh_data(const ObjID& ID, MeshInfo& info, void* data)
{
    check_id(ID, );
    make_mesh(mesh, ID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);

    mesh->_M_buffer_size = 0;
    for (auto& attrib : info.attributes) mesh->_M_buffer_size += _M_buffer_value_type_sizes.at(attrib.type) * attrib.count;
    mesh->_M_buffer_size *= info.vertices;
    glBufferData(GL_ARRAY_BUFFER, mesh->_M_buffer_size, data, _M_draw_modes.at(info.mode));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

API void api_update_mesh_attributes(const ObjID& ID, MeshInfo& info)
{
    check_id(ID, );
    make_mesh(mesh, ID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
    glBindVertexArray(mesh->_M_VAO);
    // Setting attributes values
    std::uintptr_t offset = 0;
    int index = 0;

    std::size_t size = 0;
    for (auto& attrib : info.attributes) size += _M_buffer_value_type_sizes.at(attrib.type) * attrib.count;

    for (const auto& value : info.attributes)
    {
        glVertexAttribPointer(index, value.count, _M_buffer_value_types.at(value.type), GL_FALSE, size, (GLvoid*) (offset));
        offset += value.count * _M_buffer_value_type_sizes.at(value.type);
        glEnableVertexAttribArray(index++);
    }


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


API void api_draw_mesh(const ObjID& ID, Primitive primitive, std::size_t vertices, unsigned int start_index)
{
    check_id(ID, );
    make_mesh(mesh, ID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
    glBindVertexArray(mesh->_M_VAO);
    glDrawArrays(_M_primitives.at(primitive), start_index, vertices);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

API void api_update_mesh_data(const ObjID& ID, std::size_t offset, std::size_t size, void* data)
{
    check_id(ID, );
    make_mesh(mesh, ID);
    if (mesh->_M_buffer_size < offset + size)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
    glBindVertexArray(mesh->_M_VAO);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
