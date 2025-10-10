#pragma once
#include <Core/filesystem/file.hpp>
#include <fstream>

namespace Engine::VFS
{
	class CommonFileSystem;

	class ENGINE_EXPORT CommonFile : public File
	{
	private:
		CommonFileSystem* m_fs;
		Path m_path;
		std::fstream m_stream;

	public:
		CommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream);
		~CommonFile();

		trinex_non_copyable(CommonFile);
		trinex_non_moveable(CommonFile);

		const Path& path() const;

		FileSystem* filesystem() const override;
		FilePosition wseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition wpos() override;
		FilePosition rseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition rpos() override;
		size_t read(byte* buffer, size_t size) override;
		size_t write(const byte* buffer, size_t size) override;

		friend class CommonFileSystem;
	};

	class ENGINE_EXPORT ReadOnlyCommonFile : public CommonFile
	{
	public:
		ReadOnlyCommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream);

		trinex_non_copyable(ReadOnlyCommonFile);
		trinex_non_moveable(ReadOnlyCommonFile);

		FilePosition wseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition wpos() override;
		size_t write(const byte* buffer, size_t size) override;
	};

	class ENGINE_EXPORT WriteOnlyCommonFile : public CommonFile
	{
	public:
		WriteOnlyCommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream);

		trinex_non_copyable(WriteOnlyCommonFile);
		trinex_non_moveable(WriteOnlyCommonFile);

		FilePosition rseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition rpos() override;
		size_t read(byte* buffer, size_t size) override;
	};
}// namespace Engine::VFS
