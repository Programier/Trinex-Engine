#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/localization.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <regex>
#include <sstream>

namespace Engine
{
	Localization* Localization::s_instance = nullptr;

	const String& Localization::localize(const StringView& line) const
	{
		uint64_t hash = memory_hash(line.data(), line.length());
		auto it       = m_translation_map.find(hash);

		if (it != m_translation_map.end())
			return it->second;


		for (int i = 0; i < 2; i++)
		{
			it = m_default_translation_map.find(hash);

			if (it != m_default_translation_map.end())
				return it->second;

			size_t separator_index = line.find_last_of('/');

			if (separator_index == StringView::npos)
			{
				m_default_translation_map[hash] = line;
			}
			else
			{
				m_default_translation_map[hash] = line.substr(separator_index + 1);
			}
		}

		error_log("Localization", "Failed to get localized string for line '%s'", line.data());
		return Name::none.to_string();
	}

	const String& Localization::language() const
	{
		return Settings::current_language;
	}

	Localization& Localization::language(const StringView& lang)
	{
		if (language() == lang)
			return *this;
		Settings::current_language = lang;
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

	static void load_localization(Map<uint64_t, String>& out, const Path& path)
	{
		trinex_try
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
						String p      = entry.relative(path);
						key           = p.substr(0, p.length() - Constants::translation_config_extension.length()) + "/" + key;
						uint64_t hash = memory_hash(key.c_str(), key.length());
						out[hash]     = value;
					}
				}
			}
		}
#if TRINEX_WITH_EXCEPTIONS
		trinex_catch(const std::exception& e)
		{
			error_log("Localization", "%s", e.what());
		}
#endif
	}

	Localization& Localization::reload(bool clear, bool with_default)
	{
		Path localization_dir = Project::localization_dir;

		if (with_default)
		{
			if (clear)
				m_default_translation_map.clear();

			Path path = localization_dir / Path(Settings::default_language);
			load_localization(m_default_translation_map, path);
		}

		if (clear)
			m_translation_map.clear();

		Path path = localization_dir / language();
		load_localization(m_translation_map, path);


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

	static InitializeController post_init([]() { Localization::create_instance()->reload(true, true); });

}// namespace Engine
