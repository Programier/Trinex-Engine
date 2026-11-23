#pragma once
#include <cstdint>
#include <filesystem>
#include <model.hpp>
#include <span>
#include <vector>

namespace Reflector
{
	class Archive;
	class Module;

	struct CompileRequest
	{
		std::filesystem::path source = {};
		std::filesystem::path header = {};
		std::uint64_t timestamp      = 0;
		std::uint64_t size           = 0;

		bool external : 1  = false;
		bool completed : 1 = false;
		bool outdated : 1  = false;
	};

	struct FileInfo
	{
		std::filesystem::path path;
		bool external = false;
	};

	struct ModuleInfo
	{
		Module* module          = nullptr;
		CompileRequest* request = nullptr;
	};

	struct CacheEntry
	{
		CompileRequest* request = nullptr;
	};

	class Reflector final
	{
	private:
		std::vector<CompileRequest> m_requests;
		std::vector<ModuleInfo> m_modules;
		std::vector<FileInfo> m_files;
		TypeIndex m_type_index;

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
		int on_module_files(std::span<std::string_view> args);
		int on_files(std::span<std::string_view> args, bool external);
		int on_external(std::span<std::string_view> args);
		int on_root(std::span<std::string_view> args);

		bool init(std::span<std::string_view> args);
		bool collect_compile_requests();
		bool load_modules();
		void save_modules();
		bool process();
		bool link_modules();
		bool generate_modules();

		CompileRequest* fetch_compile_request(const std::filesystem::path& path, std::uint64_t timestamp, std::uint64_t size);

	public:
		static Reflector* instance();

		int execute(std::span<std::string_view> args);
	};
} // namespace Reflector
