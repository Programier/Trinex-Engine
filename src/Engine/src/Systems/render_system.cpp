#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/thread.hpp>
#include <Graphics/global_uniform_buffer.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/render_system.hpp>


namespace Engine
{

    RenderSystem& RenderSystem::create()
    {
        Super::create();
        EngineSystem::new_system<EngineSystem>()->register_subsystem(this);
        _M_render_thread = engine_instance->thread(ThreadType::RenderThread);
        _M_rhi = engine_instance->rhi();
        return *this;
    }

    RenderSystem& RenderSystem::update(float dt)
    {
        if (Thread::this_thread() != engine_instance->thread(ThreadType::RenderThread))
        {
            _M_render_thread->wait_all();// Wait finish render of prev frame before starting rendering new frame
            _M_render_thread->insert_new_task<UpdateTask>(this, dt);
        }
        else
        {
            _M_rhi->begin_render();
            GlobalUniformBuffer::instance()->update(dt);
            Super::update(dt);
            _M_rhi->end_render();
            _M_rhi->swap_buffer();
        }
        return *this;
    }

    RenderSystem& RenderSystem::shutdown()
    {
        _M_render_thread->wait_all();
        Super::shutdown();
        _M_render_thread->wait_all();
        return *this;
    }

    RenderSystem& RenderSystem::wait()
    {
        Super::wait();
        return *this;
    }

    implement_engine_class_default_init(RenderSystem);
}// namespace Engine
