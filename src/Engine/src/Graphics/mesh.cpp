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
        set_mesh_data(_M_ID, dynamic_cast<MeshInfo&>(*this), data);
        return *this;
    }

    BasicMesh& BasicMesh::update_atributes()
    {
        update_mesh_attributes(_M_ID, dynamic_cast<MeshInfo&>(*this));
        return *this;
    }

    BasicMesh& BasicMesh::draw(Primitive primitive, unsigned int vertices, unsigned int start_index)
    {
        draw_mesh(_M_ID, primitive, (vertices ? vertices : dynamic_cast<MeshInfo&>(*this).vertices), start_index);
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
            update_mesh_date(_M_ID, offset, count, data);
        return *this;
    }

}// namespace Engine
