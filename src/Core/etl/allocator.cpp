#include <Core/etl/allocator.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/vector.hpp>
#include <Core/memory.hpp>
#include <Core/thread.hpp>
#include <cstdlib>

namespace Engine
{
	namespace
	{
		struct StackAllocatorData {
			using size_type                                = StackByteAllocator::size_type;
			static constexpr size_type min_block_size      = 1024 * 32;
			static constexpr size_type min_block_alignment = 16;

			static CriticalSection s_critical_section;
			static Vector<StackAllocatorData*> s_stacks;

			struct Node {
				Node* m_next = nullptr;

				byte* m_begin;
				byte* m_end;
				byte* m_stack;

				inline Node(size_type size, size_type align)
				{
					m_begin = allocate_memory(size, align);
					m_end   = m_begin + size;
					m_stack = m_begin;
				}

				~Node() { release_memory(m_begin); }
			};

			Thread* const m_thread;
			Node* m_head    = nullptr;
			Node* m_current = nullptr;

			StackAllocatorData() : m_thread(ThisThread::self())
			{
				s_critical_section.lock();
				s_stacks.push_back(this);
				s_critical_section.unlock();

				m_head    = allocate_block();
				m_current = m_head;
			}

			Node* allocate_block(size_type size = 0, size_type align = 0)
			{
				return allocate<Node>(glm::max(size, min_block_size), glm::max(align, min_block_alignment));
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

					prev               = m_current;
					m_current          = m_current->m_next;
					m_current->m_stack = m_current->m_begin;
				}

				m_current    = allocate_block(size, align);
				prev->m_next = m_current;
				m_current->m_stack += size;
				return m_current->m_begin;
			}

			inline void flush()
			{
				if (m_thread != ThisThread::self())
				{
					m_thread->call([this]() { flush(); });
					return;
				}

				m_current          = m_head;
				m_current->m_stack = m_current->m_begin;
			}

			~StackAllocatorData()
			{
				s_critical_section.lock();
				s_stacks.erase(std::find(s_stacks.begin(), s_stacks.end(), this));
				s_critical_section.unlock();

				while (m_head)
				{
					Node* next = m_head->m_next;
					release(m_head);
					m_head = next;
				}
			}
		};

		Vector<StackAllocatorData*> StackAllocatorData::s_stacks;
		CriticalSection StackAllocatorData::s_critical_section;
		static thread_local StackAllocatorData s_stack_allocator;
	}// namespace


	unsigned char* ByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		return static_cast<unsigned char*>(std::aligned_alloc(align, size));
	}

	void ByteAllocator::deallocate(unsigned char* ptr) noexcept
	{
		std::free(ptr);
	}

	StackByteAllocator::Mark::Mark()
	{
		auto node  = s_stack_allocator.m_current;
		m_datas[0] = node;
		m_datas[1] = node->m_stack;
	}

	StackByteAllocator::Mark::~Mark()
	{
		s_stack_allocator.m_current          = static_cast<StackAllocatorData::Node*>(m_datas[0]);
		s_stack_allocator.m_current->m_stack = static_cast<byte*>(m_datas[1]);
	}

	unsigned char* StackByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		return s_stack_allocator.allocate_aligned(size, align);
	}

	void StackByteAllocator::flush()
	{
		StackAllocatorData::s_critical_section.lock();

		for (StackAllocatorData* stack : StackAllocatorData::s_stacks)
		{
			stack->flush();
		}

		StackAllocatorData::s_critical_section.unlock();
	}
}// namespace Engine
