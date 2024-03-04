#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/sprite_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
    implement_engine_class(SpriteComponent, 0);
    implement_initialize_class(SpriteComponent)
    {}

    SpriteComponent& SpriteComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        bounding_box().write_to_batcher(scene->scene_output_layer()->lines, {255, 255, 0, 255});
        scene->scene_output_layer()->add_component(this);
        return *this;
    }


    static Matrix4f rotate_sprite(const Transform& input_transform, const SceneView& view)
    {
        Vector3D location;
        Vector3D scale;
        Quaternion quat;
        Vector3D skew;
        Vector4D perspective;
        glm::decompose(input_transform.matrix(), scale, quat, location, skew, perspective);

        Matrix4f transform = glm::inverse(glm::lookAt(location, view.camera_view().location, Constants::OY));
        return glm::scale(transform, scale);
    }

    SpriteComponent& SpriteComponent::render(class SceneRenderer* renderer, class RenderTargetBase*, class SceneLayer*)
    {
        if (Material* material = Object::find_object_checked<Material>("DefaultPackage::SpriteMaterial"))
        {
            if (PositionVertexBuffer* vertex_bufer =
                        Object::find_object_checked<PositionVertexBuffer>("DefaultPackage::ScreenPositionBuffer"))
            {
                if (Mat4MaterialParameter* parameter =
                            reinterpret_cast<Mat4MaterialParameter*>(material->find_parameter("model")))
                {
                    Matrix4f model   = rotate_sprite(world_transform(), renderer->scene_view());
                    parameter->param = model;
                }

                BindingMaterialParameter* texture_parameter =
                        reinterpret_cast<BindingMaterialParameter*>(material->find_parameter("texture"));
                Texture* tmp = nullptr;

                if (texture_parameter && texture)
                {
                    tmp = texture_parameter->texture_param();
                    texture_parameter->texture_param(reinterpret_cast<Texture*>(texture));
                }

                material->apply(this);
                vertex_bufer->rhi_bind(0, 0);
                engine_instance->rhi()->draw(6);

                if (texture_parameter && texture)
                {
                    texture_parameter->texture_param(tmp);
                }
            }
        }
        return *this;
    }


}// namespace Engine
