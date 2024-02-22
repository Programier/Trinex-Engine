#include <Core/file_manager.hpp>
#include <Core/json.hpp>
#include <Core/logger.hpp>
#include <iostream>
#include <json.hpp>

namespace Engine::JSON
{
    Value::Value() : m_type(ValueType::Undefined), m_value({})
    {}

    Value::Value(const Value& obj) : m_type(obj.m_type), m_value(obj.m_value)
    {}

    Value::Value(Value&& obj) : m_type(obj.m_type), m_value(std::move(obj.m_value))
    {
        obj.m_type = ValueType::Undefined;
    }

    Value::Value(JsonBool value) : m_type(ValueType::Bool), m_value(value)
    {}

    Value::Value(JsonInt value) : m_type(ValueType::Integer), m_value(value)
    {}

    Value::Value(JsonFloat value) : m_type(ValueType::Float), m_value(value)
    {}

    Value::Value(const JsonString& str) : m_type(ValueType::String), m_value(str)
    {}

    Value::Value(const JsonArray& array) : m_type(ValueType::Array), m_value(array)
    {}

    Value::Value(const JsonObject& obj) : m_type(ValueType::Object), m_value(obj)
    {}

    Value::Value(JsonString&& str) : m_type(ValueType::String), m_value(std::move(str))
    {}

    Value::Value(JsonArray&& array) : m_type(ValueType::Array), m_value(std::move(array))
    {}

    Value::Value(JsonObject&& object) : m_type(ValueType::Object), m_value(std::move(object))
    {}

    Value& Value::operator=(const Value& value)
    {
        if (this != &value)
        {
            m_type  = value.m_type;
            m_value = value.m_value;
        }
        return *this;
    }

    Value& Value::operator=(JsonBool value)
    {
        m_type  = ValueType::Bool;
        m_value = value;
        return *this;
    }

    Value& Value::operator=(JsonInt value)
    {
        m_type  = ValueType::Integer;
        m_value = value;
        return *this;
    }

    Value& Value::operator=(JsonFloat value)
    {
        m_type  = ValueType::Float;
        m_value = value;
        return *this;
    }

    Value& Value::operator=(const JsonString& str)
    {
        m_type  = ValueType::String;
        m_value = str;
        return *this;
    }

    Value& Value::operator=(const JsonArray& array)
    {
        m_type  = ValueType::Array;
        m_value = array;
        return *this;
    }

    Value& Value::operator=(const JsonObject& object)
    {
        m_type  = ValueType::Object;
        m_value = object;
        return *this;
    }


    Value& Value::operator=(Value&& value)
    {
        if (this != &value)
        {
            m_type       = value.m_type;
            m_value      = std::move(value.m_value);
            value.m_type = ValueType::Undefined;
        }

        return *this;
    }

    Value& Value::operator=(JsonString&& str)
    {
        m_type  = ValueType::String;
        m_value = std::move(str);
        return *this;
    }

    Value& Value::operator=(JsonArray&& array)
    {
        m_type  = ValueType::Array;
        m_value = std::move(array);
        return *this;
    }

    Value& Value::operator=(JsonObject&& object)
    {
        m_type  = ValueType::Object;
        m_value = std::move(object);
        return *this;
    }

    ValueType Value::type() const
    {
        return m_type;
    }

    Object& Object::load(const Path& file, bool mix_objects)
    {
        FileReader reader(file);
        String json(reader.size(), '\0');
        reader.read(reinterpret_cast<byte*>(json.data()), json.size());
        return parse(json, mix_objects);
    }

    static void copy_value_to(Value& value, const nlohmann::json& json_value, bool mix_objects);

    template<typename T>
    static void copy_object(Object& object, const T& json, bool mix_objects)
    {
        auto it  = json.begin();
        auto end = json.end();

        while (it != end)
        {
            String key;
            const nlohmann::json* value = nullptr;

            if constexpr (std::is_same_v<T, nlohmann::json>)
            {
                key   = it.key();
                value = &it.value();
            }
            else
            {
                key   = it->first;
                value = &it->second;
            }

            copy_value_to(object[key], *value, mix_objects);
            ++it;
        }
    }

    static void mix_objects_internal(JSON::Value& to, const JSON::Object& from)
    {
        if (to.type() != ValueType::Object)
        {
            to = from;
        }
        else
        {
            JsonObject& map = to.get<JsonObject&>();
            for (auto& [name, value] : from)
            {
                if (value.type() != ValueType::Object)
                {
                    map[name] = value;
                }
                else
                {
                    mix_objects_internal(map[name], value.get<const JSON::Object&>());
                }
            }
        }
    }

    static void copy_value_to(Value& value, const nlohmann::json::value_type& json_value, bool mix_objects)
    {
        if (json_value.is_boolean())
        {
            value = json_value.get<bool>();
        }
        else if (json_value.is_number_float())
        {
            value = json_value.get<JsonFloat>();
        }
        else if (json_value.is_number())
        {
            value = json_value.get<JsonInt>();
        }
        else if (json_value.is_string())
        {
            value = json_value.get<JsonString>();
        }
        else if (json_value.is_array())
        {
            const nlohmann::json::array_t& json_array = json_value.get<nlohmann::json::array_t>();

            JsonArray array(json_array.size());
            size_t index = 0;
            for (auto& ell : json_array)
            {
                copy_value_to(array[index++], ell, mix_objects);
            }
            value = array;
        }
        else if (json_value.is_object())
        {
            Object sub_object;
            copy_object(sub_object, json_value.get<nlohmann::json::object_t>(), mix_objects);
            if (mix_objects)
            {
                mix_objects_internal(value, sub_object);
            }
            else
            {
                value = sub_object;
            }
        }
    }

    Object& Object::parse(const String& json, bool mix_objects)
    {
        try
        {
            nlohmann::json object = nlohmann::json::parse(json, nullptr, true, true);
            copy_object(*this, object, mix_objects);
        }
        catch (const std::exception& e)
        {
            error_log("JSON", "%s", e.what());
        }

        return *this;
    }

    Object& Object::write(const Path& json)
    {
        FileWriter writer(json);
        String d = dump();
        writer.write(reinterpret_cast<const byte*>(d.data()), d.size());
        return *this;
    }


    static void copy_value_to(nlohmann::json::value_type& out, const Value& value);

    template<typename T>
    static void recursive_write_json(const Object& object, T& out)
    {
        for (auto& pair : object)
        {
            copy_value_to(out[pair.first], pair.second);
        }
    }

    static void copy_value_to(nlohmann::json::value_type& out, const Value& value)
    {
        auto type = value.type();

        if (type == ValueType::Bool)
        {
            out = value.get<JsonBool>();
        }
        else if (type == ValueType::Integer)
        {
            out = value.get<JsonInt>();
        }
        else if (type == ValueType::Float)
        {
            out = value.get<JsonFloat>();
        }
        else if (type == ValueType::String)
        {
            out = value.get<const JsonString&>();
        }
        else if (type == ValueType::Array)
        {
            const JsonArray& array = value.get<const JsonArray&>();
            nlohmann::json::array_t sub_array(array.size());

            size_t index = 0;
            for (auto& ell : array)
            {
                copy_value_to(sub_array[index], ell);
            }

            out = sub_array;
        }
        else if (type == ValueType::Object)
        {
            nlohmann::json::object_t sub_object;
            recursive_write_json(value.get<const JsonObject&>(), sub_object);
            out = sub_object;
        }
    }

    String Object::dump(const int_t indent, const char indent_char, const bool ensure_ascii) const
    {
        nlohmann::json out;
        recursive_write_json(*this, out);
        return out.dump(indent, indent_char, ensure_ascii);
    }

    const Value& Object::checked_get(const String& key) const
    {
        static const Value default_value;
        auto it = find(key);
        if (it == end())
        {
            return default_value;
        }
        return it->second;
    }
}// namespace Engine::JSON
