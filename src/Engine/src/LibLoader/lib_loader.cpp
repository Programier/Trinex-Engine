#include <LibLoader/lib_loader.hpp>
#include <SDL_log.h>
#include <iostream>
#include <unordered_map>
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static std::unordered_map<std::string, void*> _M_libraries;
#define str std::string

namespace Engine
{
    std::string library_dir;

    static std::string get_full_libname(const std::string& libname)
    {
#ifdef WIN32
        static const std::string format = ".dll";
#else
        static const std::string format = ".so";
#endif
        return library_dir + str("lib") + libname + format;
    }

    Library::Library(void* handle, const std::string& libname)
    {
        _M_handle = handle;
        _M_libname = libname;
    }

    void* Library::load_function(void* handle, const std::string& name)
    {
#ifdef WIN32
        return (void*) GetProcAddress((HMODULE) (handle), (LPCSTR) (name.c_str()));
#else
        return dlsym(handle, name.c_str());
#endif
    }

    void Library::close()
    {
        close_library(_M_libname);
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


    Library load_library(const std::string& libname)
    {
        std::string fullname = get_full_libname(libname);
        void*& lib = _M_libraries[fullname];
        if (!lib)
        {
#ifdef WIN32
            void* handle = (void*) LoadLibrary((LPCSTR) fullname.c_str());
            if (!handle)
                SDL_Log("Failed to load %s\n", fullname.c_str());
#else
            void* handle = dlopen(fullname.c_str(), RTLD_LAZY);
            if (!handle)
                SDL_Log("%s\n", dlerror());
#endif
            lib = handle;
            return Library(handle, libname);
        }

        return Library(lib, libname);
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

    void close_library(const std::string& libname)
    {
        std::string fullname = get_full_libname(libname);
        close_lib_ptr(_M_libraries[fullname]);
    }

    static struct Controller {
        ~Controller()
        {
            SDL_Log("LibrariesController: Closing all opened libs\n");
            for (auto& ell : _M_libraries) close_lib_ptr(ell.second);
        }
    } controller;

}// namespace Engine
