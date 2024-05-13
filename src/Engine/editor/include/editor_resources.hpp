#pragma once


namespace Engine
{
    class Sampler;
    class Texture2D;
    class Material;
    class PositionVertexBuffer;

    namespace EditorResources
    {
        extern Texture2D* default_icon;
        extern Texture2D* add_icon;
        extern Texture2D* move_icon;
        extern Texture2D* remove_icon;
        extern Texture2D* rotate_icon;
        extern Texture2D* scale_icon;
        extern Texture2D* select_icon;
        extern Texture2D* more_icon;
        extern Texture2D* light_sprite;
        extern Texture2D* blueprint_texture;
        extern Sampler* default_sampler;
        extern Material* axis_material;
        extern Material* grid_material;
        extern Material* point_light_overlay_material;
        extern Material* spot_light_overlay_material;
        extern PositionVertexBuffer* spot_light_overlay_positions;
        extern PositionVertexBuffer* point_light_overlay_positions;
    }// namespace EditorResources
}// namespace Engine
