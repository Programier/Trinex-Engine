#pragma once
#include <Core/etl/string.hpp>
#include <Core/types/path.hpp>
#include <Core/types/uuid.hpp>
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct SystemInfo {
		SystemType type           = SystemType::Unknown;
		Architecture architecture = Architecture::Unknown;
		const char* name;
		const char* display_name;
		const char* kernel_name;
		const char* kernel_version;
		const char* computer_name;
		const char* user_name;
	};

	struct CPUInfo {
		String vendor;
		String brand;
		Architecture architecture = Architecture::Unknown;
		CPUFeature features       = CPUFeature::Undefined;
		u32 physical_cores        = 0;
		u32 logical_cores         = 0;
		u32 cache_line_size       = 0;
		u32 l1_cache_size         = 0;
		u32 l2_cache_size         = 0;
		u32 l3_cache_size         = 0;
	};

	class ENGINE_EXPORT System
	{
	public:
		static System* instance();

		virtual ~System() = default;

		virtual SystemType system_type() const           = 0;
		virtual const char* name() const                 = 0;
		virtual const SystemInfo& system_info() const    = 0;
		virtual const CPUInfo& cpu_info() const          = 0;
		virtual const char* executable_path() const      = 0;
		virtual const char* executable_directory() const = 0;
		virtual const char* current_directory() const    = 0;
		virtual bool current_directory(const Path* path) = 0;

		virtual const char* environment(const char* name) const                            = 0;
		virtual bool environment(const char* name, const char* value, bool replace = true) = 0;
		virtual bool create_uuid(UUID* out)                                                = 0;
	};
}// namespace Trinex::Platform
