#pragma once
#include <cstring>
#include <memory>

namespace Trinex::etl
{
	using std::addressof;
	using std::align;
	using std::allocator;
	using std::allocator_traits;
	using std::construct_at;
	using std::default_delete;
	using std::destroy;
	using std::destroy_at;
	using std::destroy_n;
	using std::memmove;
	using std::pointer_traits;
	using std::to_address;
	using std::uninitialized_copy;
	using std::uninitialized_copy_n;
	using std::uninitialized_default_construct;
	using std::uninitialized_default_construct_n;
	using std::uninitialized_fill;
	using std::uninitialized_fill_n;
	using std::uninitialized_move;
	using std::uninitialized_move_n;
	using std::uninitialized_value_construct;
	using std::uninitialized_value_construct_n;
} // namespace Trinex::etl
