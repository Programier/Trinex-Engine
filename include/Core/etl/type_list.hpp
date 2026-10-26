#pragma once

namespace Trinex
{
	template<class... Ts>
	struct TypeList;

	namespace type_list_detail
	{
		using size_type       = decltype(sizeof(0));
		using difference_type = decltype(static_cast<char*>(nullptr) - static_cast<char*>(nullptr));

		template<class A, class B>
		inline constexpr bool is_same_v = false;
		template<class A>
		inline constexpr bool is_same_v<A, A> = true;

		template<bool V>
		struct bool_constant {
			static constexpr bool value = V;
		};

		template<bool Condition, class True, class False>
		struct conditional {
			using type = True;
		};
		template<class True, class False>
		struct conditional<false, True, False> {
			using type = False;
		};
		template<bool Condition, class True, class False>
		using conditional_t = typename conditional<Condition, True, False>::type;

		template<class T>
		struct remove_reference {
			using type = T;
		};
		template<class T>
		struct remove_reference<T&> {
			using type = T;
		};
		template<class T>
		struct remove_reference<T&&> {
			using type = T;
		};
		template<class T>
		struct remove_cv {
			using type = T;
		};
		template<class T>
		struct remove_cv<const T> {
			using type = T;
		};
		template<class T>
		struct remove_cv<volatile T> {
			using type = T;
		};
		template<class T>
		struct remove_cv<const volatile T> {
			using type = T;
		};
		template<class T>
		using remove_cvref_t = typename remove_cv<typename remove_reference<T>::type>::type;

		template<size_type N, class... Ts>
		struct type_at_impl {
			static_assert(N < sizeof...(Ts), "TypeList: type_at index out of bounds");
		};
		template<size_type N, class Head, class... Tail>
		struct type_at_impl<N, Head, Tail...> : type_at_impl<N - 1, Tail...> {
		};
		template<class Head, class... Tail>
		struct type_at_impl<0, Head, Tail...> {
			using type = Head;
		};

		template<size_type... Indices>
		struct index_sequence {
		};

		template<class A, class B>
		struct sequence_concat;
		template<size_type... A, size_type... B>
		struct sequence_concat<index_sequence<A...>, index_sequence<B...>> {
			using type = index_sequence<A..., (sizeof...(A) + B)...>;
		};

		// Logarithmic template recursion depth instead of O(N).
		template<size_type N>
		struct make_index_sequence_impl {
			using type = typename sequence_concat<typename make_index_sequence_impl<N / 2>::type,
			                                      typename make_index_sequence_impl<N - N / 2>::type>::type;
		};
		template<>
		struct make_index_sequence_impl<0> {
			using type = index_sequence<>;
		};
		template<>
		struct make_index_sequence_impl<1> {
			using type = index_sequence<0>;
		};
		template<size_type N>
		using make_index_sequence = typename make_index_sequence_impl<N>::type;
	}// namespace type_list_detail

	namespace type_list_detail
	{
		template<class... Lists>
		struct concat;

		template<>
		struct concat<> {
			using type = TypeList<>;
		};

		template<class... Ts>
		struct concat<TypeList<Ts...>> {
			using type = TypeList<Ts...>;
		};

		template<class... A, class... B, class... Rest>
		struct concat<TypeList<A...>, TypeList<B...>, Rest...> : concat<TypeList<A..., B...>, Rest...> {
		};

		template<class... Lists>
		using concat_t = typename concat<Lists...>::type;

		template<class List, type_list_detail::size_type Begin, type_list_detail::size_type End>
		struct slice {
			static_assert(Begin <= End && End <= List::size_v, "TypeList: slice out of bounds");

			template<type_list_detail::size_type... I>
			static auto make(type_list_detail::index_sequence<I...>) -> TypeList<typename List::template type_at<Begin + I>...>;

			using type = decltype(make(type_list_detail::make_index_sequence<(Begin <= End ? End - Begin : 0)>{}));
		};

		template<class List>
		struct reverse {
			template<type_list_detail::size_type... I>
			static auto make(type_list_detail::index_sequence<I...>)
			        -> TypeList<typename List::template type_at<List::size_v - 1 - I>...>;

			using type = decltype(make(type_list_detail::make_index_sequence<List::size_v>{}));
		};

		template<class List, type_list_detail::size_type A, type_list_detail::size_type B>
		struct swap {
			static_assert(A < List::size_v && B < List::size_v, "TypeList: swap indices out of bounds");

			template<type_list_detail::size_type... I>
			static auto make(type_list_detail::index_sequence<I...>)
			        -> TypeList<typename List::template type_at<I == A ? B : (I == B ? A : I)>...>;

			using type = decltype(make(type_list_detail::make_index_sequence<List::size_v>{}));
		};

		template<class List, class T>
		struct remove;

		template<class... Ts, class T>
		struct remove<TypeList<Ts...>, T> {
			using type =
			        concat_t<type_list_detail::conditional_t<type_list_detail::is_same_v<Ts, T>, TypeList<>, TypeList<Ts>>...>;
		};

		template<class Left, class Right>
		struct intersection;

		template<class... A, class... B>
		struct intersection<TypeList<A...>, TypeList<B...>> {
			using type =
			        concat_t<type_list_detail::conditional_t<TypeList<B...>::template contains_v<A>, TypeList<A>, TypeList<>>...>;
		};

		template<class Left, class Right>
		struct difference;

		template<class... A, class... B>
		struct difference<TypeList<A...>, TypeList<B...>> {
			using type =
			        concat_t<type_list_detail::conditional_t<TypeList<B...>::template contains_v<A>, TypeList<>, TypeList<A>>...>;
		};

		template<class Acc, class T, class... Remaining>
		struct remove_first;

		template<class... Seen, class T>
		struct remove_first<TypeList<Seen...>, T> {
			using type = TypeList<Seen...>;
		};

		template<class... Seen, class T, class Head, class... Tail>
		struct remove_first<TypeList<Seen...>, T, Head, Tail...> {
			using type = type_list_detail::conditional_t<type_list_detail::is_same_v<Head, T>, TypeList<Seen..., Tail...>,
			                                             typename remove_first<TypeList<Seen..., Head>, T, Tail...>::type>;
		};

		template<class List, type_list_detail::size_type Count>
		struct repeat {
			template<type_list_detail::size_type... I>
			static auto make(type_list_detail::index_sequence<I...>)
			        -> concat_t<type_list_detail::conditional_t<(I < Count), List, TypeList<>>...>;

			using type = decltype(make(type_list_detail::make_index_sequence<Count>{}));
		};

		template<class List, class Other>
		struct is_subset : type_list_detail::bool_constant<false> {
		};

		template<class... A, class... B>
		struct is_subset<TypeList<A...>, TypeList<B...>>
		    : type_list_detail::bool_constant<(TypeList<B...>::template contains_v<A> && ...)> {
		};

		template<class List, template<class> class Pred>
		struct filter;

		template<class... Ts, template<class> class Pred>
		struct filter<TypeList<Ts...>, Pred> {
			using type = concat_t<type_list_detail::conditional_t<bool(Pred<Ts>::value), TypeList<Ts>, TypeList<>>...>;
		};

		template<class List, template<class> class Pred>
		struct reject;

		template<class... Ts, template<class> class Pred>
		struct reject<TypeList<Ts...>, Pred> {
			using type = concat_t<type_list_detail::conditional_t<bool(Pred<Ts>::value), TypeList<>, TypeList<Ts>>...>;
		};

		template<class Acc, class... Remaining>
		struct unique_acc;

		template<class... Seen>
		struct unique_acc<TypeList<Seen...>> {
			using type = TypeList<Seen...>;
		};

		template<class... Seen, class Head, class... Tail>
		struct unique_acc<TypeList<Seen...>, Head, Tail...>
		    : unique_acc<type_list_detail::conditional_t<(type_list_detail::is_same_v<Head, Seen> || ...), TypeList<Seen...>,
		                                                 TypeList<Seen..., Head>>,
		                 Tail...> {
		};

		template<class List>
		struct unique_list;

		template<class... U>
		struct unique_list<TypeList<U...>> {
			using type = typename unique_acc<TypeList<>, U...>::type;
		};

		template<class Acc, template<class, class> class Fn, class... Ts>
		struct fold;

		template<class Acc, template<class, class> class Fn>
		struct fold<Acc, Fn> {
			using type = Acc;
		};

		template<class Acc, template<class, class> class Fn, class Head, class... Tail>
		struct fold<Acc, Fn, Head, Tail...> : fold<typename Fn<Acc, Head>::type, Fn, Tail...> {
		};

		template<class Head, class List>
		struct prepend;

		template<class Head, class... Ts>
		struct prepend<Head, TypeList<Ts...>> {
			using type = TypeList<Head, Ts...>;
		};

		template<class Sorted, class T, template<class, class> class Less>
		struct sorted_insert;

		template<class T, template<class, class> class Less>
		struct sorted_insert<TypeList<>, T, Less> {
			using type = TypeList<T>;
		};

		template<class Head, class... Tail, class T, template<class, class> class Less>
		struct sorted_insert<TypeList<Head, Tail...>, T, Less> {
			using with_tail = typename sorted_insert<TypeList<Tail...>, T, Less>::type;
			using type      = type_list_detail::conditional_t<bool(Less<T, Head>::value), TypeList<T, Head, Tail...>,
			                                                  typename prepend<Head, with_tail>::type>;
		};

		template<class Acc, template<class, class> class Less, class... Ts>
		struct sort_acc;

		template<class Acc, template<class, class> class Less>
		struct sort_acc<Acc, Less> {
			using type = Acc;
		};

		template<class Acc, template<class, class> class Less, class Head, class... Tail>
		struct sort_acc<Acc, Less, Head, Tail...> : sort_acc<typename sorted_insert<Acc, Head, Less>::type, Less, Tail...> {
		};

		template<class Left, class Right>
		struct zip;

		template<class... L, class... R>
		struct zip<TypeList<L...>, TypeList<R...>> {
			static_assert(sizeof...(L) == sizeof...(R), "TypeList: zip requires lists of equal size");
			using type = TypeList<TypeList<L, R>...>;
		};

		template<class Left, class Right>
		struct cartesian_product;

		template<class... L, class... R>
		struct cartesian_product<TypeList<L...>, TypeList<R...>> {
			template<class A>
			using row  = TypeList<TypeList<A, R>...>;
			using type = concat_t<row<L>...>;
		};

		// Defer creation of function signatures: a type list may contain void,
		// but a requested function signature may not accept those types.
		template<class List, class Return>
		struct function_forms;

		template<class... Args, class Return>
		struct function_forms<TypeList<Args...>, Return> {
			using pointer   = Return (*)(Args...);
			using signature = Return(Args...);
		};

		template<class List, class Object, class Return>
		struct method_forms;

		template<class... Args, class Object, class Return>
		struct method_forms<TypeList<Args...>, Object, Return> {
			using normal          = Return (Object::*)(Args...);
			using constant        = Return (Object::*)(Args...) const;
			using noexcept_method = Return (Object::*)(Args...) noexcept;
			using lvalue          = Return (Object::*)(Args...) &;
			using rvalue          = Return (Object::*)(Args...) &&;
		};

		template<class List, class Deferred>
		struct flatten;

		template<class... SubLists, class Deferred>
		struct flatten<TypeList<SubLists...>, Deferred> {
			using type = concat_t<SubLists...>;
		};
	}// namespace type_list_detail

	template<class... Ts>
	struct TypeList {
		using Type                                          = TypeList;
		using self                                          = TypeList;
		static constexpr type_list_detail::size_type size_v = sizeof...(Ts);
		static constexpr bool empty_v                       = size_v == 0;

		// Type-only: no instance of Ts is ever created or stored.
		constexpr TypeList() noexcept = default;

		static consteval type_list_detail::size_type size() noexcept { return size_v; }
		static consteval bool empty() noexcept { return empty_v; }

		template<type_list_detail::size_type I>
		using type_at = typename type_list_detail::type_at_impl<I, Ts...>::type;

		template<type_list_detail::size_type I = 0>
		using front_t = type_at<I>;

		template<type_list_detail::size_type Offset = 0>
		using back_t = type_at<size_v - 1 - Offset>;

		template<class T>
		static constexpr bool contains_v = (type_list_detail::is_same_v<T, Ts> || ...);

		template<class T>
		static constexpr type_list_detail::size_type count_v =
		        (type_list_detail::size_type{0} + ... + (type_list_detail::is_same_v<T, Ts> ? 1U : 0U));

		template<class... U>
		static constexpr bool contains_all_v = (contains_v<U> && ...);

		template<class... U>
		static constexpr bool contains_any_v = (contains_v<U> || ...);

		template<class T>
		static consteval bool contains() noexcept
		{
			return contains_v<T>;
		}

		template<class T>
		static consteval type_list_detail::size_type count() noexcept
		{
			return count_v<T>;
		}

		template<class T, type_list_detail::size_type Start = 0>
		static consteval type_list_detail::difference_type index_of() noexcept
		{
			static_assert(Start <= size_v, "TypeList: index_of start out of bounds");
			constexpr bool matches[size_v + 1] = {type_list_detail::is_same_v<T, Ts>..., false};
			for (type_list_detail::size_type i = Start; i < size_v; ++i)
				if (matches[i])
					return static_cast<type_list_detail::difference_type>(i);
			return -1;
		}

		template<class T>
		static consteval type_list_detail::difference_type last_index_of() noexcept
		{
			constexpr bool matches[size_v + 1] = {type_list_detail::is_same_v<T, Ts>..., false};
			for (type_list_detail::size_type i = size_v; i > 0; --i)
				if (matches[i - 1])
					return static_cast<type_list_detail::difference_type>(i - 1);
			return -1;
		}

		template<template<class> class Pred>
		static constexpr bool all_of_v = (bool(Pred<Ts>::value) && ...);

		template<template<class> class Pred>
		static constexpr bool any_of_v = (bool(Pred<Ts>::value) || ...);

		template<template<class> class Pred>
		static consteval bool all_of() noexcept
		{
			return all_of_v<Pred>;
		}

		template<template<class> class Pred>
		static consteval bool any_of() noexcept
		{
			return any_of_v<Pred>;
		}

		template<template<class> class Pred>
		static consteval bool none_of() noexcept
		{
			return !any_of_v<Pred>;
		}

		template<template<class> class Pred>
		static consteval type_list_detail::size_type count_if() noexcept
		{
			return (type_list_detail::size_type{0} + ... + (Pred<Ts>::value ? 1U : 0U));
		}

		template<template<class> class Pred>
		static consteval type_list_detail::difference_type find_if() noexcept
		{
			constexpr bool matches[size_v + 1] = {bool(Pred<Ts>::value)..., false};
			for (type_list_detail::size_type i = 0; i < size_v; ++i)
				if (matches[i])
					return static_cast<type_list_detail::difference_type>(i);
			return -1;
		}

		static constexpr bool unique_v =
		        type_list_detail::is_same_v<TypeList, typename type_list_detail::unique_acc<TypeList<>, Ts...>::type>;

		template<class Other>
		static constexpr bool is_subset_of_v = type_list_detail::is_subset<TypeList, Other>::value;

		// Operations that return TYPES (without constructing any object).
		template<class... U>
		using push_back_t = TypeList<Ts..., U...>;

		template<class... U>
		using push_front_t = TypeList<U..., Ts...>;

		template<class... Lists>
		using concat_t = type_list_detail::concat_t<TypeList, Lists...>;

		// slice [Begin, End) -- End is exclusive.
		template<type_list_detail::size_type Begin, type_list_detail::size_type End>
		using slice_t = typename type_list_detail::slice<TypeList, Begin, End>::type;

		template<type_list_detail::size_type N>
		using take_t = slice_t<0, N>;

		template<type_list_detail::size_type N>
		using drop_t = slice_t<N, size_v>;

		template<type_list_detail::size_type N = 1>
		using pop_front_t = slice_t<(N <= size_v ? N : size_v + 1), size_v>;

		template<type_list_detail::size_type N = 1>
		using pop_back_t = slice_t<0, (N <= size_v ? size_v - N : size_v + 1)>;

		template<type_list_detail::size_type Index, class... U>
		using insert_t = type_list_detail::concat_t<slice_t<0, Index>, TypeList<U...>, slice_t<Index, size_v>>;

		template<type_list_detail::size_type Index, type_list_detail::size_type Count = 1>
		using erase_t = type_list_detail::concat_t<slice_t<0, Index>, slice_t<Index + Count, size_v>>;

		template<type_list_detail::size_type Index, class U>
		using replace_t = type_list_detail::concat_t<slice_t<0, Index>, TypeList<U>, slice_t<Index + 1, size_v>>;

		template<type_list_detail::size_type A, type_list_detail::size_type B>
		using swap_t = typename type_list_detail::swap<TypeList, A, B>::type;

		using reverse_t = typename type_list_detail::reverse<TypeList>::type;

		template<type_list_detail::size_type N>
		using rotate_left_t = type_list_detail::concat_t<slice_t<(size_v == 0 ? 0 : N % size_v), size_v>,
		                                                 slice_t<0, (size_v == 0 ? 0 : N % size_v)>>;

		template<type_list_detail::size_type N>
		using rotate_right_t = rotate_left_t<(size_v == 0 ? 0 : size_v - N % size_v)>;

		template<class U>
		using remove_t = typename type_list_detail::remove<TypeList, U>::type;

		template<class U>
		using remove_first_t = typename type_list_detail::remove_first<TypeList<>, U, Ts...>::type;

		// Set-like operations; result order follows the left-hand list.
		// intersection/difference preserve duplicates from the left-hand list.
		template<class Other>
		using intersection_t = typename type_list_detail::intersection<TypeList, Other>::type;

		template<class Other>
		using difference_t = typename type_list_detail::difference<TypeList, Other>::type;

		// Ordered set union: first occurrence is retained.
		template<class Other>
		using union_t = typename type_list_detail::unique_list<concat_t<Other>>::type;

		template<type_list_detail::size_type N>
		using repeat_t = typename type_list_detail::repeat<TypeList, N>::type;

		template<template<class> class Pred>
		using filter_t = typename type_list_detail::filter<TypeList, Pred>::type;

		template<template<class> class Pred>
		using reject_t = typename type_list_detail::reject<TypeList, Pred>::type;

		template<template<class> class Pred>
		using partition_t = TypeList<filter_t<Pred>, reject_t<Pred>>;

		template<template<class> class Mapper>
		using transform_t = TypeList<typename Mapper<Ts>::type...>;

		template<class Old, class New>
		using replace_all_t = TypeList<type_list_detail::conditional_t<type_list_detail::is_same_v<Ts, Old>, New, Ts>...>;

		using unique_t = typename type_list_detail::unique_acc<TypeList<>, Ts...>::type;

		template<class Init, template<class, class> class Reducer>
		using fold_t = typename type_list_detail::fold<Init, Reducer, Ts...>::type;

		template<template<class, class> class Less>
		using sort_t = typename type_list_detail::sort_acc<TypeList<>, Less, Ts...>::type;

		template<class Other>
		using zip_t = typename type_list_detail::zip<TypeList, Other>::type;

		template<class Other>
		using cartesian_product_t = typename type_list_detail::cartesian_product<TypeList, Other>::type;

		template<class Deferred = void>
		using flatten_t = typename type_list_detail::flatten<TypeList, Deferred>::type;

		template<template<class...> class Template>
		using rebind = Template<Ts...>;

		template<class Return = void>
		using make_function = typename type_list_detail::function_forms<TypeList, Return>::pointer;

		template<class Return = void>
		using make_function_type = typename type_list_detail::function_forms<TypeList, Return>::signature;

		template<class Object, class Return = void>
		using make_method = typename type_list_detail::method_forms<TypeList, Object, Return>::normal;

		template<class Object, class Return = void>
		using make_const_method = typename type_list_detail::method_forms<TypeList, Object, Return>::constant;

		template<class Object, class Return = void>
		using make_noexcept_method = typename type_list_detail::method_forms<TypeList, Object, Return>::noexcept_method;

		template<class Object, class Return = void>
		using make_lvalue_method = typename type_list_detail::method_forms<TypeList, Object, Return>::lvalue;

		template<class Object, class Return = void>
		using make_rvalue_method = typename type_list_detail::method_forms<TypeList, Object, Return>::rvalue;

		// Familiar function-style syntax retained from the original interface.
		template<class... U>
		static consteval auto push_back() -> push_back_t<U...>
		{
			return {};
		}

		template<class... U>
		    requires(sizeof...(U) > 0)
		static constexpr auto push_back(U&&...) -> push_back_t<type_list_detail::remove_cvref_t<U>...>
		{
			return {};
		}

		template<class... U>
		static consteval auto push_front() -> push_front_t<U...>
		{
			return {};
		}

		template<class... U>
		    requires(sizeof...(U) > 0)
		static constexpr auto push_front(U&&...) -> push_front_t<type_list_detail::remove_cvref_t<U>...>
		{
			return {};
		}

		template<type_list_detail::size_type N = 1>
		static consteval auto pop_back() -> pop_back_t<N>
		{
			return {};
		}

		template<type_list_detail::size_type N = 1>
		static consteval auto pop_front() -> pop_front_t<N>
		{
			return {};
		}

		template<class... U>
		static consteval auto merge_back(TypeList<U...> = {}) -> push_back_t<U...>
		{
			return {};
		}

		template<class... U>
		static consteval auto merge_front(TypeList<U...> = {}) -> push_front_t<U...>
		{
			return {};
		}

		// Callable: []<class T>() { ... }.
		// Extra arguments are forwarded on EACH invocation; avoid moving from them repeatedly.
		template<class Func, class... Extra>
		static constexpr void for_each(Func&& func, Extra&&... extra)
		{
			(static_cast<Func&&>(func).template operator()<Ts>(static_cast<Extra&&>(extra)...), ...);
		}

		// Callable: []<class T, type_list_detail::size_type I>() { ... }.
		template<class Func>
		static constexpr void for_each_indexed(Func&& func)
		{
			[&]<type_list_detail::size_type... I>(type_list_detail::index_sequence<I...>) {
				(static_cast<Func&&>(func).template operator()<type_at<I>, I>(), ...);
			}(type_list_detail::make_index_sequence<size_v>{});
		}

		// Runtime index dispatch. Returns false when index is out of range.
		// Callable: []<class T, type_list_detail::size_type I>() { ... }.
		template<class Func>
		static constexpr bool visit_at(type_list_detail::size_type index, Func&& func)
		{
			return [&]<type_list_detail::size_type... I>(type_list_detail::index_sequence<I...>) {
				return ((index == I ? (static_cast<void>(static_cast<Func&&>(func).template operator()<type_at<I>, I>()), true)
				                    : false) ||
				        ...);
			}(type_list_detail::make_index_sequence<size_v>{});
		}

		template<typename... Args, typename F, typename... Values>
		static constexpr decltype(auto) apply(F&& f, Values&&... values)
		{
			return static_cast<F&&>(f).template operator()<Args..., Ts...>(static_cast<Values&&>(values)...);
		}
	};
}// namespace Trinex
