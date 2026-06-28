#include <Core/etl/map.hpp>
#include <Core/filesystem/file_watcher.hpp>
#include <Core/memory.hpp>
#include <Platform/platform.hpp>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <sys/inotify.h>
#include <unistd.h>

namespace Trinex::Platform
{
	namespace
	{
		namespace fs = std::filesystem;

		static constexpr u32 watch_mask = IN_ATTRIB | IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM |
		                                  IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;

		static bool is_path_inside(const Path& path, const Path& root)
		{
			if (root.empty())
				return true;

			if (!path.starts_with(root))
				return false;

			if (path.length() == root.length())
				return true;

			if (root.str().back() == Path::separator)
				return true;

			return path.str()[root.length()] == Path::separator;
		}

		static Path join_native_path(const Path& base, const char* name)
		{
			if (name && *name)
				return base / Path(name);
			return base;
		}

		class LinuxFileWatcher final : public VFS::FileWatcherBackend
		{
		private:
			struct PendingMove {
				Path path;
				bool is_directory = false;
				u8 pending_polls  = 0;
			};

			struct WatchContext {
				Identifier id = 0;
				int fd        = -1;
				bool recursive;
				bool exact_file;
				Path virtual_path;
				Path native_path;
				Path watch_root;
				TreeMap<int, Path> directories;
				Map<u32, PendingMove> pending_moves;
			};

			Vector<WatchContext> m_watches;

			static bool add_directory_watch(WatchContext& watch, const Path& directory)
			{
				const int wd = inotify_add_watch(watch.fd, directory.c_str(), watch_mask);

				if (wd < 0)
				{
					trinex_error(Log::FileSystem, "Failed to watch '%s': %s", directory.c_str(), std::strerror(errno));
					return false;
				}

				watch.directories[wd] = directory;
				return true;
			}

			static bool add_recursive_watches(WatchContext& watch, const Path& directory)
			{
				if (!add_directory_watch(watch, directory))
					return false;

				std::error_code code;

				for (const auto& entry : fs::recursive_directory_iterator(directory.str(), code))
				{
					if (code)
					{
						trinex_error(Log::FileSystem, "Failed to enumerate '%s': %s", directory.c_str(), code.message().c_str());
						return false;
					}

					if (entry.is_directory())
					{
						if (!add_directory_watch(watch, Path(entry.path().string())))
							return false;
					}
				}

				return true;
			}

			static Path to_virtual_path(const WatchContext& watch, const Path& native)
			{
				if (watch.exact_file)
					return watch.virtual_path;

				return watch.virtual_path / native.relative(watch.watch_root);
			}

			static bool matches_exact_file(const WatchContext& watch, const Path& native) { return native == watch.native_path; }

			static void emit_exact_file_event(Vector<VFS::FileWatchNotification>& events, const WatchContext& watch,
			                                  VFS::FileWatchEventType type, bool is_directory)
			{
				VFS::FileWatchNotification event;
				event.watch_id     = watch.id;
				event.type         = type;
				event.path         = watch.virtual_path;
				event.is_directory = is_directory;
				events.push_back(std::move(event));
			}

			static void emit_path_event(Vector<VFS::FileWatchNotification>& events, const WatchContext& watch, const Path& native,
			                            VFS::FileWatchEventType type, bool is_directory)
			{
				if (!watch.exact_file && !is_path_inside(native, watch.watch_root))
					return;

				if (watch.exact_file)
				{
					if (matches_exact_file(watch, native))
						emit_exact_file_event(events, watch, type, is_directory);
					return;
				}

				VFS::FileWatchNotification event;
				event.watch_id     = watch.id;
				event.type         = type;
				event.path         = to_virtual_path(watch, native);
				event.is_directory = is_directory;
				events.push_back(std::move(event));
			}

			static void emit_rename_event(Vector<VFS::FileWatchNotification>& events, const WatchContext& watch,
			                              const Path& old_native, const Path& new_native, bool is_directory)
			{
				if (watch.exact_file)
				{
					if (matches_exact_file(watch, old_native) || matches_exact_file(watch, new_native))
					{
						VFS::FileWatchNotification event;
						event.watch_id     = watch.id;
						event.type         = VFS::FileWatchEventType::Renamed;
						event.path         = watch.virtual_path;
						event.old_path     = watch.virtual_path;
						event.is_directory = is_directory;
						events.push_back(std::move(event));
					}
					return;
				}

				if (!is_path_inside(old_native, watch.watch_root) && !is_path_inside(new_native, watch.watch_root))
					return;

				VFS::FileWatchNotification event;
				event.watch_id     = watch.id;
				event.type         = VFS::FileWatchEventType::Renamed;
				event.path         = to_virtual_path(watch, new_native);
				event.old_path     = to_virtual_path(watch, old_native);
				event.is_directory = is_directory;
				events.push_back(std::move(event));
			}

			static void flush_stale_moves(WatchContext& watch, Vector<VFS::FileWatchNotification>& events)
			{
				Vector<u32> stale_cookies;

				for (auto& [cookie, move] : watch.pending_moves)
				{
					if (++move.pending_polls > 1)
					{
						emit_path_event(events, watch, move.path, VFS::FileWatchEventType::Removed, move.is_directory);
						stale_cookies.push_back(cookie);
					}
				}

				for (u32 cookie : stale_cookies)
				{
					watch.pending_moves.erase(cookie);
				}
			}

			void poll_watch(WatchContext& watch, Vector<VFS::FileWatchNotification>& events)
			{
				alignas(inotify_event) char buffer[4096];

				while (true)
				{
					const ssize_t size = ::read(watch.fd, buffer, sizeof(buffer));

					if (size <= 0)
					{
						if (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
						{
							trinex_error(Log::FileSystem, "Failed to read watcher events: %s", std::strerror(errno));
						}
						break;
					}

					ssize_t offset = 0;

					while (offset < size)
					{
						auto* event = reinterpret_cast<inotify_event*>(buffer + offset);
						offset += sizeof(inotify_event) + event->len;

						auto found_dir = watch.directories.find(event->wd);
						if (found_dir == watch.directories.end())
							continue;

						const Path directory = found_dir->second;
						const Path native    = join_native_path(directory, event->name);
						const bool is_dir    = (event->mask & IN_ISDIR) == IN_ISDIR;

						if ((event->mask & IN_IGNORED) == IN_IGNORED)
						{
							watch.directories.erase(found_dir);
							continue;
						}

						if ((event->mask & IN_MOVED_FROM) == IN_MOVED_FROM)
						{
							watch.pending_moves[event->cookie] = PendingMove{native, is_dir};
							continue;
						}

						if ((event->mask & IN_MOVED_TO) == IN_MOVED_TO)
						{
							if (watch.recursive && is_dir && fs::is_directory(native.str()))
							{
								add_recursive_watches(watch, native);
							}

							if (auto found_move = watch.pending_moves.find(event->cookie);
							    found_move != watch.pending_moves.end())
							{
								emit_rename_event(events, watch, found_move->second.path, native,
								                  found_move->second.is_directory);
								watch.pending_moves.erase(found_move);
							}
							else
							{
								emit_path_event(events, watch, native, VFS::FileWatchEventType::Created, is_dir);
							}
							continue;
						}

						if ((event->mask & IN_CREATE) == IN_CREATE)
						{
							if (watch.recursive && is_dir && fs::is_directory(native.str()))
							{
								add_recursive_watches(watch, native);
							}

							emit_path_event(events, watch, native, VFS::FileWatchEventType::Created, is_dir);
						}

						if ((event->mask & IN_DELETE) == IN_DELETE || (event->mask & IN_DELETE_SELF) == IN_DELETE_SELF ||
						    (event->mask & IN_MOVE_SELF) == IN_MOVE_SELF)
						{
							emit_path_event(events, watch, native, VFS::FileWatchEventType::Removed, is_dir);
						}

						if ((event->mask & IN_ATTRIB) == IN_ATTRIB)
						{
							emit_path_event(events, watch, native, VFS::FileWatchEventType::Metadata, is_dir);
						}

						if ((event->mask & IN_MODIFY) == IN_MODIFY || (event->mask & IN_CLOSE_WRITE) == IN_CLOSE_WRITE)
						{
							emit_path_event(events, watch, native, VFS::FileWatchEventType::Modified, is_dir);
						}
					}
				}

				flush_stale_moves(watch, events);
			}

			void close_watch(WatchContext& watch)
			{
				if (watch.fd >= 0)
				{
					::close(watch.fd);
					watch.fd = -1;
				}
			}

		public:
			bool watch(Identifier watch_id, const Path& virtual_path, const Path& native_path, bool recursive) override
			{
				WatchContext watch;
				watch.id           = watch_id;
				watch.virtual_path = virtual_path;
				watch.native_path  = native_path;
				watch.recursive    = recursive;

				std::error_code code;
				const bool is_directory = fs::is_directory(native_path.str(), code);
				watch.exact_file        = !is_directory && !recursive;

				if (watch.exact_file)
				{
					auto parent      = fs::path(native_path.str()).parent_path();
					watch.watch_root = parent.empty() ? Path(".") : Path(parent.string());
				}
				else
				{
					watch.watch_root = native_path;
				}

				if (!fs::exists(watch.watch_root.str()))
				{
					trinex_warning(Log::FileSystem, "Watch root '%s' does not exist", watch.watch_root.c_str());
					return false;
				}

				watch.fd = inotify_init1(IN_NONBLOCK);
				if (watch.fd < 0)
				{
					trinex_error(Log::FileSystem, "Failed to initialize inotify: %s", std::strerror(errno));
					return false;
				}

				const bool success =
				        recursive ? add_recursive_watches(watch, watch.watch_root) : add_directory_watch(watch, watch.watch_root);
				if (!success)
				{
					close_watch(watch);
					return false;
				}

				m_watches.push_back(std::move(watch));
				return true;
			}

			void unwatch(Identifier watch_id) override
			{
				for (auto it = m_watches.begin(); it != m_watches.end(); ++it)
				{
					if (it->id == watch_id)
					{
						close_watch(*it);
						m_watches.erase(it);
						break;
					}
				}
			}

			void poll(Vector<VFS::FileWatchNotification>& events) override
			{
				for (auto& watch : m_watches)
				{
					poll_watch(watch, events);
				}
			}

			~LinuxFileWatcher() override
			{
				for (auto& watch : m_watches)
				{
					close_watch(watch);
				}
			}
		};
	}// namespace

	ENGINE_EXPORT VFS::FileWatcherBackend* create_file_watcher()
	{
		return trx_new LinuxFileWatcher();
	}
}// namespace Trinex::Platform
