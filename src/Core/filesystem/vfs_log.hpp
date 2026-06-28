#pragma once
#define vfs_log(...) trinex_info(Trinex::Log::FileSystem, __VA_ARGS__)
#define vfs_error(...) trinex_error(Trinex::Log::FileSystem, __VA_ARGS__)
#define vfs_warning(...) trinex_warning(Trinex::Log::FileSystem, __VA_ARGS__)
#define vfs_debug(...) trinex_debug(Trinex::Log::FileSystem, __VA_ARGS__)
