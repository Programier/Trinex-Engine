module;

#include <cstring>
#include <memory>

export module Trinex.Core.ETL.Memory;

export namespace Trinex::etl
{
	// Raw memory operations
	using std::memchr;
	using std::memcmp;
	using std::memcpy;
	using std::memmove;
	using std::memset;

	// Pointer utilities
	using std::addressof;
	using std::pointer_traits;
	using std::to_address;

	// Alignment
	using std::align;
	using std::assume_aligned;

	// Object lifetime
	using std::construct_at;
	using std::destroy;
	using std::destroy_at;
	using std::destroy_n;
	using std::launder;

	// Uninitialized copy
	using std::uninitialized_copy;
	using std::uninitialized_copy_n;

	// Uninitialized move
	using std::uninitialized_move;
	using std::uninitialized_move_n;

	// Uninitialized fill
	using std::uninitialized_fill;
	using std::uninitialized_fill_n;

	// Default construction
	using std::uninitialized_default_construct;
	using std::uninitialized_default_construct_n;

	// Value construction
	using std::uninitialized_value_construct;
	using std::uninitialized_value_construct_n;
} // namespace Trinex::etl
