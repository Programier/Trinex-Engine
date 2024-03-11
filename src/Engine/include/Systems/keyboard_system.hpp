#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/keyboard.hpp>
#include <Systems/system.hpp>
#include <Event/listener_id.hpp>

namespace Engine
{
    class ENGINE_EXPORT KeyboardSystem : public Singletone<KeyboardSystem, System>
    {
        declare_class(KeyboardSystem, System);

    private:
        Keyboard::Status m_key_status[Keyboard::__COUNT__];
        Vector<Keyboard::Key> m_last_pressed_keys;
        Vector<Keyboard::Key> m_last_released_keys;


        EventSystemListenerID m_key_press_id, m_key_release_id;

        void on_key_pressed(const Event& event);
        void on_key_released(const Event& event);
        void process_last_keys(Vector<Keyboard::Key>& vector, Keyboard::Status status);
        KeyboardSystem& set(Keyboard::Key key, Keyboard::Status status);

    public:
        KeyboardSystem& create() override;
        KeyboardSystem& wait() override;
        KeyboardSystem& update(float dt) override;
        KeyboardSystem& shutdown() override;
        Keyboard::Status status_of(Keyboard::Key key) const;

        bool is_pressed(Keyboard::Key key) const;
        bool is_released(Keyboard::Key key) const;
        bool is_just_pressed(Keyboard::Key key) const;
        bool is_just_released(Keyboard::Key key) const;
        bool is_repeated(Keyboard::Key key) const;


#define implement_variadic_template(name)                                                                              \
    template<typename... Keys>                                                                                         \
    FORCE_INLINE bool name##_and(Keys&&... keys) const                                                                 \
    {                                                                                                                  \
        return (name(std::forward<Keys>(keys)) && ...);                                                                \
    }                                                                                                                  \
                                                                                                                       \
    template<typename... Keys>                                                                                         \
    FORCE_INLINE bool name##_or(Keys&&... keys) const                                                                  \
    {                                                                                                                  \
        return (name(std::forward<Keys>(keys)) || ...);                                                                \
    }


        implement_variadic_template(is_pressed);
        implement_variadic_template(is_released);
        implement_variadic_template(is_just_pressed);
        implement_variadic_template(is_just_released);
        implement_variadic_template(is_repeated);


#undef implement_variadic_template

        friend class Singletone<KeyboardSystem, System>;
    };
}// namespace Engine
