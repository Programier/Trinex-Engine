#include <LinuxPlatform/context.hpp>
#include <LinuxPlatform/memory.hpp>
#include <LinuxPlatform/system.hpp>

namespace Trinex::Platform
{
	LinuxContext::~LinuxContext() = default;

	Context* Context::instance()
	{
		static LinuxContext ctx;
		return &ctx;
	}

	System* LinuxContext::system()
	{
		return LinuxSystem::instance();
	}

	Memory* LinuxContext::memory()
	{
		return LinuxMemory::instance();
	}
} // namespace Trinex::Platform
