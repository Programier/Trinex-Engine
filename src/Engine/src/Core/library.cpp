#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static Engine::Map<Engine::String, void*> _M_libraries;
#define str String

namespace Engine
{
    Library::Library(const String& libname)
    {
        load(libname);
    }

    default_copy_constructors_cpp(Library);
    Library::Library() = default;


    static String get_libname(const String& libname, bool full = true)
    {
        if (libname.empty())
            return libname;

#ifdef WIN32
        static const String format = ".dll";
#else
        static const String format = ".so";
#endif
        if (full)
            return engine_config.libraries_dir + str("/lib") + libname + format;
        return str("lib") + libname + format;
    }

    void* Library::load_function(void* handle, const String& name)
    {
#ifdef WIN32
        void* func = (void*) GetProcAddress((HMODULE) (handle), (LPCSTR) (name.c_str()));
#else
        void* func                 = dlsym(handle, name.c_str());
#endif
        if (func == nullptr)
            error_log("Library", "Failed to load function %s from lib %s\n", name.c_str(), _M_libname.c_str());
        return func;
    }

    Library& Library::load(const String& libname)
    {
        close();
        String libnames[2] = {get_libname(libname, true), get_libname(libname, false)};

        for (auto& name : libnames)
        {
            void*& lib = _M_libraries[name];
            if (!lib)
            {
#ifdef WIN32
                lib = (void*) LoadLibrary((LPCSTR) name.c_str());
#else
                lib = dlopen(name.empty() ? nullptr : name.c_str(), RTLD_LAZY);
#endif
            }

            if (lib)
            {
                _M_handle  = lib;
                _M_libname = name;
                break;
            }
            else
            {
                _M_libraries.erase(name);
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
        return *this;
    }

    static void close_lib_ptr(void*& lib)
    {
        if (lib)
        {
#ifdef WIN32
            FreeLibrary((HMODULE) lib);
            lib = nullptr;
#else
            dlclose(lib);
            lib = nullptr;
#endif
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
