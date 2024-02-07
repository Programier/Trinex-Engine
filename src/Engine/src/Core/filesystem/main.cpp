#include <Core/engine_loading_controllers.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/native_file_system.hpp>
#include <Core/filesystem/root_filesystem.hpp>


namespace Engine::VFS
{
    static void on_init()
    {
        for (auto& file : RecursiveDirectoryIterator(""))
        {
            printf("%s\n", file.c_str());
        }
    }

    static PreInitializeController on(on_init);
}// namespace Engine::VFS
