#pragma once
#include <Core/filesystem/file.hpp>
#include <fstream>

namespace Engine::VFS
{
	class AndroidFileSystem;

	class ENGINE_EXPORT AndroidFile : public File
	{
	private:
		AndroidFileSystem* m_fs;
		Path m_path;
		std::fstream m_stream;

	public:
		AndroidFile(AndroidFileSystem* fs, const Path& path, std::fstream&& stream);
		~AndroidFile();

		trinex_non_copyable(AndroidFile);
		trinex_non_moveable(AndroidFile);

		const Path& path() const;

		FileSystem* filesystem() const override;
		FilePosition wseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition wpos() override;
		FilePosition rseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition rpos() override;
		size_t read(byte* buffer, size_t size) override;
		size_t write(const byte* buffer, size_t size) override;

		friend class AndroidFileSystem;
	};
}// namespace Engine::VFS
