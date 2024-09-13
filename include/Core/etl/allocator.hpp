#pragma once
#include <Core/export.hpp>
#include <limits>
#include <utility>

namespace Engine
{
	struct AllocatorBase {
		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new ((void*) p) U(std::forward<Args>(args)...);
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}
	};

	struct ENGINE_EXPORT ByteAllocator : AllocatorBase {
		using value_type      = unsigned char;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		unsigned char* allocate(size_type size);
		void deallocate(unsigned char* ptr, size_type size) noexcept;
	};

	template<typename T>
	struct Allocator : ByteAllocator {
		using value_type      = T;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		template<typename U>
		struct rebind {
			using other = Allocator<U>;
		};

		Allocator()                 = default;
		Allocator(const Allocator&) = default;

		template<typename U>
		Allocator(const Allocator<U>&)
		{}

		Allocator& operator=(const Allocator& other) = default;

		~Allocator()
		{}

		pointer allocate(size_type size)
		{
			return reinterpret_cast<pointer>(ByteAllocator().allocate(size * sizeof(T)));
		}

		void deallocate(pointer ptr, size_type size) noexcept
		{
			ByteAllocator().deallocate(reinterpret_cast<unsigned char*>(ptr), size * sizeof(T));
		}
	};


	template<typename T, typename U>
	inline bool operator==(const Allocator<T>&, const Allocator<U>&)
	{
		return true;
	}

	template<typename T, typename U>
	inline bool operator!=(const Allocator<T>&, const Allocator<U>&)
	{
		return false;
	}


	struct ENGINE_EXPORT BlockAllocatorBase : AllocatorBase {
	public:
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

	protected:
		size_type allign_pointer(unsigned char* p, size_type align) const noexcept;
	};

	template<typename T, std::size_t block_size = 4096>
	struct BlockAllocator : BlockAllocatorBase {
		using value_type      = T;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;

		template<typename U>
		struct rebind {
			typedef BlockAllocator<U> other;
		};

	private:
		using byte = ByteAllocator::value_type;

		union Node
		{
			value_type element;
			Node* next;
		};

		static_assert(block_size >= 2 * sizeof(Node), "block_size too small.");

		Node* m_current_block = nullptr;
		Node* m_current_slot  = nullptr;
		Node* m_last_slot     = nullptr;
		Node* m_free_slots    = nullptr;

		void allocate_block()
		{
			ByteAllocator allocator;

			union
			{
				byte* block = nullptr;
				Node* node;
			};

			block           = allocator.allocate(block_size);
			node->next      = m_current_block;
			m_current_block = node;

			byte* data        = block + sizeof(Node*);
			size_type padding = allign_pointer(data, alignof(Node));
			m_current_slot    = reinterpret_cast<Node*>(data + padding);
			m_last_slot       = reinterpret_cast<Node*>(block + block_size - sizeof(Node) + 1);
		}

	public:
		BlockAllocator() noexcept = default;

		BlockAllocator(const BlockAllocator& other) noexcept
		{}

		BlockAllocator(BlockAllocator&& other) noexcept
		{
			(*this) = std::move(other);
		}

		~BlockAllocator() noexcept
		{
			ByteAllocator allocator;

			while (m_current_block != nullptr)
			{
				Node* prev = m_current_block->next;
				allocator.deallocate(reinterpret_cast<byte*>(m_current_block), block_size);
				m_current_block = prev;
			}
		}

		BlockAllocator& operator=(const BlockAllocator& other)
		{
			return *this;
		}

		BlockAllocator& operator=(BlockAllocator&& other) noexcept
		{
			if (this != &other)
			{
				m_current_block = other.currentBlock_;
				m_current_slot  = other.m_current_slot;
				m_last_slot     = other.m_last_slot;
				m_free_slots    = other.m_free_slots;

				other.m_current_block = nullptr;
				other.m_current_slot  = nullptr;
				other.m_last_slot     = nullptr;
				other.m_free_slots    = nullptr;
			}
			return *this;
		}

		pointer allocate(size_type n = 1)
		{
			if (m_free_slots != nullptr)
			{
				pointer result = reinterpret_cast<pointer>(m_free_slots);
				m_free_slots   = m_free_slots->next;
				return result;
			}
			else
			{
				if (m_current_slot >= m_last_slot)
					allocate_block();
				return reinterpret_cast<pointer>(m_current_slot++);
			}
		}

		void deallocate(pointer ptr, size_type size) noexcept
		{
			if (ptr != nullptr)
			{
				reinterpret_cast<Node*>(ptr)->next = m_free_slots;
				m_free_slots                       = reinterpret_cast<Node*>(ptr);
			}
		}

		size_type max_size() const noexcept
		{
			auto max_blocks = std::numeric_limits<size_type>::max() / block_size;
			return (max_blocks - sizeof(Node*)) / sizeof(Node) * max_blocks;
		}
	};
}// namespace Engine
