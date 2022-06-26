#pragma once
#include <vector>
#include <BasicFunctional/engine_types.hpp>
#include <BasicFunctional/reference_wrapper.hpp>

namespace Engine
{
#define BASIC_TEXTURE                                                                                                                 \
    {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,                                    \
     1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f},                                   \
            6,                                                                                                                        \
    {                                                                                                                                 \
        3, 2                                                                                                                          \
    }

    enum Primitive
    {
        POINT,
        LINE,
        TRIANGLE
    };


    class Mesh
    {
    private:
        bool _M_is_const = false;
    protected:

        std::vector<float> _M_data;
        std::size_t _M_buffer_size = 0;
        ObjectID _M_VAO = 0, _M_VBO = 0;
        ObjectID _M_vertices = 0;
        std::vector<int> _M_attrib;
        float* sub_data = nullptr;
        void gen_vbo_vao();
        void delete_vbo_vao();

    public:
        Mesh();
        Mesh(const Mesh&);
        Mesh& operator=(const Mesh&);
        Mesh& load(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
        Mesh(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
        Mesh& sub_mesh(Mesh& mesh, std::size_t primitive_offset, std::size_t primitive_count);

        Mesh& draw(const Primitive& primitive);

        std::vector<float>& data();
        const std::vector<float>& data() const;

        Mesh& vertices_count(const unsigned int& vertices);
        unsigned int vertices_count() const;

        Mesh& attributes(const std::vector<int>& attributes);
        const std::vector<int>& attributes() const;

        Mesh& update_buffers();
        ~Mesh();
    };

}// namespace Engine
