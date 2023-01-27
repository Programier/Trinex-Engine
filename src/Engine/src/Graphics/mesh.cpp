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
        EngineInstance::get_instance()->api_interface()->generate_mesh(_M_ID);
        return *this;
    }

    BasicMesh& BasicMesh::set_data(void* data)
    {
        EngineInstance::get_instance()->api_interface()->mesh_data(_M_ID, mesh_type_size() * size(), mode,
                                                                   static_cast<void*>(mesh_data()));
        return *this;
    }

    BasicMesh& BasicMesh::update_atributes()
    {
        EngineInstance::get_instance()->api_interface()->update_mesh_attributes(_M_ID, dynamic_cast<MeshInfo&>(*this));
        return *this;
    }

    BasicMesh& BasicMesh::update_indexes()
    {
        EngineInstance::get_instance()->api_interface()->mesh_indexes_array(
                _M_ID, dynamic_cast<MeshInfo&>(*this), indexes_size(), indexes_type(), indexes_data());
        return *this;
    }

    const BasicMesh& BasicMesh::draw(Primitive primitive, std::size_t vertices, std::size_t offset) const
    {
        EngineInstance::get_instance()->api_interface()->draw_mesh(_M_ID, primitive,
                                                                   (vertices ? vertices : vertices_count()), offset);
        return *this;
    }

    BasicMesh& BasicMesh::update_data(std::size_t offset, std::size_t count, void* data)
    {
        if (!data && (offset + count) <= size() * mesh_type_size())
        {
            byte* address = mesh_data();
            address += offset;
            data = reinterpret_cast<void*>(address);
        }

        if (data)
            EngineInstance::get_instance()->api_interface()->update_mesh_data(_M_ID, offset, count, data);
        return *this;
    }

}// namespace Engine
