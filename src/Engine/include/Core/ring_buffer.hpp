#pragma once
#include <Core/engine_types.hpp>
#include <atomic>
#include <mutex>

namespace Engine
{
    class ENGINE_EXPORT RingBuffer final
    {
    public:
        class ENGINE_EXPORT AllocationContext final
        {
        private:
            RingBuffer& _M_buffer;
            size_t _M_size;
            byte* _M_start_pointer;

        public:
            AllocationContext(RingBuffer& buffer, size_t size);
            void* data() const;
            AllocationContext& submit();
            size_t size() const;

            ~AllocationContext();
        };


    private:
        std::mutex _M_mutex;
        Vector<byte> _M_data;

        size_t _M_alignment;
        std::atomic_uint64_t _M_unreaded_size;
        std::atomic<byte*> _M_write_pointer;
        std::atomic<byte*> _M_read_pointer;

        byte* _M_start_pointer;
        byte* _M_end_pointer;

        class ExecutableObject* _M_event;

    public:
        RingBuffer();
        RingBuffer(size_t size, size_t alignment = 16, class ExecutableObject* event = nullptr);
        RingBuffer& init(size_t size, size_t alignment = 16, class ExecutableObject* event = nullptr);
        bool is_inited() const;
        size_t unreaded_buffer_size() const;
        size_t free_size() const;
        size_t size() const;
        void* reading_pointer();
        RingBuffer& finish_read(size_t size);
    };
}// namespace Engine
