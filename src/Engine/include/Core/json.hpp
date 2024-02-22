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
        ValueType m_type;
        Any m_value;

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

        Value& operator=(JsonBool);
        Value& operator=(JsonInt);
        Value& operator=(JsonFloat);
        Value& operator=(const Value&);
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
            return m_value.cast<T>();
        }

        template<typename T>
        const T get() const
        {
            return m_value.cast<const T>();
        }

        template<typename T>
        const T checked_get(const std::decay_t<T>& default_value = {}) const
        {
            try
            {
                return m_value.cast<const T>();
            }
            catch (...)
            {
                return default_value;
            }
        }

        template<typename T>
        const Value& copy_to_array(Vector<T>& out, ValueType filter) const
        {
            if (type() == ValueType::Array)
            {
                const JsonArray& array = checked_get<const JsonArray&>();
                for (auto& ell : array)
                {
                    if (ell.type() == filter)
                    {
                        out.push_back(ell.get<const T&>());
                    }
                }
            }

            return *this;
        }
    };


    class ENGINE_EXPORT Object : public Map<String, Value>
    {
        using Super = Map<String, Value>;

    public:
        using Super::unordered_map;

        Object& load(const Path& file, bool mix_objects = true);
        Object& parse(const String& json, bool mix_objects = true);
        Object& write(const Path& json);
        String dump(const int_t indent = -1, const char indent_char = ' ', const bool ensure_ascii = false) const;

        const Value& checked_get(const String& key) const;

        template<typename T>
        FORCE_INLINE T checked_get_value(const String& key, const T& default_value = {}) const
        {
            return checked_get(key).checked_get<T>(default_value);
        }
    };
}// namespace Engine::JSON
