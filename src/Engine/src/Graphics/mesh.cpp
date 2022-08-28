#include <Graphics/mesh.hpp>
#include <numeric>
#include <opengl.hpp>

namespace Engine
{

    Mesh::Mesh() = default;
    void Mesh::gen_vbo_vao()
    {
        int vertex_size = 0;
        for (const auto& ell : _M_attrib) vertex_size += ell;

        glGenVertexArrays(1, &_M_VAO);
        glGenBuffers(1, &_M_VBO);

        glBindVertexArray(_M_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _M_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_size * _M_vertices, _M_data.data(), GL_STATIC_DRAW);

        // Setting attributes values
        int offset = 0;
        int index = 0;
        for (const auto& value : _M_attrib)
        {
            glVertexAttribPointer(index, value, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float),
                                  (GLvoid*) (offset * sizeof(float)));
            glEnableVertexAttribArray(index++);
            offset += value;
        }

        glBindVertexArray(0);
    }

    void Mesh::delete_vbo_vao()
    {
        if (_M_VAO != 0)
            glDeleteVertexArrays(1, &_M_VAO);
        if (_M_VBO != 0)
            glDeleteBuffers(1, &_M_VBO);
    }

    Mesh::Mesh(const Mesh& mesh)
    {
        *this = mesh;
    }

    Mesh& Mesh::operator=(const Mesh& mesh)
    {
        if (this == &mesh)
            return *this;
        delete_vbo_vao();
        _M_data = mesh._M_data;
        _M_vertices = mesh._M_vertices;
        _M_attrib = mesh._M_attrib;
        gen_vbo_vao();
        return *this;
    }

    Mesh::Mesh(Mesh&& mesh)
    {
        *this = std::move(mesh);
    }

    Mesh& Mesh::operator=(Mesh&& mesh)
    {
        if (this == &mesh)
            return *this;

        delete_vbo_vao();

        _M_data = std::move(mesh._M_data);
        _M_VAO = mesh._M_VAO, _M_VBO = mesh._M_VBO;
        _M_vertices = mesh._M_vertices;
        _M_attrib = std::move(mesh._M_attrib);
        mesh._M_VAO = mesh._M_VBO = mesh._M_vertices = 0;
        return *this;
    }

    Mesh& Mesh::load(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes)
    {
        delete_vbo_vao();
        _M_data = data;
        _M_vertices = vertices;
        _M_attrib = attributes;

        gen_vbo_vao();
        return *this;
    }

    Mesh::Mesh(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes)
        : _M_data(data), _M_vertices(vertices), _M_attrib(attributes)
    {
        gen_vbo_vao();
    }

    Mesh& Mesh::draw(const Primitive& primitive)
    {
        glBindVertexArray(_M_VAO);
        auto gl_primitive = primitive == POINT ? GL_POINTS : primitive == LINE ? GL_LINES : GL_TRIANGLES;
        glDrawArrays(gl_primitive, 0, _M_vertices);
        glBindVertexArray(0);
        return *this;
    }

    Mesh::~Mesh()
    {
        delete_vbo_vao();
    }

    std::vector<float>& Mesh::data()
    {
        return _M_data;
    }

    Mesh& Mesh::vertices_count(const unsigned int& vertices)
    {
        _M_vertices = vertices;
        return *this;
    }

    unsigned int Mesh::vertices_count() const
    {
        return _M_vertices;
    }

    Mesh& Mesh::attributes(const std::vector<int>& attributes)
    {
        _M_attrib = attributes;
        return *this;
    }

    const std::vector<int>& Mesh::attributes() const
    {
        return _M_attrib;
    }

    Mesh& Mesh::update_buffers()
    {
        delete_vbo_vao();
        gen_vbo_vao();
        return *this;
    }

    const std::vector<float>& Mesh::data() const
    {
        return _M_data;
    }

    Mesh& Mesh::sub_mesh(Mesh& mesh, std::size_t primitive_offset, std::size_t primitive_count)
    {}

}// namespace Engine
