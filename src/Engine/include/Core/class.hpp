#pragma once
#include <Core/class_members.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    template<typename T, typename = void>
    struct has_generate_fields_info : std::false_type {
    };

    template<typename T>
    struct has_generate_fields_info<T, std::void_t<decltype(&T::generate_fields_info)>> : std::true_type {
    };

    class ENGINE_EXPORT Class final : public Package
    {
    public:
        using ClassesMap = Map<String, Class*>;
        using Super      = Package;

    private:
        const Class* _M_parent = nullptr;
        Set<const Class*> _M_parents;
        Vector<ClassField*> _M_fields;
        Function<Object*()> _M_allocate_object;
        Function<Object*()> _M_allocate_without_package;
        Function<Lua::object(Object*)> _M_to_lua_object;
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
            constexpr bool is_constructible = std::is_constructible_v<Instance, Args...>;
            constexpr bool is_abstract      = std::is_abstract_v<Instance>;
            if constexpr (!is_constructible || is_abstract)
            {
                if constexpr (!is_constructible && !is_abstract)
                {
                    String class_name = full_name();
                    warn_log("Class",
                             "Cannot create allocator for class '%s', because it is not possible to call the "
                             "constructor with the received parameters",
                             class_name.c_str());

                    _M_allocate_object = [class_name]() -> Object* {
                        error_log("Class",
                                  "Cannot create instance of '%s', because it is not possible to call the "
                                  "constructor with the received parameters",
                                  class_name.c_str());
                        return nullptr;
                    };
                }
                else if constexpr (is_abstract)
                {
                    String class_name = full_name();

                    _M_allocate_object = [class_name]() -> Object* {
                        error_log("Class", "Cannot create instance of '%s', because class '%s' is abstract!",
                                  class_name.c_str(), class_name.c_str());
                        return nullptr;
                    };
                }

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
                Class* parent = const_cast<Class*>(ClassMetaData<typename Instance::Super>::find_class());
                current->update_parent_classes(parent);
            }

            current->_M_resolve_inherit = nullptr;
        }

    public:
        template<typename ClassInstance>
        class ClassRegistrar
        {
        private:
            Lua::Class<ClassInstance> _M_instance;
            Class* _M_class = nullptr;
            String _M_name;
            bool _M_register_to_lua = false;

            ClassRegistrar(Class* _class, const String& name) : _M_class(_class), _M_name(name)
            {}

        public:
            ClassRegistrar& register_to_lua(bool flag = true)
            {
                if (flag && _M_instance.lua_state() == nullptr)
                {
                    _M_instance = Lua::Interpretter::lua_class_of<ClassInstance>(_M_name);

                    if constexpr (has_super_type_v<ClassInstance>)
                    {
                        auto base_classes = Class::class_parents<typename ClassInstance::Super>();
                        _M_instance.set(Lua::base_classes, base_classes);
                    }

                    if constexpr (std::is_base_of_v<Engine::Object, ClassInstance>)
                    {
                        _M_instance.set(Lua::meta_function::construct, lua_allocate<ClassInstance>);
                        _M_instance.set(Lua::meta_function::to_string, &ClassInstance::as_string);
                    }
                }

                _M_register_to_lua = flag;
                return *this;
            }

            template<typename Key, typename Value>
            ClassRegistrar& set(Key&& key, Value&& value)
            {
                if (_M_register_to_lua)
                {
                    _M_instance.set(std::forward<Key>(key), std::forward<Value>(value));
                }
                return *this;
            }

            template<typename Key, typename ResultType, typename... Args>
            ClassRegistrar& set(Key&& key, ResultType (ClassInstance::*method)(Args...))
            {
                if (_M_register_to_lua)
                {
                    _M_instance.set(std::forward<Key>(key), method);
                }
                return *this;
            }

            template<typename Key, typename ResultType, typename... Args>
            ClassRegistrar& set(Key&& key, ResultType (ClassInstance::*method)(Args...) const)
            {
                if (_M_register_to_lua)
                {
                    _M_instance.set(std::forward<Key>(key), method);
                }
                return *this;
            }

            template<typename Key, typename PropType>
            ClassRegistrar& set(Key&& key, PropType ClassInstance::*prop, AccessType access = AccessType::Public,
                                bool is_serializable = true)
            {
                if constexpr (std::is_constructible_v<String, Key>)
                {
                    String name = std::forward<Key>(key);
                    ClassField* field =
                            Object::new_instance_named<ClassField>(name, _M_class, prop, access, is_serializable);
                    _M_class->_M_fields.push_back(field);

                    if (_M_register_to_lua && access == AccessType::Public)
                    {
                        _M_instance.set(name, prop);
                    }
                }
                return *this;
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
        const Vector<ClassField*>& fields() const;
        static void on_class_register(void*);

        template<typename InstanceClass = void, typename... Args>
        static ClassRegistrar<InstanceClass> register_new_class(const String& class_name, Args... args)
        {
            Class* class_instance = find_class(class_name);
            if (class_instance == nullptr)
            {
                info_log("Class", "Start initialize class '%s'", class_name.c_str());
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


template<typename ClassInstance, typename... Args>
static void class_register_callback(const Engine::String& name, Args&&... args)
{
    Engine::Class::ClassRegistrar<ClassInstance> registrar =
        Engine::Class::register_new_class<ClassInstance>(name, args...);

    if constexpr (Engine::has_super_type_v<ClassInstance>)
    {
        if constexpr (&ClassInstance::on_class_register != &ClassInstance::Super::on_class_register)
        {
            ClassInstance::on_class_register(&registrar);
        }
        else
        {
            Engine::logger->warning("Class",
                                    "Class '%s' does not have on_class_register method! Force using default registrar!",
                                    name.c_str());
        }
    }
    else
    {
        ClassInstance::on_class_register(&registrar);
    }
}


#define register_class(class_name, ...)                                                                                \
    InitializeController(                                                                                              \
            Function<void()>([]() { class_register_callback<class_name>(#class_name __VA_OPT__(, ##__VA_ARGS__)); }))

#define registrar_of(class_name, address) reinterpret_cast<Class::ClassRegistrar<class_name>*>(address)
