#pragma once
#include <Core/object.hpp>
#include <Graphics/shader_resource.hpp>

namespace Engine
{
    class Shader;
    class Mesh;

    struct ENGINE_EXPORT MaterialLayout {
        size_t vertex_size = 0;
        Vector<size_t> offsets;

        bool operator==(const MaterialLayout& layout) const;
    };


    class MaterialApplier;

    class ENGINE_EXPORT Material : public ShaderResource
    {
    private:
        Vector<MaterialApplier*> _M_appliers;

    public:
        MaterialApplier* create_material_applier(Mesh* mesh);
        const Material& apply_resources() const;
        bool archive_process(Archive* archive);
        ~Material();
    };


    class ENGINE_EXPORT MaterialApplier
    {
    private:
        Shader* _M_shader;
        Material* _M_material;
        MaterialLayout _M_layout;

        // Only material can construct and destruct instance of this class
        MaterialApplier(Shader* _M_shader, Material* _M_material);
        ~MaterialApplier();

    public:
        MaterialApplier& apply();
        Shader* shader() const;
        Material* material() const;

        friend class Material;
    };

}// namespace Engine
