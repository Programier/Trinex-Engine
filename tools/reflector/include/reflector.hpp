#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include <unordered_set>
#include <vector>

namespace Reflector
{
	class Archive;
	class Module;

	class Reflector final
	{
	private:
		enum class CacheState
		{
			Valid    = 0,
			Outdated = 1,
			Removed  = 2,
		};

		struct DirectoryInfo {
			std::filesystem::path path;
			bool external = false;
		};

		struct HeaderInfo {
			std::filesystem::path path;
			const DirectoryInfo* directory = nullptr;
			std::uint64_t timestamp        = 0;
			std::uint64_t size             = 0;
		};

		struct ModuleInfo {
			Module* module        = nullptr;
			std::size_t timestamp = 0;
			std::size_t size      = 0;
		};


		struct HeaderHash {
			using is_transparent = void;

			std::size_t operator()(const HeaderInfo& info) const noexcept { return (*this)(info.path); }
			std::size_t operator()(const std::filesystem::path& path) const noexcept
			{
				return std::hash<std::filesystem::path>{}(path);
			}
		};

		struct HeaderEqual {
			using is_transparent = void;

			bool operator()(const HeaderInfo& a, const HeaderInfo& b) const noexcept { return a.path == b.path; }
			bool operator()(const HeaderInfo& a, const std::filesystem::path& b) const noexcept { return a.path == b; }
			bool operator()(const std::filesystem::path& a, const HeaderInfo& b) const noexcept { return a == b.path; }
		};


	private:
		std::unordered_set<HeaderInfo, HeaderHash, HeaderEqual> m_headers;
		std::vector<ModuleInfo> m_modules;
		std::vector<DirectoryInfo> m_directories;
		std::filesystem::path m_output;
		std::filesystem::path m_include;
		std::filesystem::path m_sources;
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
		bool collect_headers();
		bool load_modules();
		void save_modules();
		void process();

		CacheState cache_state(const std::filesystem::path& path, std::uint64_t timestamp, std::uint64_t size);

	public:
		static Reflector* instance();

		int execute(std::span<std::string_view> args);
	};
}// namespace Reflector
