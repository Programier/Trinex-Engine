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

        struct HashFunction {
            FORCE_INLINE HashIndex operator()(const Name& name) const
            {
                return name.hash();
            }
        };

        struct Less {
            FORCE_INLINE bool operator()(const Name& x, const Name& y) const
            {
                return std::less<String>()(x.to_string(), y.to_string());
            }
        };

        static ENGINE_EXPORT Name none;

    private:
        Index _M_index;
        Name& init(const char* name, size_t len);

    public:
        Name();
        Name(const Name&);
        Name(Name&&);
        Name& operator=(const Name&);
        Name& operator=(Name&&);

        Name(const String& name);
        Name(const char* name);
        Name(const char* name, size_t size);

        Name& operator=(const String& name);
        Name& operator=(const char* name);

        static Name find_name(const String& name);
        static Name find_name(const char* name, size_t size = 0);


        bool is_valid() const;
        HashIndex hash() const;
        bool operator==(const String& name) const;
        bool operator==(const char* name) const;
        bool operator!=(const String& name) const;
        bool operator!=(const char* name) const;

        const String& to_string() const;
        const char* c_str() const;
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

        FORCE_INLINE bool operator<(const Name& name) const
        {
            return _M_index < name._M_index;
        }

        FORCE_INLINE bool operator<=(const Name& name) const
        {
            return _M_index <= name._M_index;
        }

        FORCE_INLINE bool operator>(const Name& name) const
        {
            return _M_index > name._M_index;
        }

        FORCE_INLINE bool operator>=(const Name& name) const
        {
            return _M_index >= name._M_index;
        }
    };

    ENGINE_EXPORT bool operator&(class Archive&, Name& name);
}// namespace Engine
