#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/group.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>
#include <cstring>

namespace Engine
{
    Group* Group::root()
    {
        static Group* root_group = nullptr;

        if (root_group == nullptr)
        {
            root_group          = new Group();
            root_group->_M_name = "Root Group";

            PostDestroyController([]() {
                delete root_group;
                root_group = nullptr;
            });
        }
        return root_group;
    }


    Group* Group::find(const String& name, bool create)
    {
        return find(name.c_str(), name.length(), create);
    }

    Group* Group::find(const char* name, bool create)
    {
        return find(name, std::strlen(name), create);
    }

    Group* Group::find_subgroup(const char* name, size_t len, bool create)
    {
        for (Group* child : _M_childs)
        {
            if (child->_M_name.equals(name, len))
                return child;
        }

        if (create)
        {
            Group* new_group     = new Group();
            new_group->_M_name   = Name(name, len);
            new_group->_M_parent = this;

            // Find index for insert
            Index index = 0;
            auto& name  = new_group->name().to_string();

            for (Index size = _M_childs.size(); index < size && name > _M_childs[index]->name().to_string(); index++)
            {
            }

            _M_childs.insert(_M_childs.begin() + index, new_group);
            return new_group;
        }

        return nullptr;
    }

    Group* Group::find(const char* name, size_t len, bool create)
    {
        const char* name_end      = name + len;
        const String& separator   = Constants::name_separator;
        const char* separator_pos = nullptr;
        Group* current            = root();

        while (current && (separator_pos = Strings::strnstr(name, name_end - name, separator.c_str(), separator.length())))
        {
            current = current->find_subgroup(name, separator_pos - name, create);
            name    = separator_pos + separator.length();
        }

        return current ? current->find_subgroup(name, name_end - name, create) : nullptr;
    }

    Group* Group::parent() const
    {
        if (_M_parent == root())
            return nullptr;

        return _M_parent;
    }

    const Vector<Group*>& Group::childs() const
    {
        return _M_childs;
    }

    const Name& Group::name() const
    {
        return _M_name;
    }

    const Vector<class Struct*>& Group::structs() const
    {
        return _M_structs;
    }

    Group& Group::add_struct(class Struct* instance)
    {
        for (class Struct* element : _M_structs)
        {
            if (element == instance)
                return *this;
        }

        // Find index for insert
        Index index = 0;
        auto& name  = instance->base_name().to_string();
        for (Index size = _M_structs.size(); index < size && name > _M_structs[index]->name().to_string(); index++)
        {
        }


        _M_structs.insert(_M_structs.begin() + index, instance);
        return *this;
    }

    Group& Group::remove_struct(class Struct* instance)
    {
        std::erase_if(_M_structs, [instance](Struct* ell) { return ell == instance; });
        return *this;
    }

    Group::Group()
    {}

    Group::~Group()
    {
        while (!_M_childs.empty())
        {
            delete _M_childs.front();
        }

        if (_M_parent)
        {
            for (Index i = 0, count = _M_parent->_M_childs.size(); i < count; i++)
            {
                if (_M_parent->_M_childs[i] == this)
                {
                    _M_parent->_M_childs.erase(_M_parent->_M_childs.begin() + i);
                    break;
                }
            }
            _M_parent = nullptr;
        }
    }
}// namespace Engine
