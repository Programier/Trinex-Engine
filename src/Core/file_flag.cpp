#include <Core/archive.hpp>
#include <Core/file_flag.hpp>


namespace Engine
{
	static constexpr size_t make_flag_from_string(const char* string)
	{
		union
		{
			size_t result = 0;
			char data[8];
		};

		size_t count = 0;

		while (count < 8 && *string)
		{
			data[count] = *string;
			++string;
			++count;
		}

		return result;
	}


	FileFlag::FileFlag(size_t first, size_t second)
	{
		data[0] = first;
		data[1] = second;
	}

	bool FileFlag::operator==(const FileFlag& other) const
	{
		return data[0] == other.data[0] && data[1] == other.data[1];
	}

	bool FileFlag::operator!=(const FileFlag& other) const
	{
		return !(*this == other);
	}

	const FileFlag& FileFlag::package_flag()
	{
		static FileFlag flag(make_flag_from_string("TRINEX"), make_flag_from_string("PACKAGE"));
		return flag;
	}

	const FileFlag& FileFlag::asset_flag()
	{
		static FileFlag flag(make_flag_from_string("TRINEX"), make_flag_from_string("ASSET"));
		return flag;
	}
}// namespace Engine
