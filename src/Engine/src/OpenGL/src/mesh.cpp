#include <cstdio>
#include <opengl_mesh.hpp>
#include <opengl_object.hpp>


#define NEW_MESH

using namespace Engine;


void OpenGL_Mesh::destroy()
{

    glDeleteBuffers(1, &_M_VBO);
    glDeleteBuffers(1, &_M_EBO);
    glDeleteVertexArrays(1, &_M_VAO);
    _M_VAO = _M_VBO = 0;
    _M_EBO = 0;

    DEALLOC_INFO;
}

declare_cpp_destructor(OpenGL_Mesh);

API void api_generate_mesh(ObjID& ID)
{
    if (ID)
        api_destroy_object_instance(ID);

    OpenGL_Mesh* mesh = new OpenGL_Mesh();

    glGenBuffers(1, &mesh->_M_VBO);
    glGenBuffers(1, &mesh->_M_EBO);
    glGenVertexArrays(1, &mesh->_M_VAO);

    ID = object_id_of(mesh);
}

API void api_set_mesh_data(const ObjID& ID, std::size_t buffer_len, DrawMode mode, void* data)
{
    make_mesh(mesh, ID);
    check(mesh, );
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);

    mesh->_M_buffer_size = buffer_len;

    glBufferData(GL_ARRAY_BUFFER, mesh->_M_buffer_size, data, _M_draw_modes.at(mode));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

API void api_update_mesh_attributes(const ObjID& ID, const MeshInfo& info)
{
    make_mesh(mesh, ID);
    check(mesh, );
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
    glBindVertexArray(mesh->_M_VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_M_EBO);

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


API void api_draw_mesh(const ObjID& ID, Primitive primitive, std::size_t vertices, std::size_t offset)
{
    make_mesh(mesh, ID);
    check(mesh, );

    glBindVertexArray(mesh->_M_VAO);

    glDrawElements(_M_primitives.at(primitive), vertices, mesh->_M_indexes_type, reinterpret_cast<void*>(offset));

    glBindVertexArray(0);
}

API void api_update_mesh_data(const ObjID& ID, std::size_t offset, std::size_t size, void* data)
{
    make_mesh(mesh, ID);
    check(mesh, );
    if (mesh->_M_buffer_size < offset + size)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->_M_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

API void api_set_mesh_indexes_array(const ObjID& ID, const MeshInfo& info, std::size_t bytes,
                                    const BufferValueType& data_type, void* data)
{
    make_mesh(mesh, ID);
    check(mesh, );

    glBindVertexArray(mesh->_M_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_M_EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, data, _M_draw_modes.at(info.mode));
    mesh->_M_indexes_type = _M_buffer_value_types.at(data_type);

    glBindVertexArray(0);
}
