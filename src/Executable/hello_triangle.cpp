#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>


namespace Engine
{


    class HelloTriangleSystem : public Singletone<HelloTriangleSystem, System>
    {
        declare_class(HelloTriangleSystem, System);


    private:
        static HelloTriangleSystem* _M_instance;

    public:
        System& create() override
        {
            Super::create();
            EngineSystem::instance()->add_object(this);

            return *this;
        }

        void wait() override
        {
            Super::wait();
        }

        System& update(float dt) override
        {
            Super::update(dt);
            engine_instance->api_interface()->begin_render();
            engine_instance->window()->bind();

            engine_instance->api_interface()->end_render();

            return *this;
        }


        System& shutdown() override
        {
            Super::shutdown();
            return *this;
        }


        friend class Singletone<HelloTriangleSystem, System>;
    };

    HelloTriangleSystem* HelloTriangleSystem::_M_instance = nullptr;


    class HelloTriangle : public CommandLet
    {
        declare_class(HelloTriangle, CommandLet);

    public:
        int_t execute(int_t argc, char** argv) override
        {
            info_log("HelloTriangle", "Start");

            engine_instance->create_window();

            EventSystem::init_all();
            System::new_system<HelloTriangleSystem>();

            engine_instance->launch_systems();

            return 0;
        }
    };

    implement_class(HelloTriangle, "");
    implement_default_initialize_class(HelloTriangle);
    implement_class(HelloTriangleSystem, "Engine");
    implement_default_initialize_class(HelloTriangleSystem);
}// namespace Engine
