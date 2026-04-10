// SPDX-License-Identifier: BSD-3-Clause

// tlsf-bsd is freely redistributable under the BSD License. See the file
// "LICENSE" for information on usage and redistribution of this file.


#pragma once

#include <cstddef>
#include <cstdint>

namespace tlsf
{
#define TLSF_ENABLE_CHECK 1
	// Second-level subdivisions: 32 bins per first-level class.
	// Max internal fragmentation bounded by 1/SL_COUNT = 3.125% (was 6.25%
	// with 16 bins). Control structure size increases ~2x for the block
	// pointer array.

	inline constexpr std::size_t TLSF_SL_COUNT = 32;


	// Configurable maximum pool size: define TLSF_MAX_POOL_BITS to clamp
	// the first-level index. Pool cannot exceed 2^TLSF_MAX_POOL_BITS bytes.
	// E.g. -DTLSF_MAX_POOL_BITS=20 for a 1MB-max pool.

#ifdef TLSF_MAX_POOL_BITS
	inline constexpr std::size_t TLSF_FL_MAX = TLSF_MAX_POOL_BITS;
#else
#if __SIZE_WIDTH__ == 64 || defined(_WIN64) || defined(__LP64__)
	inline constexpr std::size_t TLSF_FL_MAX = 39;
#else
	inline constexpr std::size_t TLSF_FL_MAX = 31;
#endif
#endif

	// FL_SHIFT = log2(SL_COUNT) + log2(ALIGN_SIZE)
#if __SIZE_WIDTH__ == 64 || defined(_WIN64) || defined(__LP64__)
	inline constexpr std::size_t TLSF_FL_SHIFT = 8;
#else
	inline constexpr std::size_t TLSF_FL_SHIFT = 7;
#endif

	inline constexpr std::size_t TLSF_FL_COUNT = (TLSF_FL_MAX - TLSF_FL_SHIFT + 1);
	inline constexpr std::size_t TLSF_MAX_SIZE = ((std::size_t{1} << (TLSF_FL_MAX - 1)) - sizeof(std::size_t));

	using ResizeCallback = void*(std::size_t size, void* userdata);
	using WalkCallback   = void(void* ptr, size_t size, bool used, void* user);

	//  Block header structure.

	//  prev:      Pointer to the previous physical block.  Only valid when the
	//             previous block is free; physically stored at the tail of that
	//             block's payload.
	//  header:    Size (upper bits) | status bits (lower 2 bits).
	//  next_free: Next block in the same free list (only valid when free).
	//  prev_free: Previous block in the same free list (only valid when free).

	struct Block {
		Block* prev;
		std::size_t header;
		Block* next_free;
		Block* prev_free;
	};

	struct TLSF {
		std::uint32_t fl;
		std::uint32_t sl[TLSF_FL_COUNT];
		void* arena;
		std::size_t size;
		Block* block[TLSF_FL_COUNT][TLSF_SL_COUNT];
		Block block_null;
		ResizeCallback* allocator;
		void* userdata;
	};

	// Heap statistics structure for monitoring allocator state.

	struct Stats {
		std::size_t total_free;  // Total free bytes available
		std::size_t largest_free;// Largest contiguous free block
		std::size_t total_used;  // Total bytes in allocated blocks
		std::size_t block_count; // Total number of blocks (free + used)
		std::size_t free_count;  // Number of free blocks (fragmentation indicator)
		std::size_t overhead;    // Metadata overhead bytes
	};

	class Allocator
	{
	private:
		TLSF m_tlsf;

	public:
		// Default constructor (zero-initializes everything)
		Allocator() = default;

		// Delete copy/move semantics to prevent accidental duplication of the allocator state
		Allocator(const Allocator&)            = delete;
		Allocator& operator=(const Allocator&) = delete;

		// Initialize the allocator with a dynamic pool.
		// @param allocator Callback for resizing
		// @param userdata  Custom user data passed to the callback
		void init(ResizeCallback* allocator, void* userdata = nullptr);


		//  Initialize the allocator with a fixed-size memory pool.
		//  The pool will not auto-grow via resize callback; when the pool is
		//  exhausted, allocations return nullptr. Callers may still extend the
		//  pool explicitly via append_pool() with adjacent memory.

		//  @param mem   Pointer to the memory region to use as the pool
		//  @param bytes Total size of the memory region in bytes
		//  @return      Usable bytes in the pool, or 0 on failure
		std::size_t init(void* mem, std::size_t bytes);


		//  Append a memory block to an existing pool, potentially coalescing with
		//  the last block if it's free.

		//  @param mem  Pointer to the memory block to append
		//  @param size Size of the memory block in bytes
		//  @return Number of bytes used from the memory block, 0 on failure

		std::size_t append_pool(void* mem, std::size_t size);


		// Reset a static pool to its initial state, discarding all allocations.
		// WARNING: All previously returned pointers become invalid.
		void pool_reset();


		//  Allocate memory from the pool.

		//  @param size Requested allocation size in bytes. A zero size request
		//  returns a unique minimum-sized allocation.
		//  @return Pointer to at least @size bytes, aligned to ALIGN_SIZE, or nullptr.
		[[nodiscard]] void* alloc(std::size_t size);


		//  Allocate memory with a specified alignment.

		//  @param size  Requested allocation size in bytes.
		//  @param align Alignment in bytes; must be a non-zero power of two
		//  @return Pointer to at least @size bytes aligned to @align, or nullptr.
		[[nodiscard]] void* alloc(std::size_t size, std::size_t align);


		// Reallocate memory to a new size.
		[[nodiscard]] void* realloc(void* ptr, std::size_t size);


		// Releases the previously allocated memory.
		void free(void* ptr);


		// Return the usable size of an existing allocation.
		// Note: Implemented as a static method because it usually only reads
		// the block header preceding the pointer.
		static std::size_t usable_size(void* ptr);


		// Collect heap statistics by walking all blocks.
		// @param stats Output structure to fill with statistics
		// @return true on success, false on failure
		Stats stats() const;

		void walk(WalkCallback* callback, void* userdata = nullptr);

#ifdef TLSF_ENABLE_CHECK
		void check() const;
#else
		inline void check() const {}
#endif
	};
}// namespace tlsf
