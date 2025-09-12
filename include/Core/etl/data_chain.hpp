#pragma once
#include <Core/etl/flat_set.hpp>

namespace Engine
{
	template<typename T, typename Compare = std::less<T>, size_t chunk_size = 64, typename AllocatorType = Allocator<T>>
	class DataChain
	{
	private:
		struct Node {
			const Node* prev;
			T data;

			inline bool less(const Node& node) const noexcept
			{
				if (prev != node.prev)
					return prev < node.prev;
				return Compare()(data, node.data);
			}
		};

		struct NodeCompare {
			bool operator()(const Node* const a, const Node* const b) const noexcept { return a->less(*b); }
		};

		using NodeAllocator         = typename AllocatorType::template rebind<Node>::other;
		using NodePtrAllocator      = typename AllocatorType::template rebind<Node*>::other;
		using ConstNodePtrAllocator = typename AllocatorType::template rebind<const Node*>::other;

	private:
		Vector<Node*, NodePtrAllocator> m_chunks;
		FlatSet<const Node*, NodeCompare, ConstNodePtrAllocator> m_storage;

	private:
		const Node* allocate(const Node& node)
		{
			uint64_t index = m_storage.size() % chunk_size;

			if (index == 0)
				m_chunks.push_back(NodeAllocator().allocate(chunk_size));
			Node* result = m_chunks.back() + index;
			return new (result) Node(node);
		}

		const Node* find_node(const Node& node)
		{
			auto it = m_storage.find(&node);
			if (it == m_storage.end())
			{
				it = m_storage.insert(allocate(node)).first;
			}
			return *it;
		}

		void destruct_chunk(uint64_t index, uint64_t count)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				Node* chunk = m_chunks[index];

				while (count > 0)
				{
					std::destroy_at(&chunk->data);
					--count;
					++chunk;
				}
			}
		}

		void destroy_chunk(uint64_t index, uint64_t count)
		{
			destruct_chunk(index, count);
			NodeAllocator().deallocate(m_chunks[index]);
		}

	public:
		class Link
		{
		private:
			DataChain* m_chain;
			const Node* m_node;

			Link(DataChain* chain, const Node* node) : m_chain(chain), m_node(node) {}

		public:
			Link(DataChain* chain = nullptr, uint64_t id = 0) : m_chain(chain), m_node(reinterpret_cast<const Node*>(id)) {}

			Link next(const T& value)
			{
				Node n{m_node, value};
				return {m_chain, m_chain->find_node(n)};
			}

			Link prev() const { return {m_chain, m_node->prev}; }
			const T& data() const { return m_node->data; }
			uint64_t id() const { return reinterpret_cast<uint64_t>(m_node); }
			bool empty() const { return m_node == nullptr; }
			bool valid() const { return m_node != nullptr; }

			bool operator==(const Link& other) const { return m_chain == other.m_chain && m_node == other.m_node; }
			bool operator!=(const Link& other) const { return !(*this == other); }

			friend class DataChain;
		};

		Link link() { return Link{this, 0}; }
		Link link(const T& value) { return Link{this, find_node({nullptr, value})}; }
		Link link(const T* values, size_t size)
		{
			if (size > 0)
			{
				Link result = Link(this, find_node({nullptr, values[0]}));
				for (size_t i = 1; i < size; ++i) result = result.next(values[i]);
				return result;
			}
			return {this, nullptr};
		}

		void clear()
		{
			size_t chunks_count = m_chunks.size();

			if (chunks_count > 0)
			{
				destruct_chunk(--chunks_count, m_storage.size() % chunk_size);
				for (uint64_t i = 0; i < chunks_count; ++i) destruct_chunk(i, chunk_size);
				m_storage.clear();
			}
		}

		size_t size() const { return m_storage.size(); }

		~DataChain()
		{
			size_t chunks_count = m_chunks.size();

			if (chunks_count > 0)
			{
				destroy_chunk(--chunks_count, m_storage.size() % chunk_size);
				for (uint64_t i = 0; i < chunks_count; ++i) destroy_chunk(i, chunk_size);
			}
		}
	};

}// namespace Engine
