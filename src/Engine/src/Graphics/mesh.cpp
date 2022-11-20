#include <Graphics/mesh.hpp>
#include <api_funcs.hpp>
#include <numeric>
#include <opengl.hpp>

namespace Engine
{
    implement_class_cpp(BasicMesh);

    BasicMesh& BasicMesh::gen()
    {
        Object::destroy();
        generate_mesh(_M_ID);
        return *this;
    }

    BasicMesh& BasicMesh::set_data(void* data)
    {
        set_mesh_data(_M_ID, mesh_type_size() * size(), mode, static_cast<void*>(mesh_data()));
        return *this;
    }

    BasicMesh& BasicMesh::update_atributes()
    {
        update_mesh_attributes(_M_ID, dynamic_cast<MeshInfo&>(*this));
        return *this;
    }

    BasicMesh& BasicMesh::update_indexes()
    {
        set_mesh_indexes_array(_M_ID, dynamic_cast<MeshInfo&>(*this), indexes_size(), indexes_type(), indexes_data());
        return *this;
    }

    const BasicMesh& BasicMesh::draw(Primitive primitive, std::size_t vertices, std::size_t offset) const
    {
        draw_mesh(_M_ID, primitive, (vertices ? vertices : dynamic_cast<const MeshInfo&>(*this).vertices), offset);
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
            update_mesh_data(_M_ID, offset, count, data);
        return *this;
    }

}// namespace Engine
