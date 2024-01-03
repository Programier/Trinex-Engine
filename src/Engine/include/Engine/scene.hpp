#pragma once
#include <Core/engine_types.hpp>
#include <Core/pointer.hpp>
#include <Engine/octree.hpp>

namespace Engine
{

    class PrimitiveComponent;
    class SceneComponent;

    class ENGINE_EXPORT SceneView final
    {
    };

    class ENGINE_EXPORT SceneInterface
    {
    public:
    private:
    public:
        virtual SceneInterface& add_primitive(PrimitiveComponent* primitive)    = 0;
        virtual SceneInterface& remove_primitive(PrimitiveComponent* primitive) = 0;
        virtual SceneComponent* root_component() const                          = 0;

        virtual ~SceneInterface();
    };

    class ENGINE_EXPORT Scene : public SceneInterface
    {
    public:
        using SceneOctree = Octree<PrimitiveComponent*>;

    private:
        SceneOctree _M_octree;
        Pointer<SceneComponent> _M_root_component;

    public:
        Scene();
        Scene& add_primitive(PrimitiveComponent* primitive) override;
        Scene& remove_primitive(PrimitiveComponent* primitive) override;
        SceneComponent* root_component() const override;
    };
}// namespace Engine
