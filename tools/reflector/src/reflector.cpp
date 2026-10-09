#include <algorithm>
#include <archive.hpp>
#include <cassert>
#include <filesystem>
#include <fstream>
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
		return true;
	}

	void Reflector::collect_headers()
	{
		for (const DirectoryInfo& dir : m_directories)
		{
			if (!fs::exists(dir.path) || !fs::is_directory(dir.path))
				continue;

			for (const auto& entry : fs::recursive_directory_iterator(dir.path, fs::directory_options::skip_permission_denied))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".hpp")
				{
					HeaderInfo& info = m_headers.emplace_back();
					info.path        = fs::relative(entry.path());
					info.size        = fs::file_size(entry.path());
					info.timestamp   = entry.last_write_time().time_since_epoch().count();
					info.external    = dir.external;
				}
			}
		}
	}

	void Reflector::load_modules()
	{
		std::ifstream file(m_output / "reflection.bin", std::ios::binary);

		if (!file.is_open())
		{
			return;
		}

		file.exceptions(std::ios::failbit | std::ios::badbit);
		Archive ar = Archive(file);

		if (ar.load<Magic>() != Magic::magic())
		{
			return;
		}

		const std::uint64_t size = ar.load<std::uint64_t>();

		for (std::uint64_t idx = 0; idx < size; ++idx)
		{
			std::uint64_t timestamp, size, next;
			ar(timestamp, size, next);

			const std::size_t object_start = file.tellg();
			const std::string path         = ar.load<std::string>();

			if (is_cache_valid(path, timestamp, size))
			{
				auto& entry = m_modules.emplace_back();

				file.seekg(object_start);
				entry.module = new Module();
				entry.module->serialize(ar);
				entry.timestamp = timestamp;
				entry.size      = size;

				assert(next == file.tellg());
			}
			else
			{
				file.seekg(next);
			}
		}
	}

	void Reflector::save_modules()
	{
		if (m_headers.empty())
			return;

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
		for (HeaderInfo& header : m_headers)
		{
			std::cout << "Generating reflection for '" << header.path << "'" << std::endl;
			std::ifstream file(header.path, std::ios::binary | std::ios::ate);

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
			}
		}
	}

	bool Reflector::is_cache_valid(std::string_view path, std::size_t timestamp, std::size_t size)
	{
		auto it = std::find_if(m_headers.begin(), m_headers.end(), [path](const HeaderInfo& info) { return info.path == path; });

		if (it == m_headers.end())
			return false;

		if (it->size != size || it->timestamp != timestamp)
			return false;

		if (it != std::prev(m_headers.end()))
			std::swap(*it, m_headers.back());

		m_headers.pop_back();
		return true;
	}

	int Reflector::execute(std::span<std::string_view> args)
	{
		if (!init(args))
			return 1;

		collect_headers();
		load_modules();
		process();
		save_modules();
		return 0;
	}
}// namespace Reflector
