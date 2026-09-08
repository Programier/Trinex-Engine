#include <Core/types/uuid.hpp>
#include <LinuxPlatform/system.hpp>
#include <cstdlib>
#include <fstream>
#include <linux/random.h>
#include <sstream>
#include <sys/random.h>
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

	static Version parse_kernel_version(const String& value)
	{
		Version version;
		std::istringstream stream(value);
		char dot = 0;
		stream >> version.major >> dot >> version.minor >> dot >> version.patch;
		return version;
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

	LinuxSystem::LinuxSystem()
	{
		utsname uts = {};
		uname(&uts);

		m_name                               = "Linux";
		m_system_info.type                   = SystemType::Linux;
		m_system_info.architecture           = current_architecture();
		m_system_info.name                   = m_name;
		m_system_info.kernel_name            = uts.sysname;
		m_system_info.kernel_version         = uts.release;
		m_system_info.version                = parse_kernel_version(m_system_info.kernel_version);
		m_system_info.computer_name          = uts.nodename;
		m_system_info.page_size              = static_cast<u32>(sysconf(_SC_PAGESIZE));
		m_system_info.allocation_granularity = m_system_info.page_size;

		if (const char* user = getenv("USER"))
			m_system_info.user_name = user;

		m_system_info.display_name = read_file_line("/etc/os-release", "PRETTY_NAME=");
		if (m_system_info.display_name.size() >= 2 && m_system_info.display_name.front() == '"' &&
		    m_system_info.display_name.back() == '"')
		{
			m_system_info.display_name = m_system_info.display_name.substr(1, m_system_info.display_name.size() - 2);
		}

		m_cpu_info.vendor       = read_file_line("/proc/cpuinfo", "vendor_id");
		m_cpu_info.brand        = read_file_line("/proc/cpuinfo", "model name");
		m_cpu_info.architecture = m_system_info.architecture;
		String cpu_features     = read_file_line("/proc/cpuinfo", "flags");

		if (cpu_features.empty())
			cpu_features = read_file_line("/proc/cpuinfo", "Features");

		m_cpu_info.features        = parse_cpu_features(cpu_features);
		m_cpu_info.logical_cores   = static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
		m_cpu_info.physical_cores  = m_cpu_info.logical_cores;
		m_cpu_info.cache_line_size = 64;
	}

	LinuxSystem* LinuxSystem::instance()
	{
		static LinuxSystem system;
		return &system;
	}

	SystemType LinuxSystem::system_type() const
	{
		return SystemType::Linux;
	}

	const String* LinuxSystem::name() const
	{
		return &m_name;
	}

	const SystemInfo* LinuxSystem::system_info() const
	{
		return &m_system_info;
	}

	const CPUInfo* LinuxSystem::cpu_info() const
	{
		return &m_cpu_info;
	}

	Path LinuxSystem::executable_path() const
	{
		char buffer[PATH_MAX] = {};
		const ssize_t size    = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		return size > 0 ? Path(StringView(buffer, static_cast<usize>(size))) : Path();
	}

	Path LinuxSystem::executable_directory() const
	{
		Path path = executable_path();
		return path.parent();
	}

	Path LinuxSystem::current_directory() const
	{
		char buffer[PATH_MAX] = {};
		return getcwd(buffer, sizeof(buffer)) ? Path(buffer) : Path();
	}

	bool LinuxSystem::current_directory(const Path* path)
	{
		return path && chdir(path->c_str()) == 0;
	}

	String LinuxSystem::environment(StringView name) const
	{
		const String variable(name);
		if (const char* value = getenv(variable.c_str()))
			return value;
		return {};
	}

	bool LinuxSystem::environment(StringView name, StringView value)
	{
		const String variable(name);
		const String variable_value(value);
		return setenv(variable.c_str(), variable_value.c_str(), 1) == 0;
	}

	bool LinuxSystem::remove_environment(StringView name)
	{
		const String variable(name);
		return unsetenv(variable.c_str()) == 0;
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
