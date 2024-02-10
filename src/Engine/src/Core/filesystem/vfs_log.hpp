#pragma once
#include <Core/logger.hpp>


#define vfs_log(...) info_log("VFS", __VA_ARGS__)
#define vfs_error(...) error_log("VFS", __VA_ARGS__)
#define vfs_warning(...) warning_log("VFS", __VA_ARGS__)
#define vfs_debug(...) debug_log("VFS", __VA_ARGS__)
