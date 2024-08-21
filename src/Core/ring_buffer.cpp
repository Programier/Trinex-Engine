#include <Core/memory.hpp>
#include <Core/ring_buffer.hpp>
#include <Core/thread.hpp>

namespace Engine
{
	RingBuffer::AllocationContext::AllocationContext(RingBuffer& buffer, size_t size) : m_buffer(buffer)
	{
		m_buffer.m_mutex.lock();

		size = align_memory(size, m_buffer.m_alignment);

		if (buffer.size() < size)
			throw EngineException("Cannot allocate new memory in ring buffer");

		while (buffer.free_size() < size)
		{
			ThreadBase::sleep_for(0.03);
		}

		m_start_pointer = buffer.m_write_pointer;
		m_size          = glm::min<size_t>(size, (buffer.m_end_pointer - buffer.m_write_pointer));
	}

	void* RingBuffer::AllocationContext::data() const
	{
		return m_start_pointer;
	}

	RingBuffer::AllocationContext& RingBuffer::AllocationContext::submit()
	{
		if (m_start_pointer)
		{
			m_buffer.m_write_pointer += m_size;
			if (m_buffer.m_write_pointer >= m_buffer.m_end_pointer)
				m_buffer.m_write_pointer = m_buffer.m_start_pointer;

			m_start_pointer = nullptr;
			m_buffer.m_unreaded_size += m_size;
			m_size = 0;

			if (m_buffer.m_event)
			{
				m_buffer.m_event->execute();
			}

			m_buffer.m_mutex.unlock();
		}
		return *this;
	}

	RingBuffer::AllocationContext::~AllocationContext()
	{
		submit();
	}

	size_t RingBuffer::AllocationContext::size() const
	{
		return m_size;
	}

	RingBuffer::RingBuffer()
	{}

	RingBuffer::RingBuffer(size_t size, size_t alignment, class ExecutableObject* event)
	{
		init(size, alignment, event);
	}


	RingBuffer& RingBuffer::init(size_t size, size_t alignment, class ExecutableObject* event)
	{
		if (!is_inited())
		{
			size        = align_memory(size, alignment);
			m_alignment = alignment;
			m_event     = event;

			m_data.resize(size, 0);
			m_read_pointer  = m_data.data();
			m_write_pointer = m_read_pointer.load();
			m_start_pointer = m_read_pointer;
			m_end_pointer   = m_read_pointer + size;
			m_unreaded_size = 0;
		}

		return *this;
	}

	bool RingBuffer::is_inited() const
	{
		return !m_data.empty();
	}

	size_t RingBuffer::unreaded_buffer_size() const
	{
		return m_unreaded_size;
	}

	size_t RingBuffer::free_size() const
	{
		return size() - unreaded_buffer_size();
	}

	size_t RingBuffer::size() const
	{
		return m_data.size();
	}

	void* RingBuffer::reading_pointer()
	{
		if (unreaded_buffer_size() == 0)
			return nullptr;
		return m_read_pointer;
	}

	RingBuffer& RingBuffer::finish_read(size_t size)
	{
		size = glm::min(align_memory(size, m_alignment), m_unreaded_size.load());
		m_read_pointer += size;

		if (m_read_pointer >= m_end_pointer)
			m_read_pointer = m_start_pointer;

		m_unreaded_size -= size;
		return *this;
	}
}// namespace Engine
