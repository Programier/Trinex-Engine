#pragma once
#include <Core/callback.hpp>
#include <Core/enums.hpp>
#include <Core/etl/array.hpp>
#include <Core/types/path.hpp>

namespace Trinex::VFS
{
	struct FileWatchEventType {
		enum Enum : u32
		{
			Undefined = 0,
			Created   = trinex_bit(0),
			Removed   = trinex_bit(1),
			Modified  = trinex_bit(2),
			Renamed   = trinex_bit(3),
			Metadata  = trinex_bit(4),
			Any       = Created | Removed | Modified | Renamed | Metadata,
		};

		trinex_bitfield_enum_struct(FileWatchEventType, u32);
	};

	struct FileWatchEvent {
		FileWatchEventType type = FileWatchEventType::Undefined;
		Path path;
		Path old_path;
		bool is_directory = false;
	};

	using FileWatchCallback = CallBack<void(const FileWatchEvent&)>;

	struct FileWatchNotification : public FileWatchEvent {
		Identifier watch_id = 0;
	};

	class trinex_api FileWatcherBackend
	{
	public:
		virtual ~FileWatcherBackend() = default;

		virtual bool watch(Identifier watch_id, const Path& virtual_path, const Path& native_path, bool recursive) = 0;
		virtual void unwatch(Identifier watch_id)                                                                  = 0;
		virtual void poll(Array<FileWatchNotification>& events)                                                   = 0;
	};
}// namespace Trinex::VFS
