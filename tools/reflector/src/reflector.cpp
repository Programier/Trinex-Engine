#include <archive.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <generator.hpp>
#include <iostream>
#include <model.hpp>
#include <parser.hpp>
#include <printer.hpp>
#include <reflector.hpp>

namespace fs = std::filesystem;

namespace Reflector
{
	class Magic
	{
	private:
		static constexpr inline std::size_t size = 4;
		char m_bytes[size]                       = {};

	public:
		constexpr Magic() = default;
		constexpr Magic(const char (&str)[size + 1])
		{
			for (std::size_t i = 0; i < size; ++i) m_bytes[i] = str[i];
		}

		constexpr bool operator==(const Magic&) const = default;

		static constexpr Magic magic() { return "TXFR"; }
		void serialize(Archive& ar) { ar.memory(m_bytes, size); }
	};

	struct CommandLineBinding {
		std::string_view cmd1;
		std::string_view cmd2;
		int (Reflector::*action)(std::span<std::string_view>);
	};

	static fs::path generated_header_path(const fs::path& output, const fs::path& header)
	{
		auto path = output / "include" / header;
		path.replace_extension(".generated.hpp");
		return path;
	}

	static fs::path generated_source_path(const fs::path& output, const fs::path& header)
	{
		auto path = output / "src" / header;
		path.replace_extension(".generated.cpp");
		return path;
	}

	static bool remove_generated_file(const fs::path& path)
	{
		std::error_code error;
		fs::remove(path, error);
		if (error)
		{
			std::cerr << "Error: Failed to remove generated file '" << path << "': " << error.message() << '\n';
			return false;
		}
		return true;
	}

	Reflector::Reflector() {}

	Reflector::~Reflector() {}

	Reflector* Reflector::instance()
	{
		static Reflector reflector;
		return &reflector;
	}

	void Reflector::help()
	{
		std::cout << "Usage:\n"
		          << "  TrinexReflector [options]\n"
		          << "\n"
		          << "Options:\n"
		          << "  --output <directory>      Directory for generated files.\n"
		          << "  --scan-dirs <dirs...>     Directories containing headers to process.\n"
		          << "  --include-dirs <dirs...>  Directories containing headers of linked libraries.\n"
		          << "  --root <directory>        Project root directory.\n"
		          << "  --help                    Show this help message.\n"
		          << "\n"
		          << "Example:\n"
		          << "  TrinexReflector \\\n"
		          << "    --root . \\\n"
		          << "    --output build/generated \\\n"
		          << "    --scan-dirs Engine/include Game/include \\\n"
		          << "    --include-dirs ThirdParty/include External/include\n";

		exit(0);
	}

	int Reflector::on_help(std::span<std::string_view> args)
	{
		help();
		return 0;
	}

	int Reflector::on_output_dir(std::span<std::string_view> args)
	{
		if (!args.empty() && !args[0].starts_with('-'))
		{
			m_output = args[0];
			return 1;
		}

		return 0;
	}

	int Reflector::on_headers_dirs(std::span<std::string_view> args, bool external)
	{
		for (int i = 0, count = static_cast<int>(args.size()); i < count; ++i)
		{
			if (args[i].starts_with('-'))
				return i;

			m_directories.emplace_back(args[i], external);
		}

		return args.size();
	}

	int Reflector::on_scan_dirs(std::span<std::string_view> args)
	{
		return on_headers_dirs(args, false);
	}

	int Reflector::on_include_dirs(std::span<std::string_view> args)
	{
		return on_headers_dirs(args, true);
	}

	int Reflector::on_root(std::span<std::string_view> args)
	{
		if (!args.empty() && !args[0].starts_with('-'))
		{
			m_root = args[0];
			return 1;
		}
		return 0;
	}

	bool Reflector::init(std::span<std::string_view> args)
	{
		m_directories.clear();
		m_output.clear();

		const CommandLineBinding bindings[] = {
		        {"--output", "-o", &Reflector::on_output_dir},
		        {"--scan-dirs", "-s", &Reflector::on_scan_dirs},
		        {"--include-dirs", "-i", &Reflector::on_include_dirs},
		        {"--root", "-r", &Reflector::on_root},
		        {"--help", "-h", &Reflector::on_help},
		};

		for (int i = 0, count = static_cast<int>(args.size()); i < count; ++i)
		{
			std::string_view arg = args[i];

			if (arg.starts_with('-'))
			{
				bool executed = false;
				for (const auto& binding : bindings)
				{
					if (binding.cmd1 == arg || binding.cmd2 == arg)
					{
						i += (this->*binding.action)(args.subspan(i + 1));
						executed = true;
						break;
					}
				}

				if (!executed)
				{
					std::cerr << "Unknown argument: " << arg << '\n';
					return false;
				}
			}
		}

		if (m_output.empty() || m_root.empty())
		{
			std::cerr << "Error: Output and root directories must be specified" << std::endl << std::endl;
			help();
			return false;
		}

		if (!fs::is_directory(m_root))
		{
			std::cerr << "Error: Project root directory does not exist or is not a directory: " << m_root << std::endl
			          << std::endl;
			return false;
		}

		std::error_code status;
		fs::current_path(m_root, status);

		if (status)
		{
			std::cerr << "Failed to set root: " << status.message() << '\n';
		}

		if (!fs::is_directory(m_output))
		{
			std::error_code ec;
			fs::create_directories(m_output, ec);

			if (ec)
			{
				std::cerr << "Error: Failed to create output directory: " << m_output << " (" << ec.message() << ")\n\n";
				return false;
			}

			if (!fs::is_directory(m_output))
			{
				std::cerr << "Error: Output path is not a directory: " << m_output << "\n\n";
				return false;
			}
		}

		m_include = m_output / "include";
		m_sources = m_output / "src";
		return true;
	}

	bool Reflector::collect_headers()
	{
		for (const DirectoryInfo& dir : m_directories)
		{
			if (!fs::is_directory(dir.path))
				continue;

			for (const auto& entry : fs::recursive_directory_iterator(dir.path, fs::directory_options::skip_permission_denied))
			{
				if (!entry.is_regular_file() || entry.path().extension() != ".hpp")
					continue;

				const fs::path& path = entry.path();
				fs::path relative    = fs::relative(path, dir.path);

				if (auto it = m_headers.find(relative); it != m_headers.end())
				{
					const fs::path first  = fs::relative(it->directory->path / it->path);
					const fs::path second = fs::relative(path);

					std::cerr << "Error: Duplicate header path: " << relative << '\n'
					          << "  First:  " << first << '\n'
					          << "  Second: " << second << '\n';

					return false;
				}

				HeaderInfo info;
				info.path      = std::move(relative);
				info.directory = &dir;
				info.size      = entry.file_size();
				info.timestamp = entry.last_write_time().time_since_epoch().count();
				m_headers.insert(std::move(info));
			}
		}

		return true;
	}

	bool Reflector::load_modules()
	{
		std::ifstream file(m_output / "reflection.bin", std::ios::binary);

		if (!file.is_open())
		{
			return true;
		}

		file.exceptions(std::ios::failbit | std::ios::badbit);
		Archive ar = Archive(file);

		if (ar.load<Magic>() != Magic::magic())
		{
			return true;
		}

		const std::uint64_t size = ar.load<std::uint64_t>();

		for (std::uint64_t idx = 0; idx < size; ++idx)
		{
			std::uint64_t timestamp, size, next;
			ar(timestamp, size, next);

			const std::size_t object_start = file.tellg();
			const std::string path         = ar.load<std::string>();

			switch (cache_state(path, timestamp, size))
			{
				case CacheState::Valid:
				{
					auto& entry = m_modules.emplace_back();

					file.seekg(object_start);
					entry.module = new Module();
					entry.module->serialize(ar);
					entry.timestamp = timestamp;
					entry.size      = size;

					assert(next == file.tellg());
					break;
				}

				case CacheState::Outdated:
				{
					file.seekg(next);
					break;
				}

				case CacheState::Removed:
				{
					if (!remove_generated_file(generated_header_path(m_output, path)))
						return false;

					if (!remove_generated_file(generated_source_path(m_output, path)))
						return false;

					file.seekg(next);
					break;
				}
			}
		}
		return true;
	}

	void Reflector::save_modules()
	{
		std::ofstream file(m_output / "reflection.bin", std::ios::binary | std::ios::trunc);

		if (!file.is_open())
		{
			return;
		}

		file.exceptions(std::ios::failbit | std::ios::badbit);
		Archive ar = Archive(file);
		ar.store(Magic::magic());
		ar.store(static_cast<std::uint64_t>(m_modules.size()));

		for (ModuleInfo& info : m_modules)
		{
			ar(info.timestamp, info.size);

			const auto size_offset = file.tellp();
			ar.store<std::uint64_t>(0);

			info.module->serialize(ar);
			const auto end = file.tellp();

			file.seekp(size_offset);
			ar.store<std::uint64_t>(static_cast<std::uint64_t>(end));
			file.seekp(end);
		}
		file.close();
	}

	void Reflector::process()
	{
		for (const HeaderInfo& header : m_headers)
		{
			std::cout << "Generating reflection for " << header.path << std::endl;
			std::ifstream file(header.directory->path / header.path, std::ios::binary | std::ios::ate);

			if (!file.is_open())
				continue;

			const auto size = file.tellg();

			if (size < 0)
				continue;

			std::string source(static_cast<std::size_t>(size), '\0');

			file.seekg(0, std::ios::beg);

			if (!file.read(source.data(), size))
				continue;

			if (Module* module = parse(source, header.path.string()))
			{
				auto& entry     = m_modules.emplace_back();
				entry.module    = module;
				entry.size      = header.size;
				entry.timestamp = header.timestamp;

				generate_header(generated_header_path(m_output, header.path), module);

				if (!header.directory->external)
				{
					generate_source(generated_source_path(m_output, header.path), module);
				}
			}
		}
	}

	Reflector::CacheState Reflector::cache_state(const fs::path& path, std::uint64_t timestamp, std::uint64_t size)
	{
		auto it = m_headers.find(path);

		if (it == m_headers.end())
			return CacheState::Removed;

		if (it->size != size || it->timestamp != timestamp)
			return CacheState::Outdated;

		m_headers.erase(it);
		return CacheState::Valid;
	}

	int Reflector::execute(std::span<std::string_view> args)
	{
		const auto begin     = std::chrono::steady_clock::now();
		auto report_duration = [&] {
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin);
			const auto seconds  = std::chrono::duration_cast<std::chrono::seconds>(duration);
			const auto milliseconds = duration - seconds;
			std::cout << "Reflector completed in " << seconds.count() << " s " << milliseconds.count() << " ms\n";
		};

		if (!init(args))
		{
			report_duration();
			return 1;
		}

		if (!collect_headers())
		{
			report_duration();
			return 1;
		}

		if (!load_modules())
		{
			report_duration();
			return 1;
		}
		process();
		save_modules();
		report_duration();
		return 0;
	}
}// namespace Reflector
