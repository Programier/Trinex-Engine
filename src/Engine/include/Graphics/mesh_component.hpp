#pragma once
#include <Core/constants.hpp>
#include <Core/object_ref.hpp>
#include <Core/pointer.hpp>
#include <Core/scene_component.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine
{
    class MaterialInterface;


    class ENGINE_EXPORT MeshComponent : public SceneComponent
    {
        declare_class(MeshComponent, SceneComponent);
    };


    class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
    {
        declare_class(StaticMeshComponent, MeshComponent);
    };

    class ENGINE_EXPORT DynamicMeshComponent : public MeshComponent
    {
        declare_class(DynamicMeshComponent, MeshComponent);
    };
}// namespace Engine
