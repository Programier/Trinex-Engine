#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace Reflector
{
	class Archive;
	class Module;

	class Reflector final
	{
	private:
		struct HeaderInfo {
			std::filesystem::path path;
			std::uint64_t timestamp = 0;
			std::uint64_t size      = 0;
			bool external           = false;
		};

		struct ModuleInfo {
			Module* module        = nullptr;
			std::size_t timestamp = 0;
			std::size_t size      = 0;
		};

		struct DirectoryInfo {
			std::filesystem::path path;
			bool external = false;
		};

		std::vector<HeaderInfo> m_headers;
		std::vector<ModuleInfo> m_modules;
		std::vector<DirectoryInfo> m_directories;
		std::filesystem::path m_output;
		std::filesystem::path m_root;


	private:
		Reflector();
		~Reflector();

		void help();
		int on_help(std::span<std::string_view> args);
		int on_output_dir(std::span<std::string_view> args);
		int on_headers_dirs(std::span<std::string_view> args, bool external);
		int on_scan_dirs(std::span<std::string_view> args);
		int on_include_dirs(std::span<std::string_view> args);
		int on_root(std::span<std::string_view> args);

		bool init(std::span<std::string_view> args);
		void collect_headers();
		void load_modules();
		void save_modules();
		void process();

		bool is_cache_valid(std::string_view path, std::size_t timestamp, std::size_t size);

	public:
		static Reflector* instance();

		int execute(std::span<std::string_view> args);
	};
}// namespace Reflector
