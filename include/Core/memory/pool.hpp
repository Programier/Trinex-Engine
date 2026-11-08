#pragma once

namespace Trinex
{
	struct PoolAllocatorEntry;

	class PoolAllocatorBase
	{
	private:
		PoolAllocatorEntry* m_objects = nullptr;

	protected:
		PoolAllocatorBase(void* memory = nullptr, usize stride = 0, usize size = 0);
		trinex_non_copyable(PoolAllocatorBase);
		trinex_non_moveable(PoolAllocatorBase);

		void add_memory(void* memory, usize stride, usize size);

		[[nodiscard]] void* allocate();
		void free(void* ptr);
	};

	template<usize stride, usize page, usize align = 16>
	class trinex_api PoolAllocator final : protected PoolAllocatorBase
	{
	public:
		using Super = PoolAllocatorBase;

		static_assert(stride >= 8);
		static_assert(page > 0);
		static_assert(align >= 8 && (align & (align - 1)) == 0);
		static_assert(stride % align == 0);

		static consteval usize calculate_stride() { return stride < align ? align : stride; }

	public:
		inline PoolAllocator() : Super() {}
		inline PoolAllocator(void* memory, usize size) : Super(memory, calculate_stride(), size) {}
		inline void add_memory(void* memory, usize size) { Super::add_memory(memory, calculate_stride(), size); }

		using Super::allocate;
		using Super::free;
	};

	class trinex_api DynamicPoolAllocatorBase
	{
	private:
		PoolAllocatorEntry* m_pools   = nullptr;
		PoolAllocatorEntry* m_objects = nullptr;

	protected:
		DynamicPoolAllocatorBase(void* memory = nullptr, usize stride = 0, usize size = 0);
		trinex_non_copyable(DynamicPoolAllocatorBase);
		trinex_non_moveable(DynamicPoolAllocatorBase);
		~DynamicPoolAllocatorBase();

		void add_pool(usize stride, usize page, usize align);
		void add_memory(void* memory, usize stride, usize size);

		[[nodiscard]] void* allocate(usize stride, usize page, usize align);
		void free(void* ptr);
	};

	template<usize stride, usize page, usize align = 16>
	class trinex_api DynamicPoolAllocator final : public DynamicPoolAllocatorBase
	{
	public:
		using Super = DynamicPoolAllocatorBase;

		static_assert(stride >= 8);
		static_assert(page > 0);
		static_assert(align >= 8 && (align & (align - 1)) == 0);
		static_assert(stride % align == 0);

		static consteval usize calculate_stride() { return stride < align ? align : stride; }

	public:
		inline DynamicPoolAllocator() : Super(nullptr, calculate_stride(), 0) {}
		inline DynamicPoolAllocator(void* memory, usize size) : Super(memory, calculate_stride(), size) {}
		inline void add_memory(void* memory, usize size) { Super::add_memory(memory, calculate_stride(), size); }

		[[nodiscard]] void* allocate() { return Super::allocate(stride, page, align); }
		void free(void* ptr) { Super::free(ptr); }
	};
} // namespace Trinex
