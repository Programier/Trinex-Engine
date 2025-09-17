#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/vector.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/thread.hpp>
#include <cstdlib>

namespace Engine
{
	namespace
	{
		struct TempAllocatorData;

		struct TempAllocatorSync {
			Vector<TempAllocatorData*> stacks;
			CriticalSection critical_section;

			inline void lock() { critical_section.lock(); }
			inline void unlock() { critical_section.unlock(); }

			inline void push(TempAllocatorData* data)
			{
				lock();
				stacks.push_back(data);
				unlock();
			}

			inline void pop(TempAllocatorData* data)
			{
				lock();
				stacks.erase(std::find(stacks.begin(), stacks.end(), data));
				unlock();
			}
		};

		struct TempAllocatorData {
			using size_type                                = StackByteAllocator::size_type;
			static constexpr size_type min_block_size      = 1024 * 32;
			static constexpr size_type min_block_alignment = 16;

			struct Node {
				Node* m_next = nullptr;

				byte* m_begin;
				byte* m_end;
				byte* m_stack;

				inline Node(size_type size, size_type align)
				{
					m_begin = ByteAllocator::allocate_aligned(size, align);
					m_end   = m_begin + size;
					m_stack = m_begin;
				}

				~Node() { ByteAllocator::deallocate(m_begin); }
			};

			TempAllocatorSync* const m_sync;
			Node* m_head     = nullptr;
			Node* m_current  = nullptr;
			Thread* m_thread = nullptr;

			TempAllocatorData(TempAllocatorSync* sync) : m_sync(sync)
			{
				m_head    = allocate_block();
				m_current = m_head;
				m_thread  = ThisThread::self();
				sync->push(this);
			}

			TempAllocatorData& reset()
			{
				if (m_thread == ThisThread::self())
				{
					m_current          = m_head;
					m_current->m_stack = m_current->m_begin;
				}
				else
				{
					m_thread->call([this]() { reset(); });
				}
				return *this;
			}

			Node* allocate_block(size_type size = 0, size_type align = 0)
			{
				return trx_new Node(Math::max(size, min_block_size), Math::max(align, min_block_alignment));
			}

			inline byte* allocate_aligned(size_type size, size_type align)
			{
				Node* prev = nullptr;

				while (m_current)
				{
					byte* ptr = align_memory(m_current->m_stack, align);

					if (ptr + size < m_current->m_end)
					{
						m_current->m_stack = ptr + size;
						return ptr;
					}

					prev      = m_current;
					m_current = m_current->m_next;

					if (m_current)
						m_current->m_stack = m_current->m_begin;
				}

				m_current    = allocate_block(size, align);
				prev->m_next = m_current;
				m_current->m_stack += size;
				return m_current->m_begin;
			}

			~TempAllocatorData()
			{
				m_sync->pop(this);

				while (m_head)
				{
					Node* next = m_head->m_next;
					trx_delete m_head;
					m_head = next;
				}
			}
		};

		static TempAllocatorSync s_stack_sync;
		static TempAllocatorSync s_frame_sync;

		static thread_local TempAllocatorData s_stack_allocator(&s_stack_sync);
		static thread_local TempAllocatorData s_frame_allocator(&s_frame_sync);
	}// namespace

	unsigned char* ByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		size = align_up(size, align);

#if PLATFORM_WINDOWS
		return static_cast<unsigned char*>(_aligned_malloc(size, align));
#else
		return static_cast<unsigned char*>(std::aligned_alloc(align, size));
#endif
	}

	void ByteAllocator::deallocate(unsigned char* ptr) noexcept
	{
#if PLATFORM_WINDOWS
		_aligned_free(ptr);
#else
		std::free(ptr);
#endif
	}

	StackByteAllocator::Mark::Mark()
	{
		auto node  = s_stack_allocator.m_current;
		m_datas[0] = node;
		m_datas[1] = node->m_stack;
	}

	StackByteAllocator::Mark::~Mark()
	{
		s_stack_allocator.m_current          = static_cast<TempAllocatorData::Node*>(m_datas[0]);
		s_stack_allocator.m_current->m_stack = static_cast<byte*>(m_datas[1]);
	}

	unsigned char* StackByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		return s_stack_allocator.allocate_aligned(size, align);
	}

	static void reset_stack_allocator(TempAllocatorSync* sync)
	{
		sync->lock();
		for (TempAllocatorData* allocator : sync->stacks) allocator->reset();
		sync->unlock();
	}

	void StackByteAllocator::reset()
	{
		reset_stack_allocator(&s_stack_sync);
	}

	unsigned char* FrameByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		return s_frame_allocator.allocate_aligned(size, align);
	}

	void FrameByteAllocator::reset()
	{
		reset_stack_allocator(&s_frame_sync);
	}
}// namespace Engine
