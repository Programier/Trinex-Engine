#include <Core/class.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Systems/event_system.hpp>
#include <Systems/imgui_system.hpp>
#include <imgui.h>

namespace Engine
{
    class TrinexEditorSystem : public Singletone<TrinexEditorSystem, System>
    {
        declare_class(TrinexEditorSystem, System);

        static TrinexEditorSystem* _M_instance;
        Index respect_count = 0;
        Identifier listener_id;
        KeyEvent event;

    public:
        TrinexEditorSystem& create() override
        {
            Super::create();
            System::new_system<ImGuiRendererSystem>()->register_subsystem(this);

            event.key   = Keyboard::F;
            listener_id = System::new_system<EventSystem>()->add_listener(
                    Event(EventType::KeyDown, event),
                    std::bind(&TrinexEditorSystem::on_f_press, this, std::placeholders::_1));
            return *this;
        }

        void on_f_press(const Event& event)
        {
            respect_count++;
        }

        TrinexEditorSystem& wait() override
        {
            Super::wait();
            return *this;
        }

        TrinexEditorSystem& update(float dt) override
        {
            ImGui::Begin("Hello :)");
            ImGui::Text("This will be editor for Trinex Engine");
            if(ImGui::Button("Press F to pay respect"))
            {
                respect_count++;
            }

            if(respect_count > 0)
            {
                ImGui::Separator();
                ImGui::NewLine();

                for(Index i = 0; i < respect_count; i++)
                {
                    ImGui::Text("Thank you for respect :D");
                }
            }

            ImGui::End();

            Super::update(dt);
            return *this;
        }

        TrinexEditorSystem& shutdown() override
        {
            Super::shutdown();
            EventSystem::instance()->remove_listener(Event(EventType::KeyDown, event), listener_id);
            return *this;
        }

        friend class Object;
        friend class Singletone<TrinexEditorSystem, System>;
    };

    TrinexEditorSystem* TrinexEditorSystem::_M_instance = nullptr;
    implement_engine_class_default_init(TrinexEditorSystem);
}// namespace Engine
