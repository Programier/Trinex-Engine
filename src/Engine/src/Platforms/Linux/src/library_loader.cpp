#include <Core/config_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Platform/platform.hpp>
#include <dlfcn.h>


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
            Path new_path = Path(ConfigManager::get_string("Engine::libraries_dir")) / path;
            auto entry    = rootfs()->find_filesystem(new_path);
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

        if (!base.str().ends_with(".so"))
        {
            base += ".so";
        }

        return Path(path.base_path()) / base;
    }

    static void prepare_env()
    {
        static String ld_library_path;
        if (!ld_library_path.empty())
            return;

        if (const char* path = getenv("LD_LIBRARY_PATH"))
        {
            ld_library_path = path;
            ld_library_path.push_back(':');
        }

        Path result = Platform::find_root_directory() / ConfigManager::get_path("Engine::libraries_dir");
        ld_library_path += result.path();

        setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), 1);
    }

    ENGINE_EXPORT void* load_library(const String& name)
    {
        prepare_env();

        Path name_path = validate_path(name);

        Path paths[] = {
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
            void* handle = dlopen(path.empty() ? nullptr : path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (handle)
            {
                return handle;
            }
            else
            {
                error_log("LinuxLibraryLoader", "%s", dlerror());
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
