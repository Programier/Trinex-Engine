#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/vector.hpp>

namespace Engine
{
	template<typename Signature>
	using CallBack = Function<Signature>;

	template<typename Signature>
	class CallBacks final
	{
	private:
		struct FuncNode {
			Function<Signature> function;
			FuncNode* prev = nullptr;
			FuncNode* next = nullptr;

			FORCE_INLINE FuncNode(const Function<Signature>& func) : function(func) {}
			FORCE_INLINE FuncNode(Function<Signature>&& func) : function(std::move(func)) {}
		};

		FuncNode* m_head = nullptr;

		FORCE_INLINE Identifier push(FuncNode* func)
		{
			if (m_head)
			{
				func->next   = m_head;
				m_head->prev = func;
			}

			m_head = func;
			return reinterpret_cast<Identifier>(func);
		}

	public:
		template<typename NodeType, typename FunctionType>
		class basic_iterator
		{
		protected:
			NodeType* m_current = nullptr;

		public:
			using value_type        = FunctionType;
			using pointer           = FunctionType*;
			using reference         = FunctionType&;
			using iterator_category = std::forward_iterator_tag;

			basic_iterator() = default;
			explicit basic_iterator(NodeType* node) : m_current(node) {}

			reference operator*() const { return m_current->function; }
			pointer operator->() const { return &m_current->function; }

			basic_iterator& operator++()
			{
				if (m_current)
					m_current = m_current->next;
				return *this;
			}

			basic_iterator operator++(int)
			{
				basic_iterator temp = *this;
				++(*this);
				return temp;
			}

			bool operator==(const basic_iterator& other) const noexcept { return m_current == other.m_current; }
			bool operator!=(const basic_iterator& other) const noexcept { return m_current != other.m_current; }
		};

		using iterator       = basic_iterator<FuncNode, Function<Signature>>;
		using const_iterator = basic_iterator<const FuncNode, const Function<Signature>>;


		FORCE_INLINE CallBacks() = default;
		FORCE_INLINE CallBacks(const CallBacks& callbacks) { (*this) = callbacks; }
		FORCE_INLINE CallBacks(CallBacks&& callbacks) { (*this) = std::move(callbacks); }

		iterator begin() noexcept { return iterator(m_head); }
		iterator end() noexcept { return iterator(nullptr); }
		const_iterator begin() const noexcept { return const_iterator(m_head); }
		const_iterator end() const noexcept { return const_iterator(nullptr); }
		const_iterator cbegin() const noexcept { return const_iterator(m_head); }
		const_iterator cend() const noexcept { return const_iterator(nullptr); }

		inline bool empty() const { return m_head == nullptr; }
		inline Identifier push(const Function<Signature>& callback) { return push(trx_new FuncNode(callback)); }
		inline Identifier push(Function<Signature>&& callback) { return push(trx_new FuncNode(std::move(callback))); }
		inline Identifier operator+=(const Function<Signature>& func) { return push(func); }
		inline Identifier operator+=(Function<Signature>&& func) { return push(func); }

		CallBacks& operator=(const CallBacks& callbacks)
		{
			if (this == &callbacks)
				return *this;

			clear();

			for (auto& func : callbacks)
			{
				push(func);
			}
			return *this;
		}

		CallBacks& operator=(CallBacks&& callbacks)
		{
			if (this == &callbacks)
				return *this;

			clear();
			m_head           = callbacks.m_head;
			callbacks.m_head = nullptr;
			return *this;
		}

		inline CallBacks& remove(Identifier ID)
		{
			FuncNode* remove = reinterpret_cast<FuncNode*>(ID);

			if (remove->prev)
				remove->prev->next = remove->next;
			else
				m_head = remove->next;

			if (remove->next)
				remove->next->prev = remove->prev;

			trx_delete remove;
			return *this;
		}

		inline CallBacks& clear()
		{
			while (m_head)
			{
				FuncNode* next = m_head->next;
				trx_delete m_head;
				m_head = next;
			}
			return *this;
		}

		template<typename... Args>
		const CallBacks& trigger(Args&&... args) const
		{
			FuncNode* node = m_head;
			while (node)
			{
				node->function(std::forward<Args>(args)...);
				node = node->next;
			}
			return *this;
		}

		template<typename... Args>
		const CallBacks& operator()(Args&&... args) const
		{
			return trigger(std::forward<Args>(args)...);
		}

		~CallBacks() { clear(); }
	};
}// namespace Engine
