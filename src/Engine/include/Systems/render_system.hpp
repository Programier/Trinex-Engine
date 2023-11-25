#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT RenderSystem : public Singletone<RenderSystem, System>
    {
        declare_class(RenderSystem, System);
        class Thread* _M_render_thread;
        struct RHI* _M_rhi;

    public:
        RenderSystem& create() override;
        RenderSystem& update(float dt) override;
        RenderSystem& shutdown() override;
        RenderSystem& wait() override;
        class Class* depends_on() const override;
        friend class Object;
        friend class RenderSystemUpdate;
        friend class RenderSystemSync;
    };
}// namespace Engine
