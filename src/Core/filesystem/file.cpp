#include <Core/filesystem/file.hpp>

namespace Engine::VFS
{
	size_t File::size()
	{
		FilePosition pos = read_position();
		read_position(0, FileSeekDir::End);
		FilePosition size = read_position();
		read_position(static_cast<FileOffset>(pos), FileSeekDir::Begin);
		return size;
	}
}// namespace Engine::VFS
