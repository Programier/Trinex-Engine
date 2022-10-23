#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <vector>


namespace Engine
{


    //    class Mesh : public Object
    //    {
    //    protected:
    //        std::vector<float> _M_data;
    //        ObjectID _M_VAO = 0, _M_VBO = 0;
    //        ObjectID _M_vertices = 0;
    //        std::vector<int> _M_attrib;
    //        void gen_vbo_vao();
    //        void delete_vbo_vao();

    //    public:
    //        Mesh();
    //        Mesh(const Mesh&);
    //        Mesh& operator=(const Mesh&);
    //        Mesh(Mesh&&);
    //        Mesh& operator =(Mesh&&);
    //        Mesh& load(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
    //        Mesh(const std::vector<float>& data, unsigned int vertices, const std::vector<int>& attributes);
    //        Mesh& sub_mesh(Mesh& mesh, std::size_t primitive_offset, std::size_t primitive_count);

    //        Mesh& draw(const Primitive& primitive);

    //        std::vector<float>& data();
    //        const std::vector<float>& data() const;

    //        Mesh& vertices_count(const unsigned int& vertices);
    //        unsigned int vertices_count() const;

    //        Mesh& attributes(const std::vector<int>& attributes);
    //        const std::vector<int>& attributes() const;

    //        Mesh& update_buffers();
    //        ~Mesh();
    //    };


    CLASS BasicMesh : public Object, public MeshInfo
    {
    public:
        implement_class_hpp(BasicMesh);

        BasicMesh& gen();
        BasicMesh& set_data(void* data);
        BasicMesh& update_data(std::size_t offset, std::size_t size, void* data);
        BasicMesh& update_atributes();
        BasicMesh& draw(Primitive primitive, unsigned int vertices = 0, unsigned int start_index = 0);
    };


    template<typename Type>
    struct Mesh : public BasicMesh {
        std::vector<Type> data;
        Mesh() = default;
        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) = default;
        Mesh& operator=(const Mesh&) = default;
        Mesh& operator=(Mesh&&) = default;

        Mesh& set_data()
        {
            BasicMesh::set_data(static_cast<void*>(data.data()));
            return *this;
        }

        Mesh& update_data(std::size_t byte_offset, std::size_t byte_count)
        {
            if ((byte_offset + byte_count) <= data.size() * sizeof(Type))
            {
                byte* address = reinterpret_cast<byte*>(data.data());
                address += byte_offset;
                BasicMesh::update_data(byte_offset, byte_count, static_cast<void*>(address));
            }
            return *this;
        }
    };

}// namespace Engine
