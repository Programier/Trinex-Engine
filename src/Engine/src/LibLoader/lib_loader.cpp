#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <LibLoader/lib_loader.hpp>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static Engine::Map<std::string, void*> _M_libraries;
#define str std::string

namespace Engine::LibraryLoader
{
    ENGINE_EXPORT std::string library_dir;

    default_copy_constructors_cpp(Library);
    Library::Library() = default;


    static std::string get_libname(const std::string& libname, bool full = true)
    {
        if (libname.empty())
            return libname;

#ifdef WIN32
        static const std::string format = ".dll";
#else
        static const std::string format = ".so";
#endif
        if (full)
            return library_dir + str("/lib") + libname + format;
        return str("lib") + libname + format;
    }

    Library::Library(void* handle, const std::string& libname)
    {
        _M_handle  = handle;
        _M_libname = libname;
    }

    void* Library::load_function(void* handle, const std::string& name)
    {
#ifdef WIN32
        void* func = (void*) GetProcAddress((HMODULE) (handle), (LPCSTR) (name.c_str()));
#else
        void* func                      = dlsym(handle, name.c_str());
#endif
        if (func == nullptr)
            info_log("Library", "Failed to load function %s from lib %s\n", name.c_str(), _M_libname.c_str());
        return func;
    }

    void Library::close()
    {
        Engine::LibraryLoader::close(_M_libname);
        _M_handle = nullptr;
    }

    bool Library::has_lib() const
    {
        return _M_handle != nullptr;
    }

    const std::string& Library::libname() const
    {
        return _M_libname;
    }


    ENGINE_EXPORT Library load(const std::string& libname)
    {
        std::string libnames[2] = {get_libname(libname), get_libname(libname, false)};

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
                return Library(lib, libname);
        }


#ifdef WIN32
        info_log("Library", "Failed to load %s\n", libname.c_str());
#else
        info_log("Library", "%s\n", dlerror());
#endif
        return Library(nullptr, libname);
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

    ENGINE_EXPORT void close(const std::string& libname)
    {
        std::string names[2] = {get_libname(libname), get_libname(libname, false)};
        for (auto& name : names)
        {
            void*& lib = _M_libraries[name];
            if (lib)
                close_lib_ptr(lib);
        }
    }

    Library::operator bool() const
    {
        return _M_handle != nullptr;
    }

    void* Library::resolve(const std::string& name)
    {
        return load_function(_M_handle, name);
    }

    static struct Controller {
        ~Controller()
        {
            info_log("LibrariesController", "Closing all opened libs\n");
            for (auto& ell : _M_libraries) close_lib_ptr(ell.second);
        }
    } controller;

}// namespace Engine::LibraryLoader
