#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <LuaBridge/Vector.h>


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


    static ClassMetaData<Engine::Class> class_instance = nullptr;

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


    static void concat_names(Vector<String>& to, const Vector<String>& from)
    {
        for (auto& line : from)
        {
            if (!line.starts_with("create :"))
            {
                to.push_back(line);
            }
        }
    }


    void Class::update_parent_classes(const Class* parent)
    {
        _M_parent = parent;
        _M_parents.insert(parent->_M_parents.begin(), parent->_M_parents.end());
        concat_names(_M_methods, parent->_M_methods);
        concat_names(_M_static_methods, parent->_M_static_methods);
        concat_names(_M_properties, parent->_M_properties);
        concat_names(_M_static_properties, parent->_M_static_properties);
    }

    void Class::insert_new_name(const char* name, const String& type_name, Vector<String>& to)
    {
        to.push_back(Strings::format("{} : {}", name, type_name));
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

    const Vector<String>& Class::properties() const
    {
        return _M_properties;
    }
    const Vector<String>& Class::static_properties() const
    {
        return _M_static_properties;
    }
    const Vector<String>& Class::methods() const
    {
        return _M_methods;
    }
    const Vector<String>& Class::static_methods() const
    {
        return _M_static_methods;
    }


    static void on_init()
    {
        LuaInterpretter::global_namespace()
                .beginNamespace("Engine")
                .beginClass<Class>("Class")
                .addFunction("parent", &Class::parent)
                .addFunction("contains_class", &Class::contains_class)
                .addStaticFunction("classes", Class::classes)
                .addFunction("create", &Class::create)
                .addFunction("instance_size", &Class::instance_size)
                .addStaticFunction("find_class", Class::find_class)
                .addFunction("properties", &Class::properties)
                .addFunction("static_properties", &Class::static_properties)
                .addFunction("methods", &Class::methods)
                .addFunction("static_methods", &Class::static_methods)
                .endClass()
                .endNamespace();
    }

    namespace
    {
        static InitializeController a(on_init);
    }
}// namespace Engine
