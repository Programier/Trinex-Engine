#include <Core/logger.hpp>
#include <Core/userdata.hpp>

namespace Engine
{
    UserData::Entry::~Entry()
    {}

    void UserData::on_create_userdata_fail(const Name& name)
    {
        error_log("Object", "Failed to create userdata with id '%s': Userdata already exist!", name.c_str());
    }

    UserData::Entry* UserData::find_userdata(const Name& name) const
    {
        auto it = m_userdata.find(name);
        if (it != m_userdata.end())
            return it->second;
        return nullptr;
    }

    UserData::~UserData()
    {
        for (auto& [name, userdata] : m_userdata)
        {
            if (userdata)
            {
                delete userdata;
            }
        }
    }
}// namespace Engine
