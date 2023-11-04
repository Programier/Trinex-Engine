#pragma once

#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>

namespace Engine
{


    class ENGINE_EXPORT ImGuiRendererSystem : public Singletone<ImGuiRendererSystem, System>
    {
        declare_class(ImGuiRendererSystem, System);

    public:
        ImGuiRendererSystem& create() override;
        ImGuiRendererSystem& update(float dt) override;
        ImGuiRendererSystem& wait() override;
        ImGuiRendererSystem& shutdown() override;
    };
}// namespace Engine
