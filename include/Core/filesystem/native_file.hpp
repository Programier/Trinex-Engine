#pragma once
#include <Core/filesystem/file.hpp>
#include <fstream>

namespace Engine::VFS
{
	class NativeFileSystem;
	class ENGINE_EXPORT NativeFile : public File
	{
	private:
		NativeFileSystem* m_fs;
		Path m_path;
		std::fstream m_stream;

	public:
		NativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream);
		~NativeFile();

		trinex_non_copyable(NativeFile);
		trinex_non_moveable(NativeFile);

		const Path& path() const;

		FileSystem* filesystem() const override;
		FilePosition wseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition wpos() override;
		FilePosition rseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition rpos() override;
		size_t read(byte* buffer, size_t size) override;
		size_t write(const byte* buffer, size_t size) override;

		friend class NativeFileSystem;
	};

	class ENGINE_EXPORT ReadOnlyNativeFile : public NativeFile
	{
	public:
		ReadOnlyNativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream);

		trinex_non_copyable(ReadOnlyNativeFile);
		trinex_non_moveable(ReadOnlyNativeFile);

		FilePosition wseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition wpos() override;
		size_t write(const byte* buffer, size_t size) override;
	};

	class ENGINE_EXPORT WriteOnlyNativeFile : public NativeFile
	{
	public:
		WriteOnlyNativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream);

		trinex_non_copyable(WriteOnlyNativeFile);
		trinex_non_moveable(WriteOnlyNativeFile);

		FilePosition rseek(FileOffset offset, FileSeekDir dir) override;
		FilePosition rpos() override;
		size_t read(byte* buffer, size_t size) override;
	};
}// namespace Engine::VFS
