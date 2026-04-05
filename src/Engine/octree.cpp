#include <Core/engine_loading_controllers.hpp>
#include <Engine/octree.hpp>

namespace Trinex
{
	struct Index {
		u8 value;

		constexpr Index(u8 idx = 0) : value(idx & 0x7) {}
		constexpr Index(bool x, bool y, bool z)
		    : value(static_cast<u8>(x) | (static_cast<u8>(y) << 1) | (static_cast<u8>(z) << 2))
		{}

		Box3i upscale(Box3i box)
		{
			const i32 sx = box.max.x - box.min.x;
			const i32 sy = box.max.y - box.min.y;
			const i32 sz = box.max.z - box.min.z;

			box.min.x -= (sx & x());
			box.min.y -= (sy & y());
			box.min.z -= (sz & z());

			box.max.x = box.min.x + (sx << 1);
			box.max.y = box.min.y + (sy << 1);
			box.max.z = box.min.z + (sz << 1);

			return box;
		}

		Box3i downscale(Box3i box)
		{
			const i32 sx = (box.max.x - box.min.x) >> 1;
			const i32 sy = (box.max.y - box.min.y) >> 1;
			const i32 sz = (box.max.z - box.min.z) >> 1;

			box.min.x += (sx & x());
			box.min.y += (sy & y());
			box.min.z += (sz & z());

			box.max.x = box.min.x + sx;
			box.max.y = box.min.y + sy;
			box.max.z = box.min.z + sz;
			return box;
		}

		FORCE_INLINE i32 x() const { return (static_cast<i32>(value) << 31) >> 31; }
		FORCE_INLINE i32 y() const { return (static_cast<i32>(value) << 30) >> 31; }
		FORCE_INLINE i32 z() const { return (static_cast<i32>(value) << 29) >> 31; }

		FORCE_INLINE Index operator!() const { return value ^ 7; }
		FORCE_INLINE operator u8() const { return value; }
	};

	static FORCE_INLINE bool is_child_of(const Box3f& parent, const Box3f& child)
	{
		return child.inside(parent) && !child.contains(parent.center());
	}

	static FORCE_INLINE Index calc_child_index(const Box3i& parent, const Box3i& child)
	{
		auto parent_center = parent.center();
		auto child_center  = child.center();
		return Index({
		        parent_center.x < child_center.x,
		        parent_center.y < child_center.y,
		        parent_center.z < child_center.z,
		});
	}

	using Node = Octree::Node;

	Node::Node(Node* owner, const Box3i& box) : m_value_ids(), m_childs{nullptr}, m_owner(owner), m_box(box), m_size(0) {}
	Node::~Node() {}

	u32 Node::insert(u32 id)
	{
		const u32 idx = static_cast<u32>(m_value_ids.size());
		m_value_ids.push_back(id);

		Node* node = this;
		while (node)
		{
			++node->m_size;
			node = node->m_owner;
		}

		return idx;
	}

	void Node::remove(Record* record, Octree* octree)
	{
		const u32 last = m_value_ids.size() - 1;
		const u32 idx  = record->index;

		if (idx != last)
		{
			const u32 ref    = m_value_ids[last];
			m_value_ids[idx] = ref;
			Record& moved    = octree->m_pool[ref];
			moved.index      = idx;
		}

		m_value_ids.pop_back();

		Node* node = this;
		while (node)
		{
			--node->m_size;
			node = node->m_owner;
		}
	}

	const Octree::Node* Octree::Node::find(const Box3i& box) const
	{
		const Node* node = this;
		while (node && !box.inside(node->box())) node = node->m_owner;

		while (node && is_child_of(node->box(), box))
		{
			node = node->child(calc_child_index(node->box(), box));
		}
		return node;
	}

	Octree::Octree()
	{
		m_root = trx_new Node(nullptr, Box3i({0, 0, 0}, {1, 1, 1}));
	}

	Octree::~Octree()
	{
		trx_delete_inline(m_root);
	}

	u32 Octree::insert(void* value, Node* node)
	{
		if (m_free_head != ~0u)
		{
			u32 id         = m_free_head;
			m_free_head    = m_pool[id].next;
			Record& record = m_pool[id];

			record.value = value;
			record.node  = node;
			record.flags = 0;
			record.index = node->insert(id);
			return id;
		}

		u32 id         = static_cast<u32>(m_pool.size());
		Record& record = m_pool.emplace_back();
		record.value   = value;
		record.node    = node;
		record.flags   = 0;
		record.index   = node->insert(id);

		return id;
	}

	Node* Octree::find_or_create(Node* head, const Box3i& box)
	{
		while (!box.inside(head->box()))
		{
			if (head == m_root)
			{
				Index index = calc_child_index(box, head->box());

				head                  = trx_new Node(nullptr, index.upscale(head->box()));
				head->m_childs[index] = m_root;
				head->m_size          = m_root->m_size;
				m_root->m_owner       = head;
				m_root                = head;
			}
			else
			{

				head = head->m_owner;
			}
		}

		while (head && head->box().size().x > 1 && is_child_of(head->box(), box))
		{
			Index index = calc_child_index(head->box(), box);

			if (head->m_childs[index] == nullptr)
			{
				head->m_childs[index] = trx_new Node(head, index.downscale(head->box()));
			}

			head = head->m_childs[index];
		}

		return head;
	}

	u32 Octree::insert(void* value, const Box3i& box)
	{
		return insert(value, find_or_create(box));
	}

	Flags<u32>& Octree::flags(u32 id)
	{
		trinex_assert(id < m_pool.size());
		return m_pool[id].flags;
	}

	Octree& Octree::update(u32 id, const Box3i& box)
	{
		trinex_assert(id < m_pool.size());

		Record& record = m_pool[id];
		Node* node     = find_or_create(record.node, box);

		if (record.node != node)
		{
			record.node->remove(&record, this);
			record.node  = node;
			record.index = node->insert(id);
		}

		return *this;
	}

	Octree& Octree::remove(u32 id)
	{
		trinex_assert(id < m_pool.size());

		Record& record = m_pool[id];
		trinex_assert(record.node);

		record.node->remove(&record, this);
		record.node  = nullptr;
		record.value = nullptr;

		record.next = m_free_head;
		m_free_head = id;
		return *this;
	}
}// namespace Trinex
