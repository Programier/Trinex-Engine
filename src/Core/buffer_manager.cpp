#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <cstring>

namespace Trinex
{
	usize BufferWriter::size()
	{
		if (!is_open())
			return 0;

		auto current_pos = position();
		offset(0, BufferSeekDir::End);
		usize size = position();
		position(current_pos);
		return size;
	}

	BufferWriter& BufferWriter::position(WritePos pos)
	{
		return offset(pos, BufferSeekDir::Begin);
	}

	usize BufferReader::size()
	{
		if (!is_open())
			return 0;

		auto current_pos = position();
		offset(0, BufferSeekDir::End);
		usize size = position();
		position(current_pos);
		return size;
	}

	BufferReader& BufferReader::position(ReadPos pos)
	{
		return offset(pos, BufferSeekDir::Begin);
	}


	void VectorWriterBase::copy_data(u8* to, const u8* from, usize count)
	{
		std::memcpy(to, from, count);
	}

	void VectorReaderBase::copy_data(u8* to, const u8* from, usize count)
	{
		std::memcpy(to, from, count);
	}
}// namespace Trinex
