#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT Name
    {

    public:
        struct Entry {
            String name;
            HashIndex hash;
        };

    private:
        Index _M_index;


        Name& init(const String& name);

    public:
        Name();
        Name(const Name&);
        Name(Name&&);
        Name& operator=(const Name&);
        Name& operator=(Name&&);

        Name(const String& name);
        Name(const char* name);

        Name& operator =(const String& name);
        Name& operator =(const char* name);


        bool is_valid() const;
        bool operator==(const String& name) const;
        bool operator==(const char* name) const;
        bool operator!=(const String& name) const;
        bool operator!=(const char* name) const;

        const String& to_string() const;
        const Name& to_string(String& out) const;

        operator const String&() const;

        FORCE_INLINE bool operator==(const Name& name) const
        {
            return name._M_index == _M_index;
        }

        FORCE_INLINE bool operator!=(const Name& name) const
        {
            return name._M_index != _M_index;
        }
    };
}// namespace Engine
