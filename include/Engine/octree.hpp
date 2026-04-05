#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/flags.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/box.hpp>

namespace Trinex
{
	class ENGINE_EXPORT Octree
	{
	private:
		struct Record;

	public:
		class Node
		{
		private:
			Vector<u32, Allocator<u32>> m_value_ids;
			Node* m_childs[8];
			Node* m_owner;
			Box3i m_box;
			usize m_size;

		private:
			trinex_non_copyable(Node);
			trinex_non_moveable(Node);

			Node(Node* owner, const Box3i& box);
			~Node();

			u32 insert(u32 id);
			void remove(Record* record, Octree* octree);

		public:
			const Node* find(const Box3i& box) const;

			inline const Vector<u32, Allocator<u32>>& value_ids() const { return m_value_ids; }
			inline Node* owner() const { return m_owner; }
			inline Node* child(u8 index) const { return m_childs[index]; }
			inline const Box3i& box() const { return m_box; }
			inline usize size() const { return m_size; }
			inline bool empty() const { return m_size == 0; }
			inline Node* find(const Box3i& box) { return const_cast<Node*>(const_cast<const Node*>(this)->find(box)); }

			friend class Octree;
		};

	private:
		struct Record {
			void* value;
			Node* node;
			Flags<u32> flags;

			union
			{
				u32 next;
				u32 index;
			};
		};

	private:
		Node* m_root = nullptr;

		Vector<Record, Allocator<Record>> m_pool;
		u32 m_free_head = ~0u;

	private:
		u32 insert(void* value, Node* node);
		Node* find_or_create(Node* start, const Box3i& box);

	public:
		Octree();
		~Octree();

		u32 insert(void* value, const Box3i& box);
		Flags<u32>& flags(u32 id);
		Octree& update(u32 id, const Box3i& box);
		Octree& remove(u32 id);

		template<typename T = void>
		inline T* value(u32 id) const
		{
			return static_cast<T*>(m_pool[id].value);
		}

		inline Node* find_or_create(const Box3i& box) { return find_or_create(m_root, box); }
		inline Node* root() const { return m_root; }
		inline bool is_valid(u32 id) const { return id < m_pool.size() && m_pool[id].value; }
		inline Node* find(const Box3i& box) const { return m_root->find(box); }
		inline usize size() const { return m_root->size(); }
		inline bool empty() const { return m_root->empty(); }
	};
}// namespace Trinex
