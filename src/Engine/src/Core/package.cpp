#include <Core/package.hpp>


namespace Engine
{
    declare_instance_info_cpp(Package);
    constructor_cpp(Package)
    {}

    constructor_cpp(Package, const String& _name)
    {
        name(_name);
    }

    Package& Package::add_object(Object* object)
    {
        if (!object)
            return *this;

        if (object->_M_package)
            _M_package->remove_object(object);

        object->_M_package = this;
        _M_objects.insert_or_assign(object->name(), object);
        return *this;
    }

    Package& Package::remove_object(Object* object)
    {
        if (!object || object->_M_package != this)
            return *this;

        object->_M_package = nullptr;
        _M_objects.erase(object->name());
        return *this;
    }

    Object* Package::get_object_by_name(const String& name) const
    {
        auto it = _M_objects.find(name);
        if (it == _M_objects.end())
            return nullptr;
        return (*it).second;
    }

    Object* Package::find_object_in_package(const String& object_name, bool recursive) const
    {
        if (!recursive)
            return get_object_by_name(object_name);

        std::size_t prev_pos = 0;
        std::size_t pos = 0;
        const Package* current_package = this;

        while (current_package && (pos = object_name.find_first_of(L"::", prev_pos)) != String::npos)
        {
            auto sub_name = object_name.substr(prev_pos, pos - prev_pos);
            prev_pos = pos + 2;
            Object* node = current_package->get_object_by_name(sub_name);
            current_package = node->instance_cast<Package>();
        }

        return current_package ? current_package->get_object_by_name(
                                         object_name.substr(prev_pos, object_name.length() - prev_pos))
                               : nullptr;
    }

    const Package::ObjectMap& Package::objects() const
    {
        return _M_objects;
    }

}// namespace Engine
