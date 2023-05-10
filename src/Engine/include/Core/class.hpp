#pragma once
#include <Core/object.hpp>
#include <functional>


namespace Engine
{

    class ENGINE_EXPORT Class : public Object
    {
    public:
        using ClassesMap = Map<String, Class*>;

    private:
        const Class* _M_parent = nullptr;
        std::function<Object*()> _M_allocate_object;
        size_t _M_instance_size = 0;

    private:
        Class(const String& name = "", Class* parent = nullptr);

        template<typename InstanceClass, typename BaseClass>
        static void update_parent_class()
        {
            const Class* parent = const_cast<const Class*>(ClassMetaData<BaseClass>::find_class());
            Class* current      = const_cast<Class*>(ClassMetaData<InstanceClass>::find_class());
            current->_M_parent  = parent;
        }

        template<typename Instance, typename... Args>
        Class& create_allocator(Args... args)
        {
            _M_allocate_object = [&]() -> Object*{
                 return static_cast<Object*>(Object::new_instance<Instance>(args...));
            };
            return *this;
        }

    public:
        const Class* parent() const;
        bool contains_class(const Class* const instance, bool recursive = false) const;
        static Class* find_class(const String& name);
        static const ClassesMap& classes();
        Object* create() const;
        size_t instance_size() const;

        template<typename InstanceClass = void, typename BaseClass = void, typename... Args>
        static Class* register_class(const String& class_name, Args... args)
        {
            Class* class_instance = find_class(class_name);
            if (class_instance == nullptr)
            {
                class_instance                     = Object::new_instance_without_package<Class>(class_name);
                class_instance->create_allocator<InstanceClass>(args...);
                class_instance->_M_instance_size = sizeof(InstanceClass);
                extern Vector<void (*)()>& initialize_list();
                initialize_list().push_back(Class::update_parent_class<InstanceClass, BaseClass>);
            }

            return class_instance;
        }

        friend class Object;
    };
}// namespace Engine
