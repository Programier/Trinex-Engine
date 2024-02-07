#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static Engine::Map<Engine::String, void*> _M_libraries;


namespace Engine
{

// Platform dependent code
#if PLATFORM_WINDOWS
    static const String format = ".dll";

    static void* platform_load_library(const String& name)
    {
        return (void*) LoadLibrary((LPCSTR) name.c_str());
    }

    static void* platform_load_func(void* handle, const char* name)
    {
        return (void*) GetProcAddress((HMODULE) (handle), (LPCSTR) name);
    }

    static void platform_close_lib(void* handle)
    {
        FreeLibrary((HMODULE) handle);
    }

#elif PLATFORM_ANDROID || PLATFORM_LINUX
    static const String format = ".so";

    static void* platform_load_library(const String& name)
    {
        return dlopen(name.empty() ? nullptr : name.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    }

    static void* platform_load_func(void* handle, const char* name)
    {
        return dlsym(handle, name);
    }

    static void platform_close_lib(void* handle)
    {
        dlclose(handle);
    }

#endif


    // Platform dependent code end


    Library::Library(const String& libname)
    {
        load(libname);
    }

    default_copy_constructors_cpp(Library);
    Library::Library() = default;


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
            Path new_path = engine_config.libraries_dir / path;
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

    void* Library::load_function(void* handle, const String& name)
    {
        void* func = platform_load_func(handle, name.c_str());
        if (func == nullptr)
            error_log("Library", "Failed to load function %s from lib %s\n", name.c_str(), _M_libname.c_str());
        return func;
    }


    static void validate_path(Path& path)
    {
        Path base = path.filename();
        if (!base.str().starts_with("lib"))
        {
            base = "lib" + base.str();
        }

        if (!base.str().ends_with(format))
        {
            base += format;
        }

        path = Path(path.base_path()) / base;
    }

    Library& Library::load(const String& libname)
    {
        close();
        Path libnames[3] =
#if TRINEX_DEBUG_BUILD
                {get_libname(libname, Global), get_libname(libname, Engine), get_libname(libname, None)};
#else
                {get_libname(libname, None), get_libname(libname, Engine), get_libname(libname, Global)};
#endif

        bool is_new_load = false;
        for (auto& path : libnames)
        {
            validate_path(path);

            void*& lib = _M_libraries[path.str()];
            if (!lib)
            {
                lib         = platform_load_library(path.str());
                is_new_load = static_cast<bool>(lib);
            }

            if (lib)
            {
                _M_handle  = lib;
                _M_libname = path.str();
                break;
            }
            else
            {
                _M_libraries.erase(path.str());
            }
        }

        if (!_M_handle)
        {
#ifdef WIN32
            error_log("Library", "Failed to load %s\n", libname.c_str());
#else
            error_log("Library", "%s\n", dlerror());
#endif
        }
        else if (is_new_load)
        {
            const Flags<EngineInstance::Flag>& flags = engine_instance->flags();

            if (flags(EngineInstance::PreInitTriggered))
            {
                PreInitializeController().execute();
            }

            if (flags(EngineInstance::InitTriggered))
            {
                InitializeController().execute();
            }

            if (flags(EngineInstance::ClassInitTriggered))
            {
                ClassInitializeController().execute();
            }

            if (flags(EngineInstance::PostInitTriggered))
            {
                PostInitializeController().execute();
            }
        }

        return *this;
    }

    static void close_lib_ptr(void*& lib)
    {
        if (lib)
        {
            platform_close_lib(lib);
            lib = nullptr;
        }
    }

    void Library::close()
    {
        if (_M_handle)
        {
            auto it = _M_libraries.find(_M_libname);
            if (it == _M_libraries.end())
            {
                throw EngineException("Unexpected error!");
            }

            if (it->second == _M_handle)
            {
                info_log("Library", "Close library: '%s'", _M_libname.c_str());
                close_lib_ptr(_M_handle);
            }
            else
            {
                throw EngineException("Unexpected error!");
            }

            _M_libraries.erase(_M_libname);
        }
    }


    bool Library::has_lib() const
    {
        return _M_handle != nullptr;
    }

    const String& Library::libname() const
    {
        return _M_libname;
    }

    ENGINE_EXPORT void close(const String& libname)
    {}

    Library::operator bool() const
    {
        return _M_handle != nullptr;
    }

    void* Library::resolve(const String& name)
    {
        return load_function(_M_handle, name);
    }

    void Library::close_all()
    {
        info_log("LibrariesController", "Closing all opened libs\n");
        for (auto& ell : _M_libraries)
        {
            info_log("LibrariesController", "Close library: '%s'", ell.first.c_str());
            close_lib_ptr(ell.second);
        }
    }
}// namespace Engine
