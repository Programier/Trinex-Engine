#include <Core/ring_buffer.hpp>
#include <Core/thread.hpp>

namespace Engine
{
    RingBuffer::AllocationContext::AllocationContext(RingBuffer& buffer, size_t size) : _M_buffer(buffer)
    {
        _M_buffer._M_mutex.lock();

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
            _M_buffer._M_mutex.unlock();
            _M_size = 0;
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
    {

    }

    RingBuffer::RingBuffer(size_t size)
    {
        init(size);
    }


    RingBuffer& RingBuffer::init(size_t size)
    {
        if(!is_inited())
        {
            _M_data.resize(size, 0);
            _M_read_pointer  = _M_data.data();
            _M_write_pointer = _M_read_pointer.load();
            _M_start_pointer = _M_read_pointer;
            _M_end_pointer   = _M_read_pointer + size;
        }

        return *this;
    }

    bool RingBuffer::is_inited() const
    {
        return !_M_data.empty();
    }

    size_t RingBuffer::unreaded_buffer_size() const
    {
        if (_M_read_pointer <= _M_write_pointer)
        {
            return _M_write_pointer - _M_read_pointer;
        }


        return (_M_end_pointer - _M_read_pointer) + (_M_write_pointer - _M_start_pointer);
    }

    size_t RingBuffer::free_size() const
    {
        return _M_data.size() - unreaded_buffer_size();
    }

    void* RingBuffer::reading_pointer()
    {
        if (unreaded_buffer_size() == 0)
            return nullptr;

        return _M_read_pointer;
    }

    RingBuffer& RingBuffer::finish_read(size_t size)
    {
        _M_read_pointer += size;
        if (_M_read_pointer >= _M_end_pointer)
            _M_read_pointer = _M_start_pointer;
        return *this;
    }
}// namespace Engine
