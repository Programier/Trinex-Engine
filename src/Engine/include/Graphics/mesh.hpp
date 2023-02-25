#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_value_type.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <vector>
#include <Core/constants.hpp>


namespace Engine
{

    CLASS BasicMesh : public ApiObject
    {
        declare_instance_info_hpp(BasicMesh);

    public:
        constructor_hpp(BasicMesh);
        delete_copy_constructors(BasicMesh);

        DrawMode draw_mode = DrawMode::StaticDraw;

        BasicMesh& gen();
        BasicMesh& set_data();
        BasicMesh& update_data(size_t offset, size_t size, void* data = nullptr);
        BasicMesh& set_indexes();
        BasicMesh& update_indexes(size_t offset, size_t size, void* data = nullptr);

        const BasicMesh& draw(Primitive primitive, size_t vertices = Constants::max_size, size_t offset = 0) const;

        virtual size_t size() const = 0;
        virtual std::uint8_t mesh_type_size() const = 0;
        virtual byte* mesh_data() = 0;
        virtual const byte* mesh_data() const = 0;

        // Indexes
        virtual const byte* indexes_data() const = 0;
        virtual byte* indexes_data() = 0;
        virtual size_t indexes_size() const = 0;
        virtual IndexBufferComponent indexes_type() const = 0;
        virtual size_t indexes_type_size() const = 0;
    };


    template<typename Type, typename IndexType = unsigned int>
    STRUCT Mesh : public BasicMesh
    {
        std::vector<Type> data;
        std::vector<IndexType> indexes;

    private:
        const IndexBufferComponent _M_indexes_engine_type = get_type_by_typeid(typeid(IndexType));

        declare_instance_info_template(Mesh);

    public:
        constructor_hpp(Mesh) = default;
        delete_copy_constructors(Mesh);

        size_t size() const override
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

        const byte* indexes_data() const override
        {
            return reinterpret_cast<const byte*>(indexes.data());
        }

        byte* indexes_data() override
        {
            return reinterpret_cast<byte*>(indexes.data());
        }

        size_t indexes_size() const override
        {
            return indexes.size();
        }

        IndexBufferComponent indexes_type() const override
        {
            return _M_indexes_engine_type;
        }

        size_t indexes_type_size() const override
        {
            return sizeof(IndexType);
        }
    };

}// namespace Engine
