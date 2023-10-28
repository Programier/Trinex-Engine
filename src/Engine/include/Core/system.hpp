#pragma once
#include <Core/package.hpp>

namespace Engine
{
    class ENGINE_EXPORT System : public Package
    {
        declare_class(System, Package);

    private:
        bool is_fully_created = false;
        static void on_create_fail();

    public:
        bool save(BufferWriter* writer = nullptr) const                         = delete;
        bool load(BufferReader* read = nullptr, bool clear = false)             = delete;
        Object* load_object(const String& name, BufferReader* reader = nullptr) = delete;

        System();
        virtual System& create();
        virtual void wait();
        virtual System& update(float dt);
        virtual System& shutdown();

        template<typename SystemType, typename... Args>
        static SystemType* new_system(Args&&... args)
        {
            if (SystemType::instance() == nullptr)
            {
                SystemType* system = SystemType::create_instance(std::forward<Args>(args)...);
                system->create();
                if(!static_cast<System*>(system)->is_fully_created)
                {
                    on_create_fail();
                }
                return system;
            }

            return SystemType::instance();
        }
        ~System();
        Identifier id() const;
    };
}// namespace Engine
