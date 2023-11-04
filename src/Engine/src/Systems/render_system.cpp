#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/global_uniform_buffer.hpp>
#include <Graphics/rhi.hpp>
#include <Platform/thread.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/render_system.hpp>


namespace Engine
{

    class RenderSystemUpdate : public ExecutableObject
    {
    private:
        RenderSystem* _M_system;

    public:
        RenderSystemUpdate(RenderSystem* system) : _M_system(system)
        {}

        int_t execute() override
        {
            return _M_system->private_update();
        }
    };

    RenderSystem& RenderSystem::create()
    {
        Super::create();
        EngineSystem::new_system<EngineSystem>()->register_subsystem(this);
        _M_render_thread = engine_instance->thread(ThreadType::RenderThread);
        _M_rhi           = engine_instance->rhi();
        _M_update        = new RenderSystemUpdate(this);

        return *this;
    }

    int_t RenderSystem::private_update()
    {
        _M_rhi->begin_render();
        GlobalUniformBuffer::instance()->update(_M_delta_time);
        Super::update(_M_delta_time);
        _M_rhi->end_render();
        _M_rhi->swap_buffer();
        return 0;
    }

    RenderSystem& RenderSystem::update(float dt)
    {
        _M_delta_time = dt;
        _M_render_thread->push_task(_M_update);
        return *this;
    }

    RenderSystem& RenderSystem::shutdown()
    {
        Super::shutdown();

        delete _M_update;
        _M_render_thread->wait_all();
        return *this;
    }

    RenderSystem& RenderSystem::wait()
    {

        _M_render_thread->wait_all();
        Super::wait();
        return *this;
    }

    implement_engine_class_default_init(RenderSystem);
}// namespace Engine
