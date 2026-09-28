#include <Core/blob.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <LinuxPlatform/filesystem.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Trinex::Platform
{
	class LinuxMappedBlob final : public Blob
	{
	public:
		LinuxMappedBlob(void* mapping, usize mapping_size, u8* data, usize size)
		    : m_mapping(mapping), m_mapping_size(mapping_size), m_data(data), m_size(size)
		{}

		~LinuxMappedBlob() override
		{
			if (m_mapping && m_mapping_size)
				::munmap(m_mapping, m_mapping_size);
		}

		usize size() const override { return m_size; }
		u8* data() override { return m_data; }
		const u8* data() const override { return m_data; }

	private:
		void* m_mapping      = nullptr;
		usize m_mapping_size = 0;

		u8* m_data   = nullptr;
		usize m_size = 0;
	};

	static inline u64 timestamp_of(const timespec& time)
	{
		return static_cast<u64>(time.tv_sec) * 1'000'000'000ull + static_cast<u64>(time.tv_nsec);
	}

	static int native_flags_of(VFS::AccessFlags flags)
	{
		const bool read   = !!(flags & VFS::AccessFlags::Read);
		const bool write  = !!(flags & VFS::AccessFlags::Write);
		const bool append = (flags & VFS::AccessFlags::Append) == VFS::AccessFlags::Append;

		int result = O_CLOEXEC;

		if (read && write)
		{
			result |= O_RDWR;
		}
		else if (write)
		{
			result |= O_WRONLY | O_CREAT;

			if (append)
				result |= O_APPEND;
			else
				result |= O_TRUNC;
		}
		else
		{
			result |= O_RDONLY;
		}

		return result;
	}


	static VFS::FileStat file_stat_of(const struct stat& stat)
	{
		VFS::FileStat result{};

		if (S_ISREG(stat.st_mode))
			result.type = VFS::FileType::Regular;
		else if (S_ISDIR(stat.st_mode))
			result.type = VFS::FileType::Directory;
		else if (S_ISLNK(stat.st_mode))
			result.type = VFS::FileType::Other;
		else
			result.type = VFS::FileType::Undefined;

		result.size = static_cast<u64>(stat.st_size);

		result.accessed_time = timestamp_of(stat.st_atim);
		result.modified_time = timestamp_of(stat.st_mtim);
		result.created_time  = 0;

		return result;
	}

	static bool create_parent_directories(const char* path)
	{
		String current;

		for (const char* p = path; *p; ++p)
		{
			if (*p != '/')
			{
				current += *p;
				continue;
			}

			if (current.empty())
			{
				current += '/';
				continue;
			}

			if (current.back() == '/')
				continue;

			if (::mkdir(current.c_str(), 0777) != 0)
			{
				if (errno != EEXIST)
					return false;

				struct stat st{};

				if (::stat(current.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
				{
					return false;
				}
			}

			current += '/';
		}

		return true;
	}

	LinuxFile::LinuxFile(int fd) : m_fd(fd)
	{
		trinex_assert(fd >= 0);
	}

	LinuxFile::~LinuxFile()
	{
		if (m_fd >= 0)
			::close(m_fd);
	}

	bool LinuxFile::readable() const
	{
		const int flags = ::fcntl(m_fd, F_GETFL);

		if (flags < 0)
			return false;

		const int access = flags & O_ACCMODE;

		return access == O_RDONLY || access == O_RDWR;
	}

	bool LinuxFile::writable() const
	{
		if (m_fd < 0)
			return false;

		const int flags = ::fcntl(m_fd, F_GETFL);

		if (flags < 0)
			return false;

		const int access = flags & O_ACCMODE;

		return access == O_WRONLY || access == O_RDWR;
	}

	u64 LinuxFile::offset(i64 value, IOWhence whence)
	{
		int native_whence;

		switch (whence)
		{
			case IOWhence::Begin: native_whence = SEEK_SET; break;
			case IOWhence::Current: native_whence = SEEK_CUR; break;
			case IOWhence::End: native_whence = SEEK_END; break;
			default: return ~u64(0);
		}

		const off64_t result = ::lseek64(m_fd, static_cast<off64_t>(value), native_whence);

		if (result == static_cast<off64_t>(-1))
			return ~u64(0);

		return static_cast<u64>(result);
	}

	u64 LinuxFile::offset() const
	{
		const off64_t result = ::lseek(m_fd, 0, SEEK_CUR);

		if (result == static_cast<off64_t>(-1))
			return ~u64(0);

		return static_cast<u64>(result);
	}

	usize LinuxFile::read(void* buffer, usize size)
	{
		ssize_t result;

		do
		{
			result = ::read(m_fd, buffer, size);
		} while (result < 0 && errno == EINTR);

		return result > 0 ? static_cast<usize>(result) : 0;
	}

	usize LinuxFile::write(const void* buffer, usize size)
	{
		ssize_t result;

		do
		{
			result = ::write(m_fd, buffer, size);
		} while (result < 0 && errno == EINTR);

		return result > 0 ? static_cast<usize>(result) : 0;
	}

	bool LinuxFile::flush()
	{
		for (;;)
		{
			if (::fsync(m_fd) == 0)
				return true;

			if (errno != EINTR)
				return false;
		}
	}

	VFS::FileSystem* LinuxFile::filesystem() const
	{
		return LinuxFileSystem::instance();
	}

	VFS::FileStat LinuxFile::stat() const
	{
		struct ::stat st{};

		if (::fstat(m_fd, &st) != 0)
			return {};

		return file_stat_of(st);
	}

	Ref<Blob> LinuxFile::map(usize offset, usize size)
	{
		return map(m_fd, offset, size);
	}

	Ref<Blob> LinuxFile::map(int fd, usize offset, usize size)
	{
		struct ::stat st{};

		if (::fstat(fd, &st) != 0)
			return {};

		if (st.st_size < 0)
			return {};

		const u64 file_size64 = static_cast<u64>(st.st_size);

		if (offset > file_size64)
			return {};

		const usize available = static_cast<usize>(file_size64 - offset);

		if (size == ~usize(0))
		{
			size = available;
		}
		else if (size > available)
		{
			return {};
		}

		if (size == 0)
		{
			return Ref<LinuxMappedBlob>::make(nullptr, 0, nullptr, 0);
		}

		const long page_size_native = ::sysconf(_SC_PAGESIZE);

		if (page_size_native <= 0)
			return {};

		const usize page_size = static_cast<usize>(page_size_native);

		const usize aligned_offset = offset - (offset % page_size);

		const usize delta = offset - aligned_offset;

		if (size > ~usize(0) - delta)
			return {};

		const usize mapping_size = delta + size;

		const int file_flags = ::fcntl(fd, F_GETFL);

		if (file_flags < 0)
			return {};

		const int access = file_flags & O_ACCMODE;

		int protection;
		int mapping_flags;

		switch (access)
		{
			case O_RDONLY:
				protection    = PROT_READ;
				mapping_flags = MAP_PRIVATE;
				break;

			case O_RDWR:
				protection    = PROT_READ | PROT_WRITE;
				mapping_flags = MAP_SHARED;
				break;

			case O_WRONLY:
			default: return {};
		}

		void* mapping = ::mmap(nullptr, mapping_size, protection, mapping_flags, fd, static_cast<off_t>(aligned_offset));

		if (mapping == MAP_FAILED)
			return {};

		auto* data = static_cast<u8*>(mapping) + delta;

		return Ref<LinuxMappedBlob>::make(mapping, mapping_size, data, size);
	}

	LinuxFileSystem* LinuxFileSystem::instance()
	{
		static LinuxFileSystem fs;
		return &fs;
	}

	int LinuxFileSystem::create_descriptor(PathView path, VFS::AccessFlags flags)
	{
		String native_path(path);

		if (flags.all(VFS::AccessFlags::Recursive | VFS::AccessFlags::Write))
		{
			if (!create_parent_directories(native_path.c_str()))
				return {};
		}

		return ::open(native_path.c_str(), native_flags_of(flags), 0666);
	}

	Ref<VFS::File> LinuxFileSystem::open(PathView path, VFS::AccessFlags flags)
	{
		int fd = create_descriptor(path, flags);

		if (fd < 0)
			return {};

		return Ref<LinuxFile>::make(fd);
	}

	Ref<Blob> LinuxFileSystem::map(PathView path, VFS::AccessFlags flags, usize offset, usize size)
	{
		int fd = create_descriptor(path, flags);

		if (fd < 0)
			return {};

		auto blob = LinuxFile::map(fd, offset, size);
		::close(fd);

		return blob;
	}

	bool LinuxFileSystem::stat(PathView path, VFS::FileStat& out) const
	{
		String native_path(path);

		struct ::stat stat{};

		if (::lstat(native_path.c_str(), &stat) != 0)
			return false;

		out = file_stat_of(stat);
		return true;
	}

	bool LinuxFileSystem::create_directory(PathView path)
	{
		String native_path(path);
		return create_parent_directories(native_path.c_str());
	}

	bool LinuxFileSystem::remove(PathView path)
	{
		String native_path(path);
		return ::remove(native_path.c_str()) == 0;
	}

	bool LinuxFileSystem::copy(PathView src, PathView dst)
	{
		String native_src(src);
		String native_dst(dst);

		int src_fd = ::open(native_src.c_str(), O_RDONLY | O_CLOEXEC);

		if (src_fd < 0)
			return false;

		struct ::stat src_stat{};

		if (::fstat(src_fd, &src_stat) != 0)
		{
			::close(src_fd);
			return false;
		}

		if (!S_ISREG(src_stat.st_mode))
		{
			::close(src_fd);
			return false;
		}

		int dst_fd = ::open(native_dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, src_stat.st_mode & 0777);

		if (dst_fd < 0)
		{
			::close(src_fd);
			return false;
		}

		bool success = true;
		u8 buffer[64 * 1024];

		for (;;)
		{
			ssize_t read_size;

			do
			{
				read_size = ::read(src_fd, buffer, sizeof(buffer));
			} while (read_size < 0 && errno == EINTR);

			if (read_size == 0)
				break;

			if (read_size < 0)
			{
				success = false;
				break;
			}

			usize written = 0;

			while (written < static_cast<usize>(read_size))
			{
				ssize_t write_size;

				do
				{
					write_size = ::write(dst_fd, buffer + written, static_cast<usize>(read_size) - written);
				} while (write_size < 0 && errno == EINTR);

				if (write_size <= 0)
				{
					success = false;
					break;
				}

				written += static_cast<usize>(write_size);
			}

			if (!success)
				break;
		}

		if (::close(dst_fd) != 0)
			success = false;

		::close(src_fd);

		return success;
	}

	bool LinuxFileSystem::move(PathView src, PathView dst)
	{
		String native_src(src);
		String native_dst(dst);

		if (::rename(native_src.c_str(), native_dst.c_str()) == 0)
			return true;

		if (errno != EXDEV)
			return false;

		if (!copy(src, dst))
			return false;

		if (!remove(src))
		{
			return false;
		}

		return true;
	}

	bool LinuxFileSystem::walk(PathView path, const WalkCallback& callback, VFS::WalkFlags flags) const
	{
		return false;
	}

	trinex_on_pre_init()
	{
		rootfs()->mount("/", retain_ref(LinuxFileSystem::instance()));
	}
}// namespace Trinex::Platform
