#include <Core/config.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/string_functions.hpp>
#include <algorithm>


namespace Engine
{
#define MAKE_MAP(t, v)                                                                                                 \
    {                                                                                                                  \
#v, offsetof(t, v)                                                                                             \
    }

    static const Map<String, ArrayOffset> string_map = {
            MAKE_MAP(ConfigStringValue, resources_dir),
            MAKE_MAP(ConfigStringValue, api),
            MAKE_MAP(ConfigStringValue, base_commandlet),
            MAKE_MAP(ConfigStringValue, lua_scripts_dir),
    };

    static const Map<String, ArrayOffset> boolean_map = {
            MAKE_MAP(ConfigBooleanValue, delete_resources_after_load),
    };

    static const Map<String, ArrayOffset> integer_map = {
            MAKE_MAP(ConfigIntegerValue, lz4_compression_level),
            MAKE_MAP(ConfigIntegerValue, max_gc_collected_objects),
    };


    static String& param_parser(String& str)
    {
        static auto lstrip_callback = [](char ch) -> bool { return !std::isalpha(ch); };
        static auto rstrip_callback = [](char ch) -> bool { return !std::isalpha(ch) && !std::isdigit(ch); };

        str = Strings::lstrip(str, lstrip_callback);
        str = Strings::rstrip(str, rstrip_callback);

        for (String::size_type i = 1; i < str.size(); i++)
        {
            if (std::isupper(str[i]) && !std::isupper(str[i - 1]))// Is new word
            {
                str.insert(i++, 1, '_');
            }
        }

        str = Strings::replace_all(str, " ", "_");

        while (str.find("__") != String::npos) str = Strings::replace_all(str, "__", "_");

        std::transform(str.begin(), str.end(), str.begin(), [](char c) { return std::tolower(c); });
        return str;
    }


    template<typename Callback>
    static void read_file(TextFileReader* reader_ptr, Callback callback)
    {
        TextFileReader& reader = *reader_ptr;
        reader.stream().clear();
        reader.position(0);

        String param_name;
        while (reader.read_line(param_name, '='))
        {
            callback(param_name, reader_ptr);
        }
    }

    ConfigStringValue& ConfigStringValue::init(TextFileReader* reader_ptr)
    {
        auto callback = [this](String& param_name, TextFileReader* reader) -> void {
            auto param_offset = string_map.find(param_parser(param_name));
            if (param_offset != string_map.end())
            {
                String* param = reinterpret_cast<String*>(reinterpret_cast<byte*>(this) + param_offset->second);
                reader->read_line(*param);
                *param = Strings::strip(*param);
            }
            else
                reader->read_line();
        };
        read_file(reader_ptr, callback);

        resources_dir = FileManager::make_dirname(resources_dir);
        return *this;
    }

    template<typename Type, typename Instance>
    Instance& typed_init(TextFileReader* reader_ptr, const Map<String, ArrayOffset>& map, Instance* instance)
    {
        auto callback = [&instance, &map](String& param_name, TextFileReader* reader) -> void {
            auto param_offset = map.find(param_parser(param_name));
            if (param_offset != map.end())
            {
                Type* param = reinterpret_cast<Type*>(reinterpret_cast<byte*>(instance) + param_offset->second);
                (*reader) >> (*param);
            }

            reader->read_line();
        };

        read_file(reader_ptr, callback);
        return *instance;
    }

    ConfigBooleanValue& ConfigBooleanValue::init(TextFileReader* reader_ptr)
    {
        return typed_init<bool>(reader_ptr, boolean_map, this);
    }

    ConfigIntegerValue& ConfigIntegerValue::init(TextFileReader* reader_ptr)
    {
        return typed_init<int_t>(reader_ptr, integer_map, this);
    }

    EngineConfig& EngineConfig::init(const String& filename)
    {

        TextFileReader* reader_ptr = FileManager::root_file_manager()->create_text_file_reader(filename);
        if (reader_ptr == nullptr)
        {
            String error = Strings::format("EngineConfig: Config {} not found!",
                                           (FileManager::root_file_manager()->work_dir() + filename).c_str());
            throw CriticalError(error);
        }

        ConfigStringValue::init(reader_ptr);
        ConfigBooleanValue::init(reader_ptr);
        ConfigIntegerValue::init(reader_ptr);

        if (max_gc_collected_objects < 100)
        {
            max_gc_collected_objects = 2000;
        }

        delete reader_ptr;
        return *this;
    }

    ENGINE_EXPORT EngineConfig engine_config;
}// namespace Engine
