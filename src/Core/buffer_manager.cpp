#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <cstring>

namespace Engine
{
	size_t BufferWriter::size()
	{
		if (!is_open())
			return 0;

		auto current_pos = position();
		offset(0, BufferSeekDir::End);
		size_t size = position();
		position(current_pos);
		return size;
	}

	BufferWriter& BufferWriter::position(WritePos pos)
	{
		return offset(pos, BufferSeekDir::Begin);
	}

	size_t BufferReader::size()
	{
		if (!is_open())
			return 0;

		auto current_pos = position();
		offset(0, BufferSeekDir::End);
		size_t size = position();
		position(current_pos);
		return size;
	}

	BufferReader& BufferReader::position(ReadPos pos)
	{
		return offset(pos, BufferSeekDir::Begin);
	}


	void VectorWriterBase::copy_data(byte* to, const byte* from, size_t count)
	{
		std::memcpy(to, from, count);
	}

	void VectorReaderBase::copy_data(byte* to, const byte* from, size_t count)
	{
		std::memcpy(to, from, count);
	}
}// namespace Engine
