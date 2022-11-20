#pragma once
#include <Core/buffer_value_type.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <vector>


namespace Engine
{

    CLASS BasicMesh : public Object, public MeshInfo
    {
    protected:
    public:
        implement_class_hpp(BasicMesh);

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
    };


    template<typename Type, typename IndexType = unsigned int>
    STRUCT Mesh : public BasicMesh
    {
        std::vector<Type> data;
        std::vector<IndexType> indexes;

    private:
        const BufferValueType _M_indexes_engine_type = get_type_by_typeid(typeid(IndexType));

    public:
        Mesh() = default;
        Mesh(const Mesh& obj)
        {
            *this = obj;
        }
        Mesh(Mesh && obj)
        {
            *this = std::move(obj);
        }

        Mesh& operator=(const Mesh& obj)
        {
            if(this == &obj)
                return *this;
            data = obj.data;
            indexes = obj.indexes;
            return *this;
        }

        Mesh& operator=(Mesh&& obj)
        {
            if(this == &obj)
                return *this;
            data = std::move(obj.data);
            indexes = std::move(obj.indexes);
            return *this;
        }

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
    };

}// namespace Engine
