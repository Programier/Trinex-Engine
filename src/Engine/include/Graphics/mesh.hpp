#pragma once
#include <vector>


namespace Engine
{
#define BASIC_TEXTURE                                                                                                  \
    {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,                     \
     1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f},                    \
            6,                                                                                                         \
    {                                                                                                                  \
        3, 2                                                                                                           \
    }

    enum Primitive
    {
        POINT,
        LINE,
        TRIANGLE
    };


    class Mesh
    {
        std::vector<float> _M_data;
        unsigned int _M_VAO = 0, _M_VBO = 0;
        unsigned int _M_vertices = 0;
        std::vector<int> _M_attrib;

        void gen_vbo_vao();
        void delete_vbo_vao();

    public:
        Mesh();
        Mesh(const Mesh&);
        Mesh& operator=(const Mesh&);
        Mesh& load(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);

        Mesh(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
        Mesh& draw(const Primitive& primitive);

        std::vector<float>& data();

        Mesh& vertices_count(const unsigned int& vertices);
        unsigned int vertices_count();

        Mesh& attributes(const std::vector<int>& attributes);
        const std::vector<int>& attributes();

        Mesh& update_buffers();
        ~Mesh();
    };

}// namespace Engine
