#pragma once
#include <Platform/system.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT LinuxSystem final : public System
	{
	private:
		LinuxSystem();

	public:
		static LinuxSystem* instance();

		SystemType system_type() const override;
		const char* name() const override;
		const SystemInfo& system_info() const override;
		const CPUInfo& cpu_info() const override;
		const char* executable_path() const override;
		const char* executable_directory() const override;
		const char* current_directory() const override;
		bool current_directory(const Path* path) override;
		const char* environment(const char* name) const override;
		bool environment(const char* name, const char* value, bool replace = true) override;
		bool create_uuid(UUID* out) override;
	};
}// namespace Trinex::Platform
