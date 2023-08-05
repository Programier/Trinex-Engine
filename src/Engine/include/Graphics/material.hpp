#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/shader_resource.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class Shader;
    class MeshComponent;

    struct ENGINE_EXPORT MaterialLayout {
        size_t vertex_size = 0;
        Vector<size_t> offsets;

        bool operator==(const MaterialLayout& layout) const;
    };


    class ENGINE_EXPORT MaterialApplier;

    class ENGINE_EXPORT Material : public ShaderResource
    {
    public:
        using Super       = Object;
        using TexturesMap = TreeMap<BindingIndex, Pointer<Texture>>;

    private:
        Vector<MaterialApplier*> _M_appliers;
        TexturesMap _M_textures;

    public:
        MaterialApplier* create_material_applier(MeshComponent* mesh);
        const Material& apply_resources() const;
        bool archive_process(Archive* archive);

        Material& add_texture(BindingIndex index, Texture* texture);
        Material& remove_texture(BindingIndex index);
        const TexturesMap& textures() const;

        ~Material();

        static void on_class_register(void*);
    };


    class ENGINE_EXPORT MaterialApplier
    {
    private:
        Shader* _M_shader;
        Material* _M_material;
        MaterialLayout _M_layout;
        BindingIndex _M_global_ubo_index;

        // Only material can construct and destruct instance of this class
        MaterialApplier(Shader* _M_shader, Material* _M_material, BindingIndex global_ubo = 255);
        ~MaterialApplier();

    public:
        MaterialApplier& apply();
        Shader* shader() const;
        Material* material() const;

        friend class Material;
    };

}// namespace Engine
