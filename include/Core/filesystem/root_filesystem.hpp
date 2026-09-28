#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/filesystem/file_watcher.hpp>
#include <Core/filesystem/filesystem.hpp>

namespace Trinex::VFS
{
	class ENGINE_EXPORT RootFS : public Singletone<RootFS, FileSystem>
	{
	public:
		struct Greater {
			using is_transparent = void;
			bool operator()(StringView lhs, StringView rhs) const { return lhs > rhs; }
		};

		using FileSystems = FlatMap<String, Ref<FileSystem>, Greater>;

	private:
		static RootFS* s_instance;
		FileSystems m_file_systems;


	public:
		Ref<File> open(PathView path, AccessFlags flags = AccessFlags::Read) override;
		Ref<Blob> map(PathView path, AccessFlags flags = AccessFlags::Read, usize offset = 0, usize size = ~usize(0)) override;

		bool stat(PathView path, FileStat& out) const override;
		bool create_directory(PathView path) override;
		bool remove(PathView path) override;
		bool copy(PathView src, PathView dst) override;
		bool move(PathView src, PathView dst) override;
		bool walk(PathView path, const WalkCallback& callback, WalkFlags flags = WalkFlags::Default) const override;
		using FileSystem::walk;

		bool mount(PathView point, PathView path);
		bool mount(PathView point, Ref<FileSystem> system);
		bool unmount(PathView point);

		FileSystem* resolve(PathView& path) const;

		friend class Singletone<RootFS, FileSystem>;
	};
}// namespace Trinex::VFS


namespace Trinex
{
	ENGINE_EXPORT VFS::RootFS* rootfs();
}
