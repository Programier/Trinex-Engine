#pragma once

#include <Core/filesystem/file.hpp>
#include <Core/filesystem/filesystem.hpp>

namespace Trinex::Platform
{
	class LinuxFile : public VFS::File
	{
	private:
		int m_fd;

	public:
		LinuxFile(int fd);
		~LinuxFile();

		bool readable() const override;
		bool writable() const override;

		u64 offset(i64 value, IOWhence whence = IOWhence::Current) override;
		u64 offset() const override;
		usize read(void* buffer, usize size) override;
		usize write(const void* buffer, usize size) override;
		bool flush() override;

		VFS::FileSystem* filesystem() const override;
		VFS::FileStat stat() const override;
		Ref<Blob> map(usize offset = 0, usize size = ~usize(0)) override;

		static Ref<Blob> map(int fd, usize offset = 0, usize size = ~usize(0));
	};

	class LinuxFileSystem : public ExternalLifetime<VFS::FileSystem>
	{
	private:
		int create_descriptor(PathView path, VFS::AccessFlags flags = VFS::AccessFlags::Read);

	public:
		static LinuxFileSystem* instance();

		Ref<VFS::File> open(PathView path, VFS::AccessFlags flags) override;
		Ref<Blob> map(PathView path, VFS::AccessFlags flags, usize offset, usize size) override;
		bool stat(PathView path, VFS::FileStat& out) const override;

		bool create_directory(PathView path) override;
		bool remove(PathView path) override;
		bool copy(PathView src, PathView dst) override;
		bool move(PathView src, PathView dst) override;

		bool walk(PathView path, const WalkCallback& callback, VFS::WalkFlags flags = VFS::WalkFlags::Default) const override;
	};
}// namespace Trinex::Platform
