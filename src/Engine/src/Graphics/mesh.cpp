#include <Core/engine.hpp>
#include <Graphics/mesh.hpp>
#include <api.hpp>
#include <numeric>

namespace Engine
{
    declare_instance_info_cpp(BasicMesh);
    constructor_cpp(BasicMesh)
    {}

    BasicMesh& BasicMesh::gen()
    {
        ApiObject::destroy();
        EngineInstance::instance()->api_interface()->generate_mesh(_M_ID);
        return *this;
    }

    BasicMesh& BasicMesh::set_data()
    {
        EngineInstance::instance()->api_interface()->mesh_data(_M_ID, mesh_type_size() * size(), draw_mode,
                                                               static_cast<void*>(mesh_data()));
        return *this;
    }

    BasicMesh& BasicMesh::set_indexes()
    {
        EngineInstance::instance()->api_interface()->mesh_indexes_array(_M_ID, indexes_size() * indexes_type_size(),
                                                                        indexes_type(), indexes_data());
        return *this;
    }


    BasicMesh& BasicMesh::update_indexes(size_t offset, size_t count, void* data)
    {
        if (!data && (offset + count) <= indexes_size() * indexes_type_size())
        {
            byte* address = indexes_data();
            address += offset;
            data = reinterpret_cast<void*>(address);
        }

        if (data)
            EngineInstance::instance()->api_interface()->update_mesh_indexes_array(_M_ID, offset, count, data);

        return *this;
    }

    const BasicMesh& BasicMesh::draw(Primitive primitive, size_t vertices, size_t offset) const
    {
        EngineInstance::instance()->api_interface()->draw_mesh(_M_ID, primitive,
                                                               glm::min(indexes_size() - offset, vertices), offset);
        return *this;
    }

    BasicMesh& BasicMesh::update_data(size_t offset, size_t count, void* data)
    {
        if (!data && (offset + count) <= size() * mesh_type_size())
        {
            byte* address = mesh_data();
            address += offset;
            data = reinterpret_cast<void*>(address);
        }

        if (data)
            EngineInstance::instance()->api_interface()->update_mesh_data(_M_ID, offset, count, data);
        return *this;
    }

}// namespace Engine
