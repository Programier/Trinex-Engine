#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/set.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Platform/platform.hpp>


namespace Engine
{
	struct LibInfo {
		void* handle = nullptr;
		String name;
	};

	struct Less {
		bool operator()(const LibInfo& __x, const LibInfo& __y) const
		{
			return __x.handle < __y.handle;
		}
	};


	static Engine::TreeSet<LibInfo, Less> m_libraries;
	Library::Library(const String& libname)
	{
		load(libname);
	}

	default_copy_constructors_cpp(Library);
	Library::Library() = default;


	void* Library::load_function(void* handle, const String& name)
	{
		void* func = Platform::LibraryLoader::find_function(handle, name);

		if (func == nullptr)
			error_log("Library", "Failed to load function %s from lib %s\n", name.c_str(), m_libname.c_str());

		return func;
	}

	Library& Library::load(const String& libname)
	{
		close();

		m_handle = Platform::LibraryLoader::load_library(libname);

		if (m_handle)
		{
			LibInfo info;
			info.handle = m_handle;
			info.name   = libname;
			m_libraries.insert(info);

			LoadingControllerBase::exec_all_if_already_triggered();
		}

		return *this;
	}

	void Library::close()
	{
		if (m_handle)
		{

			LibInfo info;
			info.handle = m_handle;

			auto it = m_libraries.find(info);

			if (it == m_libraries.end())
			{
				throw EngineException("Unexpected error!");
			}

			if (it->handle == m_handle)
			{
				m_libraries.erase(it);
				info_log("Library", "Close library: '%s'", m_libname.c_str());
				Platform::LibraryLoader::close_library(m_handle);
			}
			else
			{
				throw EngineException("Unexpected error!");
			}
		}
	}

	bool Library::has_lib() const
	{
		return m_handle != nullptr;
	}

	const String& Library::libname() const
	{
		return m_libname;
	}

	Library::operator bool() const
	{
		return m_handle != nullptr;
	}

	void* Library::resolve(const String& name)
	{
		return load_function(m_handle, name);
	}

	void Library::close_all()
	{
		info_log("LibrariesController", "Closing all opened libs\n");

		for (auto& ell : m_libraries)
		{
			info_log("LibrariesController", "Close library: '%s'", ell.name.c_str());
			Platform::LibraryLoader::close_library(ell.handle);
		}
		m_libraries.clear();
	}
}// namespace Engine
