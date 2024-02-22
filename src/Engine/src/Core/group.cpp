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
            root_group->m_name = "Root Group";

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
        for (Group* child : m_childs)
        {
            if (child->m_name.equals(name, len))
                return child;
        }

        if (create)
        {
            Group* new_group     = new Group();
            new_group->m_name   = Name(name, len);
            new_group->m_parent = this;

            // Find index for insert
            Index index = 0;
            auto& name  = new_group->name().to_string();

            for (Index size = m_childs.size(); index < size && name > m_childs[index]->name().to_string(); index++)
            {
            }

            m_childs.insert(m_childs.begin() + index, new_group);
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
        if (m_parent == root())
            return nullptr;

        return m_parent;
    }

    const Vector<Group*>& Group::childs() const
    {
        return m_childs;
    }

    const Name& Group::name() const
    {
        return m_name;
    }

    const Vector<class Struct*>& Group::structs() const
    {
        return m_structs;
    }

    Group& Group::add_struct(class Struct* instance)
    {
        for (class Struct* element : m_structs)
        {
            if (element == instance)
                return *this;
        }

        // Find index for insert
        Index index = 0;
        auto& name  = instance->base_name().to_string();
        for (Index size = m_structs.size(); index < size && name > m_structs[index]->name().to_string(); index++)
        {
        }


        m_structs.insert(m_structs.begin() + index, instance);
        return *this;
    }

    Group& Group::remove_struct(class Struct* instance)
    {
        std::erase_if(m_structs, [instance](Struct* ell) { return ell == instance; });
        return *this;
    }

    Group::Group()
    {}

    Group::~Group()
    {
        while (!m_childs.empty())
        {
            delete m_childs.front();
        }

        if (m_parent)
        {
            for (Index i = 0, count = m_parent->m_childs.size(); i < count; i++)
            {
                if (m_parent->m_childs[i] == this)
                {
                    m_parent->m_childs.erase(m_parent->m_childs.begin() + i);
                    break;
                }
            }
            m_parent = nullptr;
        }
    }
}// namespace Engine
