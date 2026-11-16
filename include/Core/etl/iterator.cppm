module;

#include <iterator>

export module Trinex.Core.ETL.Iterator;

export namespace Trinex::etl
{
	// Range access
	using std::begin;
	using std::cbegin;
	using std::cend;
	using std::end;

	using std::crbegin;
	using std::crend;
	using std::rbegin;
	using std::rend;

	using std::data;
	using std::empty;
	using std::size;
	using std::ssize;

	// Iterator operations
	using std::advance;
	using std::distance;
	using std::next;
	using std::prev;

	// Iterator category tags
	using std::bidirectional_iterator_tag;
	using std::contiguous_iterator_tag;
	using std::forward_iterator_tag;
	using std::input_iterator_tag;
	using std::output_iterator_tag;
	using std::random_access_iterator_tag;

	// Iterator traits
	template<typename I>
	using iterator_traits = std::iterator_traits<I>;

	template<typename I>
	using incrementable_traits = std::incrementable_traits<I>;

	template<typename I>
	using indirectly_readable_traits = std::indirectly_readable_traits<I>;

	// Iterator associated types
	template<typename I>
	using iter_value_t = std::iter_value_t<I>;

	template<typename I>
	using iter_difference_t = std::iter_difference_t<I>;

	template<typename I>
	using iter_reference_t = std::iter_reference_t<I>;

	template<typename I>
	using iter_rvalue_reference_t = std::iter_rvalue_reference_t<I>;

	template<typename I>
	using iter_common_reference_t = std::iter_common_reference_t<I>;

	// Iterator concepts
	template<typename I>
	concept input_or_output_iterator = std::input_or_output_iterator<I>;

	template<typename I>
	concept input_iterator = std::input_iterator<I>;

	template<typename I, typename T>
	concept output_iterator = std::output_iterator<I, T>;

	template<typename I>
	concept forward_iterator = std::forward_iterator<I>;

	template<typename I>
	concept bidirectional_iterator = std::bidirectional_iterator<I>;

	template<typename I>
	concept random_access_iterator = std::random_access_iterator<I>;

	template<typename I>
	concept contiguous_iterator = std::contiguous_iterator<I>;

	template<typename S, typename I>
	concept sentinel_for = std::sentinel_for<S, I>;

	template<typename S, typename I>
	concept sized_sentinel_for = std::sized_sentinel_for<S, I>;

	// Iterator adaptors
	template<typename I>
	using reverse_iterator = std::reverse_iterator<I>;

	template<typename I>
	using move_iterator = std::move_iterator<I>;

	template<typename I>
	using move_sentinel = std::move_sentinel<I>;

	template<typename I>
	using counted_iterator = std::counted_iterator<I>;

	template<typename I, typename S>
	using common_iterator = std::common_iterator<I, S>;

	// Insert iterators
	template<typename C>
	using back_insert_iterator = std::back_insert_iterator<C>;

	template<typename C>
	using front_insert_iterator = std::front_insert_iterator<C>;

	template<typename C>
	using insert_iterator = std::insert_iterator<C>;

	using std::back_inserter;
	using std::front_inserter;
	using std::inserter;

	// Iterator factories
	using std::make_move_iterator;
	using std::make_reverse_iterator;

	// Sentinels
	using std::default_sentinel;
	using std::default_sentinel_t;

	using std::unreachable_sentinel;
	using std::unreachable_sentinel_t;
} // namespace Trinex::etl
