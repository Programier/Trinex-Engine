#pragma once
#include <Core/etl/storage.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/utility.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	template<typename>
	class DelegateRef;

	template<typename R, typename... Args>
	class DelegateRef<R(Args...)>
	{
	private:
		void* m_object                = nullptr;
		R (*m_caller)(void*, Args...) = nullptr;

	public:
		constexpr DelegateRef() noexcept = default;
		constexpr DelegateRef(null) noexcept {}

		template<typename F>
		    requires(!etl::same_as<etl::remove_cvref_t<F>, DelegateRef> && etl::is_invocable_r_v<R, F&, Args...>)
		DelegateRef(F&& func) noexcept
		    : m_object(const_cast<void*>(static_cast<const void*>(__builtin_addressof(func)))),
		      m_caller([](void* object, Args... args) -> R {
			      using Fn = etl::remove_reference_t<F>;
			      return (*static_cast<Fn*>(object))(static_cast<Args&&>(args)...);
		      })
		{}

		R operator()(Args... args) const { return m_caller(m_object, static_cast<Args&&>(args)...); }

		explicit operator bool() const noexcept { return m_caller != nullptr; }
	};

	template<auto value>
	struct DelegateTag {
	};

	class DelegateBase
	{
	public:
	protected:
		using Context = Storage<24, 8>;

		template<typename Method, typename T>
		struct PointerMethodContext {
			Method method;
			T* object;
		};

		template<typename F>
		static constexpr bool is_inline_storable = sizeof(F) <= sizeof(Context) && alignof(F) <= alignof(Context);

		mutable Context m_context = {};

	protected:
		template<typename... Args>
		static void dummy(Args...)
		{}

		template<typename T>
		static void static_copy_of(DelegateBase* self, const DelegateBase* src)
		{
			self->m_context.construct<T>(src->m_context.as<T>());
		}

		template<typename T>
		static void static_move_of(DelegateBase* self, DelegateBase* src)
		{
			self->m_context.construct<T>(static_cast<T&&>(src->m_context.as<T>()));
			src->m_context.destroy<T>();
		}

		template<typename T>
		static void static_destroy_of(DelegateBase* self)
		{
			self->m_context.destroy<T>();
		}

		template<typename T>
		static void heap_copy_of(DelegateBase* self, const DelegateBase* src)
		{
			self->m_context.construct<T*>(trx_new T(*(src->m_context.as<T*>())));
		}

		template<typename T>
		static void heap_move_of(DelegateBase* self, DelegateBase* src)
		{
			self->m_context.construct<T*>(src->m_context.as<T*>());
		}

		template<typename T>
		static void heap_destroy_of(DelegateBase* self)
		{
			trx_delete self->m_context.as<T*>();
		}
	};

	template<typename T>
	class Delegate;

	template<typename R, typename... Args>
	class Delegate<R(Args...)> : private DelegateBase
	{
	public:
		using FunctionPtr = R (*)(Args...);

	private:
		struct ContextManager {
			R (*invoke)(const Delegate*, Args...);
			void (*copy)(DelegateBase*, const DelegateBase*);
			void (*move)(DelegateBase*, DelegateBase*);
			void (*release)(DelegateBase*);
		};

		const ContextManager* m_manager = nullptr;

	private:
		template<auto func>
		static const ContextManager* static_function_manager()
		{
			static const ContextManager manager = {
			        .invoke  = [](const Delegate*, Args... args) -> R { return func(etl::forward<Args>(args)...); },
			        .copy    = DelegateBase::dummy,
			        .move    = DelegateBase::dummy,
			        .release = DelegateBase::dummy,
			};

			return &manager;
		}

		static const ContextManager* pointer_function_manager()
		{
			static const ContextManager manager = {
			        .invoke = [](const Delegate* delegate, Args... args) -> R {
				        return delegate->m_context.as<R (*)(Args...)>()(etl::forward<Args>(args)...);
			        },
			        .copy    = DelegateBase::static_copy_of<R (*)(Args...)>,
			        .move    = DelegateBase::static_move_of<R (*)(Args...)>,
			        .release = DelegateBase::dummy,
			};

			return &manager;
		}

		template<auto method, typename T>
		static const ContextManager* static_method_manager()
		{
			static const ContextManager manager = {
			        .invoke = [](const Delegate* delegate, Args... args) -> R {
				        T* object = delegate->m_context.as<T*>();
				        return (object->*method)(etl::forward<Args>(args)...);
			        },
			        .copy    = DelegateBase::static_copy_of<T*>,
			        .move    = DelegateBase::static_move_of<T*>,
			        .release = DelegateBase::dummy,
			};

			return &manager;
		}

		template<typename T, typename Method>
		static const ContextManager* pointer_method_manager()
		{
			using MethodContext = PointerMethodContext<Method, T>;

			static const ContextManager manager = {
			        .invoke = [](const Delegate* delegate, Args... args) -> R {
				        auto [method, object] = delegate->m_context.as<MethodContext>();
				        return (object->*method)(etl::forward<Args>(args)...);
			        },
			        .copy    = DelegateBase::static_copy_of<MethodContext>,
			        .move    = DelegateBase::static_move_of<MethodContext>,
			        .release = DelegateBase::dummy,
			};

			return &manager;
		}

		template<typename F>
		static const ContextManager* static_lambda_manager()
		{
			static const ContextManager manager = {
			        .invoke = [](const Delegate* delegate, Args... args) -> R {
				        F& context = delegate->m_context.as<F>();
				        return context(etl::forward<Args>(args)...);
			        },
			        .copy    = DelegateBase::static_copy_of<F>,
			        .move    = DelegateBase::static_move_of<F>,
			        .release = DelegateBase::static_destroy_of<F>,
			};

			return &manager;
		}

		template<typename F>
		static const ContextManager* heap_lambda_manager()
		{
			static const ContextManager manager = {
			        .invoke = [](const Delegate* delegate, Args... args) -> R {
				        F* context = delegate->m_context.as<F*>();
				        return (*context)(etl::forward<Args>(args)...);
			        },

			        .copy    = DelegateBase::heap_copy_of<F>,
			        .move    = DelegateBase::heap_move_of<F>,
			        .release = DelegateBase::heap_destroy_of<F>,
			};

			return &manager;
		}

	public:
		constexpr Delegate() noexcept = default;

		Delegate(FunctionPtr func) noexcept { bind(func); }

		template<auto func>
		    requires(!etl::is_member_function_pointer_v<decltype(func)> && etl::is_invocable_r_v<R, decltype(func), Args...>)
		Delegate(DelegateTag<func>) noexcept
		{
			bind<func>();
		}

		template<auto method, typename T>
		    requires(!etl::is_volatile_v<T> && etl::is_member_function_pointer_v<decltype(method)> &&
		             etl::is_invocable_r_v<R, decltype(method), T&, Args...>)
		Delegate(T* object, DelegateTag<method>) noexcept
		{
			bind<method>(object);
		}

		template<typename T, typename Method>
		    requires(etl::is_member_function_pointer_v<etl::decay_t<Method>> &&
		             etl::is_invocable_r_v<R, etl::decay_t<Method>, T*, Args...>)
		Delegate(T* object, Method&& method) noexcept
		{
			bind(object, etl::forward<Method>(method));
		}

		template<typename F>
		    requires(!etl::same_as<etl::decay_t<F>, Delegate> && !etl::is_member_function_pointer_v<etl::decay_t<F>> &&
		             etl::is_invocable_r_v<R, etl::decay_t<F>&, Args...>)
		Delegate(F&& func)
		{
			bind(etl::forward<F>(func));
		}

		Delegate(const Delegate& other) : m_manager(other.m_manager)
		{
			if (m_manager)
			{
				m_manager->copy(this, &other);
			}
		}

		Delegate(Delegate&& other) : m_manager(other.m_manager)
		{
			if (m_manager)
			{
				m_manager->move(this, &other);
				other.m_manager = nullptr;
			}
		}

		Delegate& operator=(const Delegate& other)
		{
			if (this == &other)
				return *this;

			Delegate tmp(other);

			reset();

			m_manager = tmp.m_manager;

			if (m_manager)
			{
				m_manager->move(this, &tmp);
				tmp.m_manager = nullptr;
			}

			return *this;
		}

		Delegate& operator=(Delegate&& other) noexcept
		{
			if (this == &other)
				return *this;

			reset();
			m_manager = other.m_manager;

			if (m_manager)
			{
				m_manager->move(this, &other);
				other.m_manager = nullptr;
			}

			return *this;
		}

		Delegate& operator=(FunctionPtr func) noexcept
		{
			bind(func);
			return *this;
		}

		template<typename F>
		    requires(!etl::same_as<etl::remove_cvref_t<F>, Delegate> && etl::is_invocable_r_v<R, etl::decay_t<F>&, Args...>)
		Delegate& operator=(F&& func)
		{
			bind(etl::forward<F>(func));
			return *this;
		}

		template<auto func>
		    requires(!etl::is_member_function_pointer_v<decltype(func)> && etl::is_invocable_r_v<R, decltype(func), Args...>)
		Delegate& operator=(DelegateTag<func>) noexcept
		{
			bind<func>();
			return *this;
		}

		~Delegate() { reset(); }

		void reset() noexcept
		{
			if (m_manager)
			{
				m_manager->release(this);
				m_manager = nullptr;
			}
		}

		[[nodiscard]]
		explicit operator bool() const noexcept
		{
			return m_manager != nullptr;
		}

		R operator()(Args... args) const
		{
			trinex_assert(m_manager);
			return m_manager->invoke(this, etl::forward<Args>(args)...);
		}

		template<auto func>
		    requires(!etl::is_member_function_pointer_v<decltype(func)> && etl::is_invocable_r_v<R, decltype(func), Args...>)
		void bind() noexcept
		{
			reset();

			m_manager = static_function_manager<func>();
		}

		void bind(R (*func)(Args...)) noexcept
		{
			reset();

			if (func == nullptr)
				return;

			m_context.as<R (*)(Args...)>() = func;
			m_manager                      = pointer_function_manager();
		}

		template<auto method, typename T>
		    requires(!etl::is_volatile_v<T> && etl::is_member_function_pointer_v<decltype(method)> &&
		             etl::is_invocable_r_v<R, decltype(method), T&, Args...>)
		void bind(T* object) noexcept
		{
			reset();

			if (!object)
			{
				return;
			}

			m_context.as<T*>() = object;
			m_manager          = static_method_manager<method, T>();
		}

		template<typename T, typename Method>
		    requires(etl::is_member_function_pointer_v<Method> && etl::is_invocable_r_v<R, Method, T*, Args...>)
		void bind(T* object, Method method) noexcept
		{
			reset();

			if (!object || !method)
			{
				return;
			}

			m_context.as<PointerMethodContext<Method, T>>() = {method, object};
			m_manager                                       = pointer_method_manager<T, Method>();
		}

		template<typename F>
		    requires(!etl::same_as<etl::remove_cvref_t<F>, Delegate> && etl::is_invocable_r_v<R, etl::decay_t<F>&, Args...> &&
		             !etl::is_convertible_v<etl::decay_t<F>, FunctionPtr>)
		void bind(F&& func)
		{
			using Callable = etl::decay_t<F>;

			reset();

			if constexpr (DelegateBase::is_inline_storable<Callable>)
			{
				m_context.construct<Callable>(etl::forward<F>(func));
				m_manager = static_lambda_manager<Callable>();
			}
			else
			{
				m_context.as<Callable*>() = trx_new Callable(etl::forward<F>(func));
				m_manager                 = heap_lambda_manager<Callable>();
			}
		}

		template<typename F>
		    requires(!etl::same_as<etl::remove_cvref_t<F>, Delegate> && etl::is_invocable_r_v<R, etl::decay_t<F>&, Args...> &&
		             etl::is_convertible_v<etl::decay_t<F>, FunctionPtr>)
		void bind(F&& func) noexcept
		{
			bind(static_cast<FunctionPtr>(etl::forward<F>(func)));
		}
	};

	class DelegateHandle
	{
		template<typename>
		friend class MulticastDelegate;

	private:
		void* m_node = nullptr;

		constexpr explicit DelegateHandle(void* node) noexcept : m_node(node) {}

	public:
		constexpr DelegateHandle() noexcept = default;

		[[nodiscard]]
		explicit constexpr operator bool() const noexcept
		{
			return m_node != nullptr;
		}

		friend constexpr bool operator==(DelegateHandle, DelegateHandle) noexcept = default;
	};


	template<typename>
	class MulticastDelegate;


	template<typename... Args>
	class MulticastDelegate<void(Args...)>
	{
	public:
		using DelegateType = Delegate<void(Args...)>;
		using FunctionPtr  = typename DelegateType::FunctionPtr;
		using Handle       = DelegateHandle;

	private:
		template<typename T>
		struct IsRValueReference {
			static constexpr bool value = false;
		};

		template<typename T>
		struct IsRValueReference<T&&> {
			static constexpr bool value = true;
		};

		static_assert((!IsRValueReference<Args>::value && ...), "MulticastDelegate does not support rvalue-reference arguments");

	private:
		struct Node {
			Node* next;
			DelegateType delegate;

			explicit Node(DelegateType delegate) noexcept : next(nullptr), delegate(delegate) {}
		};

	private:
		static void append(Node*& tail, Node* node) noexcept
		{
			if (!tail)
			{
				node->next = node;
				tail       = node;
				return;
			}

			node->next = tail->next;
			tail->next = node;
			tail       = node;
		}

		// Concatenates two circular lists in O(1).
		// Source becomes empty.
		static void merge(Node*& destination, Node*& source) noexcept
		{
			if (!source)
				return;

			if (!destination)
			{
				destination = source;
				source      = nullptr;
				return;
			}

			Node* destination_head = destination->next;
			Node* source_head      = source->next;

			destination->next = source_head;
			source->next      = destination_head;

			destination = source;
			source      = nullptr;
		}

		static void unlink(Node*& tail, Node* previous, Node* node) noexcept
		{
			if (node == previous)
			{
				// Only one node.
				tail = nullptr;
			}
			else
			{
				previous->next = node->next;

				if (tail == node)
					tail = previous;
			}

			node->next = nullptr;
		}

		static void destroy(Node*& tail) noexcept
		{
			if (!tail)
				return;

			Node* node = tail->next;

			tail->next = nullptr;
			tail       = nullptr;

			while (node)
			{
				Node* next = node->next;
				trx_delete node;

				node = next;
			}
		}

	private:
		class Broadcast
		{
		private:
			MulticastDelegate* m_owner;
			Broadcast* m_previous;

			// Circular list containing physically unlinked nodes.
			Node* m_garbage = nullptr;

			// Iterator state of this particular broadcast.
			Node* m_next = nullptr;
			Node* m_last = nullptr;

		public:
			explicit Broadcast(MulticastDelegate* owner) noexcept : m_owner(owner), m_previous(owner->m_broadcast)
			{
				if (owner->m_tail)
				{
					m_next = owner->m_tail->next;
					m_last = owner->m_tail;
				}

				owner->m_broadcast = this;
			}

			~Broadcast() noexcept
			{
				m_owner->m_broadcast = m_previous;

				if (m_previous)
				{
					MulticastDelegate::merge(m_previous->m_garbage, m_garbage);
				}
				else
				{
					MulticastDelegate::destroy(m_garbage);
				}
			}

			[[nodiscard]]
			Broadcast* previous() const noexcept
			{
				return m_previous;
			}

			// Transfer an already-unlinked node into garbage.
			void on_node_remove(Node* node) noexcept { append(m_garbage, node); }

			// Transfer an entire circular list into garbage.
			void on_list_remove(Node*& tail) noexcept { merge(m_garbage, tail); }

			void stop() noexcept
			{
				m_next = nullptr;
				m_last = nullptr;
			}

			// Called before a node is physically unlinked from the active list.
			void on_unlink(Node* node, Node* previous, Node* successor) noexcept
			{
				// This broadcast was going to visit this node next.
				if (m_next == node)
				{
					if (node == m_last)
					{
						// Removed node was the final pending callback.
						m_next = nullptr;
					}
					else
					{
						m_next = successor;
					}
				}

				// The removed node was the snapshot boundary.
				if (m_last == node)
				{
					if (m_next)
						m_last = previous;
					else
						m_last = nullptr;
				}
			}

			[[nodiscard]]
			Node* next() noexcept
			{
				Node* node = m_next;

				if (!node)
					return nullptr;

				if (node == m_last)
					m_next = nullptr;
				else
					m_next = node->next;

				return node;
			}
		};

	private:
		// Active circular callback list.
		Node* m_tail = nullptr;

		// Top-most currently active broadcast frame.
		Broadcast* m_broadcast = nullptr;

	private:
		void notify_unlink(Node* node, Node* previous, Node* successor) noexcept
		{
			for (Broadcast* broadcast = m_broadcast; broadcast; broadcast = broadcast->previous())
			{
				broadcast->on_unlink(node, previous, successor);
			}
		}

	public:
		constexpr MulticastDelegate() noexcept = default;

		MulticastDelegate(const MulticastDelegate&) = delete;

		MulticastDelegate& operator=(const MulticastDelegate&) = delete;

		MulticastDelegate(MulticastDelegate&&) = delete;

		MulticastDelegate& operator=(MulticastDelegate&&) = delete;

		~MulticastDelegate() { destroy(m_tail); }

	public:
		Handle add(DelegateType delegate)
		{
			if (!delegate)
				return {};

			Node* node = trx_new Node(delegate);

			// Always append directly.
			//
			// Existing broadcasts have their own m_last snapshot, so they
			// won't see this node. A nested broadcast started afterwards
			// will see it.
			append(m_tail, node);

			return Handle(node);
		}

		Handle add(FunctionPtr function)
		{
			if (!function)
				return {};

			return add(DelegateType(function));
		}

		template<auto Function>
		Handle add()
		{
			return add(DelegateType::template from<Function>());
		}

		template<auto Method, typename T>
		Handle add(T* object)
		{
			return add(DelegateType::template from<Method>(object));
		}

	public:
		bool remove(Handle handle) noexcept
		{
			if (!m_tail || !handle)
				return false;

			Node* target = static_cast<Node*>(handle.m_node);

			Node* previous = m_tail;
			Node* node     = m_tail->next;

			for (;;)
			{
				if (node == target)
				{
					Node* successor = node->next;

					if (m_broadcast)
					{
						// Update every active broadcast before node->next
						// gets reused by the garbage list.
						notify_unlink(node, previous, successor);

						unlink(m_tail, previous, node);

						m_broadcast->on_node_remove(node);
					}
					else
					{
						unlink(m_tail, previous, node);
						delete node;
					}

					return true;
				}

				if (node == m_tail)
					break;

				previous = node;
				node     = node->next;
			}

			return false;
		}

		void clear() noexcept
		{
			if (!m_tail)
				return;

			if (!m_broadcast)
			{
				destroy(m_tail);
				return;
			}

			// No currently active broadcast may continue traversing this list.
			for (Broadcast* broadcast = m_broadcast; broadcast; broadcast = broadcast->previous())
			{
				broadcast->stop();
			}

			// Move the whole active list into the current garbage collector
			// in O(1).
			m_broadcast->on_list_remove(m_tail);
		}

	public:
		[[nodiscard]]
		bool contains(Handle handle) const noexcept
		{
			if (!m_tail || !handle)
				return false;

			Node* target = static_cast<Node*>(handle.m_node);

			Node* node = m_tail->next;

			for (;;)
			{
				if (node == target)
					return true;

				if (node == m_tail)
					break;

				node = node->next;
			}

			return false;
		}

		[[nodiscard]]
		bool empty() const noexcept
		{
			return m_tail == nullptr;
		}

		[[nodiscard]]
		explicit operator bool() const noexcept
		{
			return m_tail != nullptr;
		}

	public:
		void broadcast(Args... args)
		{
			if (!m_tail)
				return;

			Broadcast state(this);
			while (Node* node = state.next())
			{
				node->delegate(args...);
			}
		}

		void operator()(Args... args) { broadcast(args...); }
	};
}// namespace Trinex
