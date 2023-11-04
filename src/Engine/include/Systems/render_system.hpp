#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT RenderSystem : public Singletone<RenderSystem, System>
    {
        declare_class(RenderSystem, System);

    private:
        class RenderSystemUpdate* _M_update = nullptr;
        class RenderSystemSync* _M_sync     = nullptr;

        class Thread* _M_render_thread = nullptr;
        struct RHI* _M_rhi             = nullptr;
        float _M_delta_time;


        int_t private_update();

    public:
        RenderSystem& create() override;
        RenderSystem& update(float dt) override;
        RenderSystem& shutdown() override;
        RenderSystem& wait() override;
        friend class Object;
        friend class RenderSystemUpdate;
        friend class RenderSystemSync;
    };
}// namespace Engine
