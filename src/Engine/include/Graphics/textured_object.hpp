#pragma once

#include <Core/export.hpp>
#include <Graphics/drawable_object.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT TexturedObject : public DrawableObject, public Mesh<float>
    {
    protected:
        Texture2D _M_diffuse;

    public:
        TexturedObject();
        DrawableObject* copy() const override;
        const Texture2D& diffuse_texture() const;
        TexturedObject& diffuse_texture(const Texture2D& texture);

        bool is_empty_layer() const override;
        void render_layer(const glm::mat4& prev, on_render_layer_func = empty_drawable_callback_handler) const override;
    };
}
