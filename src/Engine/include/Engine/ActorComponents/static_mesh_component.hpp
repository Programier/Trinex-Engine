#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine
{
    class ENGINE_EXPORT StaticMeshComponent : public PrimitiveComponent
    {
        declare_class(StaticMeshComponent, PrimitiveComponent);

    public:
        class StaticMesh* mesh = nullptr;

        StaticMeshComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override;
        StaticMeshComponent& render(class SceneRenderer*, class RenderViewport*, class SceneLayer*) override;
    };
}// namespace Engine