#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <string_view>

namespace Engine
{
    static Map<std::type_index, const class Class*>& indexed_classes_map()
    {
        static Map<std::type_index, const class Class*> map = {};
        return map;
    }

    ENGINE_EXPORT const class Class* ClassMetaDataHelper::find_class(const std::type_index& index)
    {
        static auto& map = indexed_classes_map();

        auto it = map.find(index);
        if (it == map.end())
            return nullptr;

        return it->second;
    }

    ClassMetaDataHelper::ClassMetaDataHelper(const std::type_index& index, const class Class* instance)
    {
        indexed_classes_map()[index] = instance;
    }

    static Class::ClassesMap& classes_map()
    {
        static Class::ClassesMap classes;
        return classes;
    }

    const Class* Class::parent() const
    {
        return _M_parent;
    }

    Class::Class(const String& _name)
    {
        name(_name);
        classes_map().insert({_name, this});
        _M_parents = {this};
    }

    bool Class::contains_class(const Class* const instance) const
    {
        return _M_parents.contains(instance);
    }

    Class* Class::find_class(const String& name)
    {
        ClassesMap& map = classes_map();
        auto it         = map.find(name);
        if (it == map.end())
            return nullptr;
        return it->second;
    }

    const Class::ClassesMap& Class::classes()
    {
        return classes_map();
    }

    void Class::update_parent_classes(const Class* parent)
    {
        if (parent)
        {
            if (parent->_M_resolve_inherit)
            {
                const_cast<Class*>(parent)->_M_resolve_inherit();
            }

            info_log("Class", "Setting class '%s' as the parent of class '%s'", parent->full_name().c_str(),
                     full_name().c_str());
            _M_parent = parent;
            _M_parents.insert(parent->_M_parents.begin(), parent->_M_parents.end());
        }
    }

    Object* Class::create() const
    {
        if (_M_allocate_object)
            return _M_allocate_object();
        return nullptr;
    }

    Object* Class::create_without_package() const
    {
        if (_M_allocate_without_package)
            return _M_allocate_without_package();
        return nullptr;
    }

    size_t Class::instance_size() const
    {
        return _M_instance_size;
    }

    Lua::object Class::to_lua_object(Object* object) const
    {
        if (object->class_instance() == this)
        {
            return _M_to_lua_object(object);
        }
        auto _class = object->class_instance();

        if (_class)
        {
            return _class->to_lua_object(object);
        }

        return {};
    }


    static void on_init()
    {
        Lua::Interpretter::namespace_of("Engine")                             //
                .new_usertype<Class>("Class",                                 //
                                     "parent", &Class::parent,                //
                                     "contains_class", &Class::contains_class,//
                                     "classes", Class::classes,               //
                                     "create", &Class::create,                //
                                     "instance_size", &Class::instance_size,  //
                                     "find_class", Class::find_class);
    }

    namespace
    {
        static InitializeController a(on_init);
    }
}// namespace Engine
