module;
#include <Core/etl/std.hpp>
export module Trinex.Core.ETL:Memory;

export namespace Trinex::etl
{
	// Pointer utilities
	using std::addressof;
	using std::pointer_traits;
	using std::to_address;

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

	using std::allocator_traits;
} // namespace Trinex::etl
