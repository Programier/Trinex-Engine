#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/localization.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <cstring>
#include <regex>
#include <sstream>

namespace Engine
{
    Localization* Localization::_M_instance = nullptr;

    const String& Localization::localize(const StringView& line) const
    {
        HashIndex hash = memory_hash_fast(line.data(), line.length());
        auto it        = _M_translation_map.find(hash);

        if (it != _M_translation_map.end())
            return it->second;


        for (int i = 0; i < 2; i++)
        {
            it = _M_default_translation_map.find(hash);

            if (it != _M_default_translation_map.end())
                return it->second;

            size_t separator_index = line.find_last_of('/');

            if (separator_index == StringView::npos)
            {
                _M_default_translation_map[hash] = line;
            }
            else
            {
                _M_default_translation_map[hash] = line.substr(separator_index + 1);
            }
        }

        throw EngineException("Failed to get localized string");
    }

    const String& Localization::language() const
    {
        return engine_config.current_language;
    }

    Localization& Localization::language(const StringView& lang)
    {
        if (engine_config.current_language == lang)
            return *this;

        engine_config.current_language = lang;
        reload();

        on_language_changed.trigger();
        return *this;
    }

    static bool parse_string(const std::string& formatString, String& key, String& value)
    {
        static std::regex regexPattern(R"(([^=]+) => (.+))");
        std::smatch match;

        if (std::regex_match(formatString, match, regexPattern))
        {
            key   = match[1].str();
            value = match[2].str();
            return true;
        }
        else
        {
            return false;
        }
    }

    static void load_localization(Map<HashIndex, String>& out, const Path& path)
    {
        try
        {
            std::stringstream stream;

            for (auto& entry : VFS::RecursiveDirectoryIterator(path))
            {
                if (entry.extension() != Constants::translation_config_extension)
                    continue;
                info_log("Localization", "Loading localization file '%s'", entry.c_str());

                FileReader reader(entry);
                if (!reader.is_open())
                    continue;

                std::stringstream stream;
                stream << reader.read_string();

                String line;
                while (std::getline(stream, line))
                {
                    String key, value;
                    if (parse_string(line, key, value))
                    {
                        String p = entry.relative(path);
                        key = p.substr(0, p.length() - Constants::translation_config_extension.length()) + "/" + key;
                        HashIndex hash = memory_hash_fast(key.c_str(), key.length());
                        out[hash]      = value;
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            error_log("Localization", "%s", e.what());
        }
    }

    Localization& Localization::reload(bool clear, bool with_default)
    {
        if (with_default)
        {
            if (clear)
                _M_default_translation_map.clear();

            Path path = engine_config.localization_dir / engine_config.default_language;
            load_localization(_M_default_translation_map, path);
        }

        if (clear)
            _M_translation_map.clear();

        Path path = engine_config.localization_dir / engine_config.current_language;
        load_localization(_M_translation_map, path);


        return *this;
    }

    ENGINE_EXPORT const String& Object::language()
    {
        return Localization::instance()->language();
    }

    ENGINE_EXPORT void Object::language(const StringView& new_language)
    {
        Localization::instance()->language(new_language);
    }

    ENGINE_EXPORT const String& Object::localize(const StringView& line)
    {
        return Localization::instance()->localize(line);
    }

    ENGINE_EXPORT const char* operator""_localized(const char* line, size_t len)
    {
        return Localization::instance()->localize(StringView(line, len)).c_str();
    }

    static PostInitializeController post_init([]() { Localization::create_instance()->reload(true, true); });

}// namespace Engine
