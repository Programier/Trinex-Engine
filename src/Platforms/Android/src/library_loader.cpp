#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Engine/project.hpp>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <dlfcn.h>
#include <jni.h>


namespace Engine::Platform::LibraryLoader
{
	enum LibPathModification
	{
		None,
		Engine,
		Global,
		AppLocal,
	};


	static const String& get_native_library_dir()
	{
		return m_android_platform_info.libraries_path;
	}

	static Path get_app_lib_path()
	{
		static Path path = get_native_library_dir();
		info_log("AndroidLibraryLoader", "Application library path is '%s'", path.c_str());
		return path;
	}

	static Path get_libname(const Path& path, LibPathModification mode)
	{
		if (path.empty())
			return path;

		if (mode == LibPathModification::None)
			return path;

		if (mode == Engine)
		{
			Path new_path = Path(Project::libraries_dir) / path;
			auto entry    = rootfs()->find_filesystem(new_path);
			if (entry.first == nullptr || entry.first->type() != VFS::FileSystem::Type::Native)
				return path.filename();
			return entry.first->native_path(entry.second);
		}

		if (mode == LibPathModification::Global)
		{
			return path.filename();
		}

		return get_app_lib_path() / path;
	}

	static Path validate_path(const Path& path)
	{
		Path base = path.filename();
		if (!base.str().starts_with("lib"))
		{
			base = "lib" + base.str();
		}

		if (!base.str().ends_with(".so"))
		{
			base += ".so";
		}

		return Path(path.base_path()) / base;
	}


	ENGINE_EXPORT void* load_library(const String& name)
	{
		Path name_path = validate_path(name);

		Path paths[] = {
#if TRINEX_DEBUG_BUILD
		        get_libname(name_path, Global), get_libname(name_path, AppLocal), get_libname(name_path, Engine),
		        get_libname(name_path, None)
#else
		        get_libname(name_path, None), get_libname(name_path, Engine), get_libname(name_path, AppLocal),
		        get_libname(name_path, Global)
#endif
		};

		for (auto& path : paths)
		{
			void* handle = dlopen(path.empty() ? nullptr : path.c_str(), RTLD_GLOBAL);
			if (handle)
			{
				return handle;
			}
			else
			{
				const char* msg = dlerror();
				info_log("Message", "%s", msg);
			}
		}

		return nullptr;
	}// namespace Engine::Platform::LibraryLoader

	ENGINE_EXPORT void close_library(void* handle)
	{
		if (handle)
		{
			dlclose(handle);
		}
	}

	ENGINE_EXPORT void* find_function(void* handle, const String& name)
	{
		if (handle == nullptr)
			return nullptr;

		return dlsym(handle, name.c_str());
	}
}// namespace Engine::Platform::LibraryLoader
