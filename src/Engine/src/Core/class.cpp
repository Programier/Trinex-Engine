#include <Core/class.hpp>
#include <Core/logger.hpp>


namespace Engine
{
    template<>
    const class Engine::Class* const ClassMetaData<Engine::Class>::class_instance = nullptr;

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
        _M_parent = parent;
        _M_parents.insert(parent->_M_parents.begin(), parent->_M_parents.end());
    }

    Object* Class::create() const
    {
        if (_M_allocate_object)
            return _M_allocate_object();
        return nullptr;
    }

    size_t Class::instance_size() const
    {
        return _M_instance_size;
    }
}// namespace Engine
