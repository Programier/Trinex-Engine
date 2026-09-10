#include <LinuxPlatform/memory.hpp>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <unistd.h>

namespace Trinex::Platform
{
	static int to_native_protection(MemoryProtection protection)
	{
		if (protection == MemoryProtection::NoAccess)
			return PROT_NONE;

		int result = PROT_NONE;

		if (protection & MemoryProtection::Read)
			result |= PROT_READ;

		if (protection & MemoryProtection::Write)
			result |= PROT_WRITE;

		if (protection & MemoryProtection::Execute)
			result |= PROT_EXEC;

		return result;
	}

	static int to_native_flags(VirtualMemoryFlags flags)
	{
		int result = MAP_ANONYMOUS;

		if (flags & VirtualMemoryFlags::Shared)
			result |= MAP_SHARED;
		else
			result |= MAP_PRIVATE;

		if (flags & VirtualMemoryFlags::LargePage)
			result |= MAP_HUGETLB;

		if (flags & VirtualMemoryFlags::Stack)
			result |= MAP_STACK;

		if (flags & VirtualMemoryFlags::NoReserve)
			result |= MAP_NORESERVE;

		if (flags & VirtualMemoryFlags::Populate)
			result |= MAP_POPULATE;

		if (flags & VirtualMemoryFlags::FixedNoReplace)
			result |= MAP_FIXED_NOREPLACE;

		return result;
	}

	static u64 read_meminfo_value(const char* prefix)
	{
		std::ifstream file("/proc/meminfo");
		String line;

		while (std::getline(file, line))
		{
			if (!line.starts_with(prefix))
				continue;

			std::istringstream stream(line.substr(std::strlen(prefix)));
			u64 value = 0;
			stream >> value;
			return value * 1024;
		}

		return 0;
	}


	LinuxMemory::LinuxMemory() {}

	LinuxMemory* LinuxMemory::instance()
	{
		static LinuxMemory memory;
		return &memory;
	}

	void* LinuxMemory::reserve(usize size, void* address, VirtualMemoryFlags flags)
	{
		if (size == 0)
			return nullptr;

		if ((flags & VirtualMemoryFlags::FixedNoReplace) && address == nullptr)
			return nullptr;

		const int native_flags = to_native_flags(flags);

		void* result = ::mmap(address, size, PROT_NONE, native_flags, -1, 0);

		if (result == MAP_FAILED)
			return nullptr;

		return result;
	}

	bool LinuxMemory::commit(void* address, usize size, MemoryProtection protection)
	{
		if (address == nullptr || size == 0)
			return false;

		return ::mprotect(address, size, to_native_protection(protection)) == 0;
	}

	bool LinuxMemory::decommit(void* address, usize size)
	{
		if (address == nullptr || size == 0)
			return false;

		if (::mprotect(address, size, PROT_NONE) != 0)
			return false;

		if (::madvise(address, size, MADV_DONTNEED) != 0)
			return false;

		return true;
	}

	bool LinuxMemory::release(void* address, usize size)
	{
		if (address == nullptr || size == 0)
			return false;

		return ::munmap(address, size) == 0;
	}

	bool LinuxMemory::protect(void* address, usize size, MemoryProtection protection)
	{
		if (address == nullptr || size == 0)
			return false;

		return ::mprotect(address, size, to_native_protection(protection)) == 0;
	}

	bool LinuxMemory::flush(void* address, usize size)
	{
		if (address == nullptr || size == 0)
			return false;

		auto* begin = static_cast<u8*>(address);
		auto* end   = begin + size;

		__builtin___clear_cache(begin, end);

		return true;
	}

	bool LinuxMemory::lock(void* address, usize size)
	{
		if (address == nullptr || size == 0)
			return false;

		return ::mlock(address, size) == 0;
	}

	bool LinuxMemory::unlock(void* address, usize size)
	{
		if (address == nullptr || size == 0)
			return false;

		return ::munlock(address, size) == 0;
	}

	usize LinuxMemory::page_size() const
	{
		static const usize size = [] {
			const long value = ::sysconf(_SC_PAGESIZE);
			return value > 0 ? static_cast<usize>(value) : usize{0};
		}();

		return size;
	}

	usize LinuxMemory::granularity() const
	{
		return page_size();
	}

	MemoryInfo LinuxMemory::info() const
	{
		MemoryInfo info;
		struct sysinfo sys = {};

		if (sysinfo(&sys) == 0)
		{
			info.total_physical     = static_cast<u64>(sys.totalram) * sys.mem_unit;
			info.available_physical = read_meminfo_value("MemAvailable:");
			info.total_virtual      = (static_cast<u64>(sys.totalram) + static_cast<u64>(sys.totalswap)) * sys.mem_unit;
			info.available_virtual  = (static_cast<u64>(sys.freeram) + static_cast<u64>(sys.freeswap)) * sys.mem_unit;
		}

		std::ifstream statm("/proc/self/statm");
		u64 size     = 0;
		u64 resident = 0;
		statm >> size >> resident;

		const u64 page_size   = static_cast<u64>(sysconf(_SC_PAGESIZE));
		info.process_virtual  = size * page_size;
		info.process_physical = resident * page_size;

		return info;
	}
}// namespace Trinex::Platform
