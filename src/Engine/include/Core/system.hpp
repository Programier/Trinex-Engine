#pragma once
#include <Core/executable_object.hpp>
#include <Core/package.hpp>

namespace Engine
{
    class ENGINE_EXPORT System : public Object
    {
        declare_class(System, Object);

    public:
        class UpdateTask : public ExecutableObject
        {
        private:
            System* _M_system;
            float _M_dt;

        public:
            UpdateTask(System* system, float dt);
            int_t execute() override;
        };


    private:
        bool is_fully_created = false;
        static void on_create_fail();
        static void on_new_system(System* system);

    protected:
        Vector<System*> _M_subsystems;
        System* _M_parent_system;

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

        const Vector<System*>& subsystems() const;
        System& register_subsystem(System* system);
        System& remove_subsystem(System* system);
        System* parent_system() const;
        System& sort_subsystems();

        virtual class Class* depends_on() const;


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
