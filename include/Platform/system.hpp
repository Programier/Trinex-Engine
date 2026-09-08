#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Core/types/uuid.hpp>
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct SystemInfo {
		SystemType type           = SystemType::Unknown;
		Architecture architecture = Architecture::Unknown;
		Version version;
		String name;
		String display_name;
		String kernel_name;
		String kernel_version;
		String computer_name;
		String user_name;
		u32 page_size              = 0;
		u32 allocation_granularity = 0;
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

		virtual SystemType system_type() const                      = 0;
		virtual const String* name() const                          = 0;
		virtual const SystemInfo* system_info() const               = 0;
		virtual const CPUInfo* cpu_info() const                     = 0;
		virtual Path executable_path() const                        = 0;
		virtual Path executable_directory() const                   = 0;
		virtual Path current_directory() const                      = 0;
		virtual bool current_directory(const Path* path)            = 0;
		virtual String environment(StringView name) const           = 0;
		virtual bool environment(StringView name, StringView value) = 0;
		virtual bool remove_environment(StringView name)            = 0;
		virtual bool create_uuid(UUID* out)                         = 0;
	};
}// namespace Trinex::Platform
