#pragma once
#include <Core/engine_types.hpp>


namespace Engine::JSON
{
    enum class ValueType
    {
        Undefined = 0,
        Bool,
        Integer,
        Float,
        String,
        Array,
        Object
    };

    class Object;
    class Value;

    using JsonBool   = bool;
    using JsonInt    = int_t;
    using JsonFloat  = float;
    using JsonString = String;
    using JsonArray  = Vector<class Engine::JSON::Value>;
    using JsonObject = class Engine::JSON::Object;

    class ENGINE_EXPORT Value
    {
    private:
        ValueType _M_type;
        std::any _M_value;

    public:
        Value();
        Value(const Value&);
        Value(Value&&);
        Value(JsonBool);
        Value(JsonInt);
        Value(JsonFloat);
        Value(const JsonString&);
        Value(const JsonArray&);
        Value(const JsonObject&);
        Value(JsonString&&);
        Value(JsonArray&&);
        Value(JsonObject&&);

        Value& operator=(const Value&);
        Value& operator=(JsonBool);
        Value& operator=(JsonInt);
        Value& operator=(JsonFloat);
        Value& operator=(const JsonString&);
        Value& operator=(const JsonArray&);
        Value& operator=(const JsonObject&);

        Value& operator=(Value&&);
        Value& operator=(JsonString&&);
        Value& operator=(JsonArray&&);
        Value& operator=(JsonObject&&);

        ValueType type() const;

        template<typename T>
        T get()
        {
            return std::any_cast<T>(_M_value);
        }

        template<typename T>
        T get() const
        {
            return std::any_cast<T>(_M_value);
        }

        template<typename T>
        T checked_get(const T& default_value = {}) const
        {
            try
            {
                return std::any_cast<T>(_M_value);
            }
            catch (...)
            {
                return default_value;
            }
        }
    };


    class ENGINE_EXPORT Object : public Map<String, Value>
    {
        using Super = Map<String, Value>;

    public:
        using Super::unordered_map;

        Object& load(const Path& file);
        Object& parse(const String& json);
        Object& write(const Path& json);
        String dump(const int_t indent = -1, const char indent_char = ' ', const bool ensure_ascii = false) const;

        const Value& checked_get(const String& key) const;

        template<typename T>
        FORCE_INLINE T checked_get_value(const String& key, const T& default_value = {}) const
        {
            return checked_get(key).checked_get<T>();
        }
    };
}// namespace Engine::JSON
