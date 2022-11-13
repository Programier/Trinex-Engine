#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <vector>


namespace Engine
{

    CLASS BasicMesh : public Object, public MeshInfo
    {
    public:
        implement_class_hpp(BasicMesh);

        BasicMesh& gen();
        BasicMesh& set_data(void* data);
        BasicMesh& update_data(std::size_t offset, std::size_t size, void* data = nullptr);
        BasicMesh& update_atributes();
        BasicMesh& draw(Primitive primitive, unsigned int vertices = 0, unsigned int start_index = 0);

        virtual std::size_t size() const = 0;
        virtual std::uint8_t mesh_type_size() const = 0;
        virtual byte* mesh_data() = 0;
        virtual const byte* mesh_data() const = 0;
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

        std::size_t size() const override
        {
            return data.size();
        }

        std::uint8_t mesh_type_size() const override
        {
            return sizeof(Type);
        }

        byte* mesh_data() override
        {
            return reinterpret_cast<byte*>(data.data());
        }

        const byte* mesh_data() const override
        {
            return reinterpret_cast<const byte*>(data.data());
        }
    };

}// namespace Engine
