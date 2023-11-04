#pragma once
#include <Core/package.hpp>

namespace Engine
{
    class ENGINE_EXPORT System : public Object
    {
        declare_class(System, Object);

    private:
        bool is_fully_created = false;
        static void on_create_fail();
        static void on_new_system(System* system);

    protected:
        Vector<System*> _M_subsystems;

    public:
        bool save(BufferWriter* writer = nullptr) const                         = delete;
        bool load(BufferReader* read = nullptr, bool clear = false)             = delete;
        Object* load_object(const String& name, BufferReader* reader = nullptr) = delete;

        System();
        virtual System& create();
        virtual System& wait();
        virtual System& update(float dt);
        virtual System& shutdown();
        static System* new_system_by_name(const String& name);

        System& on_child_remove(Object* object) override;
        System& on_child_set(Object* object) override;

        const Vector<System*>& subsystems() const;
        System& register_subsystem(System* system, Index index = ~static_cast<Index>(0));
        System& remove_subsystem(System* system);


        template<typename SystemType, typename... Args>
        static SystemType* new_system(Args&&... args)
        {
            if (SystemType::instance() == nullptr)
            {
                SystemType* system = SystemType::create_instance(std::forward<Args>(args)...);
                on_new_system(system);
                return system;
            }

            return SystemType::instance();
        }
        ~System();
        Identifier id() const;
    };
}// namespace Engine
