#pragma once
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Platform/enums.hpp>
#include <Platform/object.hpp>

namespace Trinex::Platform
{
	struct FileWatchDesc {
		Path path;
		FileWatchEventType event_mask = FileWatchEventType::Any;
		bool recursive                = false;
	};

	struct FileWatchEvent {
		FileWatchEventType type = FileWatchEventType::Undefined;
		Identifier watch_id     = 0;
		Path path;
		Path old_path;
		bool is_directory = false;
	};

	class ENGINE_EXPORT FileWatch : public Object
	{
	public:
		virtual Identifier id() const    = 0;
		virtual const Path* path() const = 0;
	};

	class ENGINE_EXPORT FileWatcher
	{
	public:
		static FileWatcher* instance();

		virtual ~FileWatcher() = default;

		virtual FileWatch* watch(const FileWatchDesc* desc)       = 0;
		virtual FileWatcher* unwatch(FileWatch* watch)            = 0;
		virtual FileWatcher* poll(Vector<FileWatchEvent>* events) = 0;
	};
}// namespace Trinex::Platform
