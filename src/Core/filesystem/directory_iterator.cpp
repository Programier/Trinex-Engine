#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/filesystem/root_filesystem.hpp>


namespace Engine::VFS
{
	DirectoryIterator::DirectoryIterator() : m_interface(nullptr) {}

	DirectoryIterator::DirectoryIterator(const Path& path) : m_interface(rootfs()->create_directory_iterator(path)) {}

	DirectoryIterator::DirectoryIterator(const DirectoryIterator& other)
	    : m_interface(other.m_interface ? other.m_interface->copy() : nullptr)
	{}

	DirectoryIterator::DirectoryIterator(DirectoryIterator&& other) : m_interface(other.m_interface)
	{
		other.m_interface = nullptr;
	}

	DirectoryIterator& DirectoryIterator::operator=(const DirectoryIterator& other)
	{
		if (this == &other)
			return *this;

		if (m_interface)
			delete m_interface;

		m_interface = other.m_interface ? other.m_interface->copy() : nullptr;
		return *this;
	}

	DirectoryIterator& DirectoryIterator::operator=(DirectoryIterator&& other)
	{
		if (this == &other)
			return *this;

		if (m_interface)
			delete m_interface;

		m_interface       = other.m_interface;
		other.m_interface = nullptr;
		return *this;
	}

	DirectoryIterator& DirectoryIterator::begin()
	{
		return *this;
	}

	DirectoryIterator& DirectoryIterator::end()
	{
		static DirectoryIterator m_end;
		return m_end;
	}

	DirectoryIterator& DirectoryIterator::operator++()
	{
		if (m_interface)
		{
			m_interface->next();
		}
		return *this;
	}

	const Path& DirectoryIterator::operator*()
	{
		static const Path default_path;
		return m_interface && m_interface->is_valid() ? m_interface->path() : default_path;
	}

	bool DirectoryIterator::operator!=(const DirectoryIterator& other) const
	{
		return !((*this) == other);
	}

	bool DirectoryIterator::operator==(const DirectoryIterator& other) const
	{
		if (m_interface == other.m_interface)
			return true;

		if (m_interface == nullptr)
			return !other.m_interface->is_valid();

		if (other.m_interface == nullptr)
			return !m_interface->is_valid();

		if (m_interface->id() != other.m_interface->id())
			return false;

		return m_interface->is_equal(other.m_interface);
	}


	RecursiveDirectoryIterator::RecursiveDirectoryIterator() : m_interface(nullptr) {}

	RecursiveDirectoryIterator::RecursiveDirectoryIterator(const Path& path)
	    : m_interface(rootfs()->create_recursive_directory_iterator(path))
	{}

	RecursiveDirectoryIterator::RecursiveDirectoryIterator(const RecursiveDirectoryIterator& other)
	    : m_interface(other.m_interface ? other.m_interface->copy() : nullptr)
	{}

	RecursiveDirectoryIterator::RecursiveDirectoryIterator(RecursiveDirectoryIterator&& other) : m_interface(other.m_interface)
	{
		other.m_interface = nullptr;
	}

	RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator=(const RecursiveDirectoryIterator& other)
	{
		if (this == &other)
			return *this;

		if (m_interface)
			delete m_interface;

		m_interface = other.m_interface ? other.m_interface->copy() : nullptr;
		return *this;
	}

	RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator=(RecursiveDirectoryIterator&& other)
	{
		if (this == &other)
			return *this;

		if (m_interface)
			delete m_interface;

		m_interface       = other.m_interface;
		other.m_interface = nullptr;
		return *this;
	}

	RecursiveDirectoryIterator& RecursiveDirectoryIterator::begin()
	{
		return *this;
	}

	RecursiveDirectoryIterator& RecursiveDirectoryIterator::end()
	{
		static RecursiveDirectoryIterator m_end;
		return m_end;
	}

	RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator++()
	{
		if (m_interface)
		{
			m_interface->next();
		}
		return *this;
	}

	const Path& RecursiveDirectoryIterator::operator*()
	{
		static const Path default_path;
		return m_interface && m_interface->is_valid() ? m_interface->path() : default_path;
	}

	bool RecursiveDirectoryIterator::operator!=(const RecursiveDirectoryIterator& other) const
	{
		return !((*this) == other);
	}

	bool RecursiveDirectoryIterator::operator==(const RecursiveDirectoryIterator& other) const
	{
		if (m_interface == other.m_interface)
			return true;

		if (m_interface == nullptr)
			return !other.m_interface->is_valid();

		if (other.m_interface == nullptr)
			return !m_interface->is_valid();

		if (m_interface->id() != other.m_interface->id())
			return false;

		return m_interface->is_equal(other.m_interface);
	}
}// namespace Engine::VFS
