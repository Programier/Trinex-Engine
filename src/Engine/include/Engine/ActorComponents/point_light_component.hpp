#pragma once

#include <Engine/ActorComponents/light_component.hpp>


namespace Engine
{
    class ENGINE_EXPORT PointLightComponent : public LightComponent
    {
        declare_class(PointLightComponent, LightComponent);

    public:
        float radius;

        PointLightComponent();
        Type light_type() const override;
    };

}// namespace Engine
