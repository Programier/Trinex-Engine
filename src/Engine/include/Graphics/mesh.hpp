#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_value_type.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <vector>


namespace Engine
{

    CLASS BasicMesh : public ApiObject, public MeshInfo
    {
        declare_instance_info_hpp(BasicMesh);

    public:
        constructor_hpp(BasicMesh);
        delete_copy_constructors(BasicMesh);

        BasicMesh& gen();
        BasicMesh& set_data(void* data);
        BasicMesh& update_data(std::size_t offset, std::size_t size, void* data = nullptr);
        BasicMesh& update_atributes();
        BasicMesh& update_indexes();
        const BasicMesh& draw(Primitive primitive, std::size_t vertices = 0, std::size_t offset = 0) const;

        virtual std::size_t size() const = 0;
        virtual std::uint8_t mesh_type_size() const = 0;
        virtual byte* mesh_data() = 0;
        virtual const byte* mesh_data() const = 0;

        // Indexes
        virtual void* indexes_data() const = 0;
        virtual std::size_t indexes_size() const = 0;
        virtual BufferValueType indexes_type() const = 0;
        virtual std::size_t vertices_count() const = 0;
    };


    template<typename Type, typename IndexType = unsigned int>
    STRUCT Mesh : public BasicMesh
    {
        std::vector<Type> data;
        std::vector<IndexType> indexes;

    private:
        const BufferValueType _M_indexes_engine_type = get_type_by_typeid(typeid(IndexType));

        declare_instance_info_template(Mesh);

    public:
        constructor_hpp(Mesh) = default;
        delete_copy_constructors(Mesh);

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

        void* indexes_data() const override
        {
            return (void*) (indexes.data());
        }

        std::size_t indexes_size() const override
        {
            return indexes.size() * sizeof(IndexType);
        }

        BufferValueType indexes_type() const override
        {
            return _M_indexes_engine_type;
        }

        std::size_t vertices_count() const override
        {
            return indexes.size();
        }
    };

}// namespace Engine
