#include <Core/filesystem/root_filesystem.hpp>
#include <Engine/project.hpp>
#include <Platform/platform.hpp>
#include <windows.h>


namespace Engine::Platform::LibraryLoader
{
	enum LibPathModification
	{
		None,
		Engine,
		Global,
	};

	static Path get_libname(const Path& path, LibPathModification mode)
	{
		if (path.empty())
			return path;

		if (mode == LibPathModification::None)
			return path;

		if (mode == Engine)
		{
			Path new_path = Path(Project::libraries_dir) / path;
			auto entry	  = rootfs()->find_filesystem(new_path);
			if (entry.first == nullptr || entry.first->type() != VFS::FileSystem::Type::Native)
				return path.filename();
			return entry.first->native_path(entry.second);
		}

		if (mode == LibPathModification::Global)
		{
			return path.filename();
		}

		return path;
	}


	static Path validate_path(const Path& path)
	{
		Path base = path.filename();
		if (!base.str().starts_with("lib"))
		{
			base = "lib" + base.str();
		}

		if (!base.str().ends_with(".dll"))
		{
			base += ".dll";
		}

		return Path(path.base_path()) / base;
	}

	ENGINE_EXPORT void* load_library(const String& name)
	{
		Path name_path = validate_path(name);
		Path paths[]   = {
#if TRINEX_DEBUG_BUILD
			get_libname(name_path, Global),
			get_libname(name_path, Engine),
			get_libname(name_path, None)
#else
			get_libname(name_path, None),
			get_libname(name_path, Engine),
			get_libname(name_path, Global)
#endif
		};

		for (auto& path : paths)
		{
			void* handle = (void*) LoadLibrary((LPCSTR) path.c_str());
			if (handle)
			{
				return handle;
			}
		}

		return nullptr;
	}// namespace Engine::Platform::LibraryLoader

	ENGINE_EXPORT void close_library(void* handle)
	{
		if (handle)
		{
			FreeLibrary((HMODULE) handle);
		}
	}

	ENGINE_EXPORT void* find_function(void* handle, const String& name)
	{
		if (handle == nullptr)
			return nullptr;
		return (void*) GetProcAddress((HMODULE) (handle), (LPCSTR) name.c_str());
	}
}// namespace Engine::Platform::LibraryLoader
