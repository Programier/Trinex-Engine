#include <Core/class.hpp>
#include <Systems/imgui_system.hpp>
#include <imgui.h>
#include <Core/thread.hpp>


namespace Engine
{
    class SystemGraphRendererSystem : public Singletone<SystemGraphRendererSystem, System>
    {
        declare_class(SystemGraphRendererSystem, System);

    public:
        SystemGraphRendererSystem& create() override
        {
            Super::create();
            System::new_system<ImGuiRendererSystem>()->register_subsystem(this);
            return *this;
        }

        static void show_system_tree(System* system)
        {
            if(ImGui::TreeNodeEx(system->string_name().c_str()))
            {
                for(System* subsystem : system->subsystems())
                {
                    show_system_tree(subsystem);
                }

                ImGui::TreePop();
            }
        }

        SystemGraphRendererSystem& update(float dt) override
        {
            Super::update(dt);
            System* base_system = this;

            while(base_system->parent_system())
            {
                base_system = base_system->parent_system();
            }

            ImGui::Begin("System Graph");

            ImGui::Text("FPS: %f\n", 1.0f / dt);
            show_system_tree(base_system);

            ImGui::End();

            return *this;
        }
    };

    implement_engine_class_default_init(SystemGraphRendererSystem);
}// namespace Engine
