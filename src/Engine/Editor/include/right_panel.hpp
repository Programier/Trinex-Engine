#pragma once
#include <panel.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/textured_object.hpp>


namespace Editor
{
    class RightPanel : public Panel
    {
        void render_position_object(Engine::Translate* object);
        void render_scale_object(Engine::Scale* object);
        void render_rotation_object(Engine::Rotate* object);
        void render_drawable_object(Engine::Drawable* object);
        void render_textured_object(Engine::TexturedObject* object);
        void render_transform_object(Engine::ModelMatrix* object);

    public:
        void render() override;
    };
}
