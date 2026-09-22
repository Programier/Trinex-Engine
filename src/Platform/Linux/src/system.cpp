#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/uuid.hpp>
#include <LinuxPlatform/system.hpp>
#include <cstdlib>
#include <fstream>
#include <linux/limits.h>
#include <linux/random.h>
#include <pwd.h>
#include <sys/random.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>


namespace Trinex::Platform
{
	static String read_file_line(const char* path, const char* prefix)
	{
		std::ifstream file(path);
		String line;

		while (std::getline(file, line))
		{
			if (line.starts_with(prefix))
			{
				auto pos = line.find(':');
				if (pos == String::npos)
					pos = line.find('=');

				if (pos != String::npos)
				{
					pos++;
					while (pos < line.size() && line[pos] == ' ') pos++;
					return line.substr(pos);
				}
			}
		}

		return {};
	}

	static CPUFeature parse_cpu_features(const String& flags)
	{
		CPUFeature features = CPUFeature::Undefined;

		auto has = [&flags](const char* name) {
			const String token = String(" ") + name + " ";
			return (String(" ") + flags + " ").find(token) != String::npos;
		};

		features.set(CPUFeature::SSE, has("sse"));
		features.set(CPUFeature::SSE2, has("sse2"));
		features.set(CPUFeature::SSE3, has("sse3"));
		features.set(CPUFeature::SSSE3, has("ssse3"));
		features.set(CPUFeature::SSE41, has("sse4_1"));
		features.set(CPUFeature::SSE42, has("sse4_2"));
		features.set(CPUFeature::AVX, has("avx"));
		features.set(CPUFeature::AVX2, has("avx2"));
		features.set(CPUFeature::AVX512F, has("avx512f"));
		features.set(CPUFeature::AES, has("aes"));
		features.set(CPUFeature::SHA, has("sha_ni") || has("sha1") || has("sha2"));
		features.set(CPUFeature::FMA, has("fma"));
		features.set(CPUFeature::NEON, has("asimd") || has("neon"));
		features.set(CPUFeature::CRC32, has("crc32") || has("sse4_2"));

		return features;
	}

	static Architecture current_architecture()
	{
#if ARCH_X86_64
		return Architecture::X86_64;
#elif ARCH_ARM
		return sizeof(void*) == 8 ? Architecture::ARM64 : Architecture::ARM;
#else
		return Architecture::Unknown;
#endif
	}

	LinuxSystem::LinuxSystem() {}

	LinuxSystem* LinuxSystem::instance()
	{
		static LinuxSystem system;
		return &system;
	}

	SystemType LinuxSystem::system_type() const
	{
		return SystemType::Linux;
	}

	const char* LinuxSystem::name() const
	{
		return "Linux";
	}

	const SystemInfo& LinuxSystem::system_info() const
	{
		static SystemInfo info = []() -> SystemInfo {
			static utsname uts = {};
			uname(&uts);

			static String display_name = read_file_line("/etc/os-release", "PRETTY_NAME=");
			if (display_name.size() >= 2 && display_name.front() == '"' && display_name.back() == '"')
				display_name = display_name.substr(1, display_name.size() - 2);

			SystemInfo info;
			info.type           = SystemType::Linux;
			info.architecture   = current_architecture();
			info.name           = "Linux";
			info.kernel_name    = uts.sysname;
			info.kernel_version = uts.release;
			info.computer_name  = uts.nodename;
			info.display_name   = display_name.c_str();

			info.user_name = []() -> const char* {
				passwd pwd{};
				passwd* result = nullptr;

				size_t size = 16384;
				Vector<char> buffer(size);

				for (;;)
				{
					const int error = getpwuid_r(getuid(), &pwd, buffer.data(), buffer.size(), &result);

					if (error == 0 && result)
					{
						static String username = pwd.pw_name;
						return username.c_str();
					}

					if (error != ERANGE)
						return "Unknown";

					buffer.resize(buffer.size() * 2);
				}
			}();

			return info;
		}();

		return info;
	}

	const CPUInfo& LinuxSystem::cpu_info() const
	{
		static CPUInfo info = []() -> CPUInfo {
			static String vendor = read_file_line("/proc/cpuinfo", "vendor_id");
			static String brand  = read_file_line("/proc/cpuinfo", "model name");

			String cpu_features = read_file_line("/proc/cpuinfo", "flags");
			if (cpu_features.empty())
				cpu_features = read_file_line("/proc/cpuinfo", "Features");

			CPUInfo info;
			info.vendor          = vendor.c_str();
			info.brand           = brand.c_str();
			info.architecture    = instance()->system_info().architecture;
			info.features        = parse_cpu_features(cpu_features);
			info.logical_cores   = static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
			info.physical_cores  = info.logical_cores;
			info.cache_line_size = 64;

			return info;
		}();

		return info;
	}

	const char* LinuxSystem::executable_path() const
	{
		static const char* path = []() -> const char* {
			static String path;
			path.resize(256);

			for (;;)
			{
				const ssize_t length = ::readlink("/proc/self/exe", path.data(), path.size());

				if (length < 0)
					return {};

				if (static_cast<usize>(length) < path.size())
				{
					path.resize(static_cast<usize>(length));
					return path.c_str();
				}

				path.resize(path.size() * 2);
			}
		}();

		return path;
	}

	const char* LinuxSystem::executable_directory() const
	{
		static const char* directory = []() -> const char* {
			static String path = instance()->executable_path();

			if (path.empty())
				return "";

			const usize pos = path.find_last_of('/');

			if (pos == String::npos)
				return ".";

			if (pos == 0)
				return "/";

			path.resize(pos);
			return path.c_str();
		}();

		return directory;
	}

	const char* LinuxSystem::current_directory() const
	{
		static thread_local char buffer[PATH_MAX];
		return getcwd(buffer, PATH_MAX);
	}

	bool LinuxSystem::current_directory(const Path* path)
	{
		return path && chdir(path->c_str()) == 0;
	}

	const char* LinuxSystem::environment(const char* name) const
	{
		return getenv(name);
	}

	bool LinuxSystem::environment(const char* name, const char* value, bool replace)
	{
		if (value == nullptr)
		{
			return unsetenv(value) == 0;
		}
		else
		{
			return setenv(name, value, replace) == 0;
		}
	}

	bool LinuxSystem::create_uuid(UUID* out)
	{
		if (out == nullptr)
			return false;

		*out     = UUID();
		u8* data = out->data();

		if (getrandom(data, UUID::byte_count, 0) != static_cast<ssize_t>(UUID::byte_count))
			return false;

		data[6] = static_cast<u8>((data[6] & 0x0F) | 0x40);
		data[8] = static_cast<u8>((data[8] & 0x3F) | 0x80);
		return true;
	}
}// namespace Trinex::Platform
