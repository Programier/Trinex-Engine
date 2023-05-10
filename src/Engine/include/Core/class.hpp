#pragma once
#include <Core/engine_lua.hpp>
#include <Core/etl/deffered_method_invoker.hpp>
#include <Core/object.hpp>
#include <Core/predef.hpp>
#include <functional>

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
        size_t _M_instance_size         = 0;
        Vector<DefferedMethodInvokerBase*> _M_lua_invokers;
        void (*_M_post_init)() = nullptr;

        template<typename Instance>
        static Instance* lua_allocate()
        {
            Object* object = const_cast<Class*>(ClassMetaData<Instance>::find_class())->_M_allocate_object();
            return object->instance_cast<Instance>();
        }

    private:
        Class(const String& name = "");

        void update_parent_classes(const Class* parent);

        template<typename InstanceClass, typename BaseClass>
        static void post_init()
        {
            Class* current = const_cast<Class*>(ClassMetaData<InstanceClass>::find_class());
            if (current == nullptr || current->_M_post_init == nullptr)
            {
                return;
            }

            const Class* parent = const_cast<const Class*>(ClassMetaData<BaseClass>::find_class());

            if (parent != nullptr)
            {
                if (parent->_M_post_init != nullptr)
                {
                    parent->_M_post_init();
                }

                current->update_parent_classes(parent);
            }

            size_t namespaces = 0;
            auto lua_class    = LuaInterpretter::lua_class_of<InstanceClass>(current->name(), namespaces);

            for (auto invoker : current->_M_lua_invokers)
            {
                invoker->invoke(&lua_class);
                delete invoker;
            }

            auto _n = lua_class.endClass();

            while (namespaces-- > 0)
            {
                _n = _n.endNamespace();
            }

            current->_M_lua_invokers.clear();

            current->_M_post_init = nullptr;
        }

        template<typename Instance, typename... Args>
        Class& create_allocator(Args... args)
        {
            if constexpr (std::is_abstract_v<Instance>)
            {
                _M_allocate_object = []() -> Object* { return nullptr; };
            }
            else
            {
                _M_allocate_object = [&]() -> Object* {
                    return static_cast<Object*>(Object::new_instance<Instance>(args...));
                };
            }
            return *this;
        }

    public:
        template<typename Instance>
        class LuaRegistrar
        {
        private:
            using LuaBridgeClass = luabridge::Namespace::Class<Instance>;
            Class* _M_class;
            LuaRegistrar(Class* _class) : _M_class(_class)
            {
                register_static_method("create", lua_allocate<Instance>);
            }

        public:
            LuaRegistrar(const LuaRegistrar&) = default;

            FORCE_INLINE Class* operator&()
            {
                return _M_class;
            }

            template<typename Return, typename... Args>
            LuaRegistrar& register_method(const char* name, Return (Instance::*method)(Args... args))
            {
                LuaBridgeClass& (LuaBridgeClass::*invoked_method)(const char*, Return (Instance::*)(Args...)) =
                        &LuaBridgeClass::template addFunction<Return, Args...>;
                DefferedMethodInvokerBase* invoker = new DefferedMethodInvoker(invoked_method, name, method);
                _M_class->_M_lua_invokers.push_back(invoker);
                return *this;
            }

            template<typename Return, typename... Args>
            LuaRegistrar& register_method(const char* name, Return (Instance::*method)(Args... args) const)
            {
                LuaBridgeClass& (LuaBridgeClass::*invoked_method)(const char*, Return (Instance::*)(Args...) const) =
                        &LuaBridgeClass::template addFunction<Return, Args...>;
                DefferedMethodInvokerBase* invoker = new DefferedMethodInvoker(invoked_method, name, method);
                _M_class->_M_lua_invokers.push_back(invoker);
                return *this;
            }

            template<typename Return, typename... Args>
            LuaRegistrar& register_static_method(const char* name, Return (*method)(Args... args))
            {
                LuaBridgeClass& (LuaBridgeClass::*invoked_method)(const char*, Return (*)(Args...)) =
                        &LuaBridgeClass::template addStaticFunction<Return (*)(Args...)>;
                DefferedMethodInvokerBase* invoker = new DefferedMethodInvoker(invoked_method, name, method);
                _M_class->_M_lua_invokers.push_back(invoker);
                return *this;
            }

            friend class Class;
        };

        const Class* parent() const;
        bool contains_class(const Class* const instance) const;
        static Class* find_class(const String& name);
        static const ClassesMap& classes();
        Object* create() const;
        size_t instance_size() const;

        template<typename InstanceClass = void, typename BaseClass = void, typename... Args>
        static LuaRegistrar<InstanceClass> register_new_class(const String& class_name, Args... args)
        {
            Class* class_instance = find_class(class_name);
            if (class_instance == nullptr)
            {
                class_instance = Object::new_instance_without_package<Class>(class_name);
                class_instance->create_allocator<InstanceClass>(args...);
                class_instance->_M_instance_size = sizeof(InstanceClass);
                extern Vector<void (*)()>& initialize_list();
                class_instance->_M_post_init = Class::post_init<InstanceClass, BaseClass>;
                initialize_list().push_back(class_instance->_M_post_init);
            }

            LuaRegistrar<InstanceClass> registrar(class_instance);
            return registrar;
        }

        friend class Object;
    };
}// namespace Engine
