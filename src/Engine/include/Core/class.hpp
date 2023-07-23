#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>


namespace Engine
{
    class ENGINE_EXPORT Class : public Object
    {
    public:
        using ClassesMap = Map<String, Class*>;

    private:
        const Class* _M_parent = nullptr;
        Set<const Class*> _M_parents;
        std::function<Object*()> _M_allocate_object;
        std::function<Object*()> _M_allocate_without_package;
        std::function<Lua::object(Object*)> _M_to_lua_object;
        void (*_M_resolve_inherit)() = nullptr;
        size_t _M_instance_size      = 0;

        template<typename Instance>
        static Instance* lua_allocate()
        {
            Object* object = const_cast<Class*>(ClassMetaData<Instance>::find_class())->_M_allocate_object();
            return object->instance_cast<Instance>();
        }

    private:
        Class(const String& name = "");
        void update_parent_classes(const Class* parent);

        template<typename Instance, typename... Args>
        Class& create_allocator(Args... args)
        {
            if constexpr (std::is_abstract_v<Instance>)
            {
                _M_allocate_object          = []() -> Object* { return nullptr; };
                _M_allocate_without_package = _M_allocate_object;
            }
            else
            {
                _M_allocate_object = [&]() -> Object* {
                    return static_cast<Object*>(Object::new_instance<Instance>(args...));
                };

                _M_allocate_without_package = [&]() -> Object* {
                    return static_cast<Object*>(Object::new_instance_without_package<Instance>(args...));
                };
            }
            return *this;
        }

        template<typename CurrentType, typename... Args>
        static Lua::base_list<CurrentType, Args...> mix_parents(const Lua::base_list<Args...>&)
        {
            return Lua::base_list<CurrentType, Args...>();
        }

        template<typename Type>
        static decltype(auto) class_parents()
        {
            constexpr bool has_parent = has_super_type<Type>::value;

            if constexpr (has_parent)
            {
                if constexpr (std::is_base_of_v<Object, typename Type::Super>)
                {
                    auto p = class_parents<typename Type::Super>();
                    return mix_parents<Type>(p);
                }
                else
                {
                    return Lua::base_list<Type>();
                }
            }
            else
            {
                return Lua::base_list<Type>();
            }
        }


        template<typename Instance>
        static Lua::object to_lua_object_private(Object* object)
        {
            return Lua::make_object(Lua::Interpretter::state()->lua_state(), reinterpret_cast<Instance*>(object));
        }

        template<typename Instance>
        static void private_resolve_inherit()
        {
            Class* current = const_cast<Class*>(ClassMetaData<Instance>::find_class());
            if constexpr (has_super_type_v<Instance>)
            {
                Class* parent  = const_cast<Class*>(ClassMetaData<typename Instance::Super>::find_class());
                current->update_parent_classes(parent);
            }

            current->_M_resolve_inherit = nullptr;
        }

    public:
        template<typename ClassInstance>
        class ClassRegistrar
        {
        private:
            Class* _M_class = nullptr;
            String _M_name;

            ClassRegistrar(Class* _class, const String& name) : _M_class(_class), _M_name(name)
            {}

        public:
            Lua::Class<ClassInstance> get() const
            {
                Lua::Class<ClassInstance> lua_class = Lua::Interpretter::lua_class_of<ClassInstance>(_M_name);

                if constexpr (has_super_type_v<ClassInstance>)
                {
                    auto base_classes = Class::class_parents<typename ClassInstance::Super>();
                    lua_class.set(Lua::base_classes, base_classes);
                }
                    auto invoker = new DefferedMethodInvoker(invoked_method, std::forward<Key>(key),
                                                             std::forward<Value>(value));

                if constexpr (std::is_base_of_v<Engine::Object, ClassInstance>)
                {
                    lua_class.set(Lua::meta_function::construct, lua_allocate<ClassInstance>);
                    lua_class.set(Lua::meta_function::to_string, &ClassInstance::as_string);
                }

                return lua_class;
            }

            friend class Class;
        };

        const Class* parent() const;
        bool contains_class(const Class* const instance) const;
        static Class* find_class(const String& name);
        static const ClassesMap& classes();
        Object* create() const;
        Object* create_without_package() const;
        size_t instance_size() const;
        Lua::object to_lua_object(Object* object) const;

        template<typename InstanceClass = void, typename... Args>
        static ClassRegistrar<InstanceClass> register_new_class(const String& class_name, Args... args)
        {
            Class* class_instance = find_class(class_name);
            if (class_instance == nullptr)
            {
                logger->log("Class", "Start initialize class '%s'", class_name.c_str());
                class_instance = Object::new_instance_without_package<Class>(class_name);
                class_instance->create_allocator<InstanceClass>(args...);
                class_instance->_M_instance_size   = sizeof(InstanceClass);
                class_instance->_M_to_lua_object   = to_lua_object_private<InstanceClass>;
                class_instance->_M_resolve_inherit = private_resolve_inherit<InstanceClass>;

                ClassMetaData<InstanceClass> meta_data(class_instance);
                InitializeController initializer(class_instance->_M_resolve_inherit);
                return ClassRegistrar<InstanceClass>(class_instance, class_name);
            }

            throw EngineException(Strings::format("Class {} already registered!", class_name));
        }

        friend class Object;
        friend struct LuaInterpretter;
        friend class Package;
    };
}// namespace Engine


#define register_class(class_name, ...)                                                                                \
    Engine::Class::register_new_class<class_name>(#class_name __VA_OPT__(, ##__VA_ARGS__))
