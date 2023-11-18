#include <Core/memory.hpp>
#include <Core/ring_buffer.hpp>
#include <Core/thread.hpp>

namespace Engine
{
    RingBuffer::AllocationContext::AllocationContext(RingBuffer& buffer, size_t size) : _M_buffer(buffer)
    {
        _M_buffer._M_mutex.lock();

        size = align_memory(size, _M_buffer._M_alignment);

        if (buffer.size() < size)
            throw EngineException("Cannot allocate new memory in ring buffer");

        while (buffer.free_size() < size)
        {
            ThreadBase::sleep_for(0.03);
        }

        _M_start_pointer = buffer._M_write_pointer;
        _M_size          = glm::min<size_t>(size, (buffer._M_end_pointer - buffer._M_write_pointer));
    }

    void* RingBuffer::AllocationContext::data() const
    {
        return _M_start_pointer;
    }

    RingBuffer::AllocationContext& RingBuffer::AllocationContext::submit()
    {
        if (_M_start_pointer)
        {
            _M_buffer._M_write_pointer += _M_size;
            if (_M_buffer._M_write_pointer >= _M_buffer._M_end_pointer)
                _M_buffer._M_write_pointer = _M_buffer._M_start_pointer;

            _M_start_pointer = nullptr;
            _M_buffer._M_unreaded_size += _M_size;
            _M_size = 0;

            if (_M_buffer._M_event)
            {
                _M_buffer._M_event->execute();
            }

            _M_buffer._M_mutex.unlock();
        }
        return *this;
    }

    RingBuffer::AllocationContext::~AllocationContext()
    {
        submit();
    }

    size_t RingBuffer::AllocationContext::size() const
    {
        return _M_size;
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
            size         = align_memory(size, alignment);
            _M_alignment = alignment;
            _M_event     = event;

            _M_data.resize(size, 0);
            _M_read_pointer  = _M_data.data();
            _M_write_pointer = _M_read_pointer.load();
            _M_start_pointer = _M_read_pointer;
            _M_end_pointer   = _M_read_pointer + size;
            _M_unreaded_size = 0;
        }

        return *this;
    }

    bool RingBuffer::is_inited() const
    {
        return !_M_data.empty();
    }

    size_t RingBuffer::unreaded_buffer_size() const
    {
        return _M_unreaded_size;
    }

    size_t RingBuffer::free_size() const
    {
        return size() - unreaded_buffer_size();
    }

    size_t RingBuffer::size() const
    {
        return _M_data.size();
    }

    void* RingBuffer::reading_pointer()
    {
        if (unreaded_buffer_size() == 0)
            return nullptr;
        return _M_read_pointer;
    }

    RingBuffer& RingBuffer::finish_read(size_t size)
    {
        size = glm::min(align_memory(size, _M_alignment), _M_unreaded_size.load());
        _M_read_pointer += size;

        if (_M_read_pointer >= _M_end_pointer)
            _M_read_pointer = _M_start_pointer;

        _M_unreaded_size -= size;
        return *this;
    }
}// namespace Engine
