#pragma once
#include <Platform/system.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT LinuxSystem final : public System
	{
	private:
		SystemInfo m_system_info;
		CPUInfo m_cpu_info;
		String m_name;

	private:
		LinuxSystem();

	public:
		static LinuxSystem* instance();

		SystemType system_type() const override;
		const String* name() const override;
		const SystemInfo* system_info() const override;
		const CPUInfo* cpu_info() const override;
		Path executable_path() const override;
		Path executable_directory() const override;
		Path current_directory() const override;
		bool current_directory(const Path* path) override;
		String environment(StringView name) const override;
		bool environment(StringView name, StringView value) override;
		bool remove_environment(StringView name) override;
		bool create_uuid(UUID* out) override;
	};
}// namespace Trinex::Platform
