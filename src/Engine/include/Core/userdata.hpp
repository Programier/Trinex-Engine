#pragma once

#include <Core/name.hpp>


namespace Engine
{
    class ENGINE_EXPORT UserData final
    {
    public:
        class ENGINE_EXPORT Entry
        {
        public:
            virtual ~Entry();
        };

    private:
        Map<Name, UserData::Entry*, Name::HashFunction> m_userdata;

        static void on_create_userdata_fail(const Name& name);

    public:
        UserData::Entry* find_userdata(const Name& name) const;

        template<typename Type>
        inline typename std::enable_if<std::is_base_of_v<UserData::Entry, Type>, Type*>::type
        find_typed_userdata(const Name& name) const
        {
            return reinterpret_cast<Type*>(find_userdata(name));
        }

        template<typename Type, typename... Args>
        inline typename std::enable_if<std::is_base_of_v<UserData::Entry, Type>, Type*>::type create_userdata(const Name& name,
                                                                                                              Args&&... args)
        {
            if (find_userdata(name))
            {
                on_create_userdata_fail(name);
                return nullptr;
            }

            Type* type       = new Type(std::forward<Args>(args)...);
            m_userdata[name] = type;
            return type;
        }

        template<typename Type, typename... Args>
        inline typename std::enable_if<std::is_base_of_v<UserData::Entry, Type>, Type*>::type find_or_create_userdata(const Name& name,
                                                                                                             Args&&... args)
        {
            if (Type* userdata = find_typed_userdata<Type>(name))
            {
                return userdata;
            }

            Type* type       = new Type(std::forward<Args>(args)...);
            m_userdata[name] = type;
            return type;
        }

        ~UserData();
    };

}// namespace Engine
