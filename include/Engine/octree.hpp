#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/set.hpp>
#include <Core/math/box.hpp>

namespace Engine
{
	template<typename ElementType, typename HashType = Hash<ElementType>, typename Pred = std::equal_to<ElementType>>
	class Octree
	{
	public:
		using ValueType = ElementType;

		struct Index {
			union
			{
				struct {
					byte x : 1;
					byte y : 1;
					byte z : 1;
				};

				byte index : 3 = 0;
			};

			Index(bool x, bool y, bool z) : x(x), y(y), z(z) {}
			Index(byte index = 0) : index(index) {}

			Index(const Index&)            = default;
			Index& operator=(const Index&) = default;

			FORCE_INLINE Vector3f factor() const
			{
				return {
				        2.0f * static_cast<float>(x) - 1.0f,
				        2.0f * static_cast<float>(y) - 1.0f,
				        2.0f * static_cast<float>(z) - 1.0f,
				};
			}

			FORCE_INLINE Index operator!() const { return ~index; }
		};

		class Node
		{
		private:
			static FORCE_INLINE Octree::Index calc_child_index(const Box3f& parent, const Box3f& child)
			{
				auto parent_center = parent.center();
				auto child_center  = child.center();

				return Octree::Index({
				        parent_center.x < child_center.x,
				        parent_center.y < child_center.y,
				        parent_center.z < child_center.z,
				});
			}

			static FORCE_INLINE bool is_child_of(const Box3f& parent, const Box3f& child)
			{
				return child.inside(parent) && !child.contains(parent.center());
			}

		private:
			Set<ElementType, HashType, Pred> m_values;
			Node* m_childs[8];
			Node* m_owner;
			Box3f m_box;
			size_t m_size;

			Node(Node* owner, const Box3f& box) : m_values(), m_childs{nullptr}, m_owner(owner), m_box(box), m_size(0) {}

			trinex_non_copyable(Node);
			trinex_non_moveable(Node);

			~Node()
			{
				for (byte i = 0; i < 8; i++)
				{
					if (m_childs[i] != nullptr)
					{
						delete m_childs[i];
						m_childs[i] = nullptr;
					}
				}
			}

			inline Node& on_value_add()
			{
				Node* node = this;

				while (node)
				{
					++node->m_size;
					node = node->m_owner;
				}

				return *this;
			}

			inline Node& on_value_remove()
			{
				Node* node = this;

				while (node)
				{
					--node->m_size;
					node = node->m_owner;
				}
				return *this;
			}

		public:
			inline Node& add(const ElementType& element)
			{
				if (m_values.insert(element).second)
					return on_value_add();
				return *this;
			}

			inline Node& add(ElementType&& element)
			{
				if (m_values.erase(std::move(element)).second)
					return on_value_add();
				return *this;
			}

			inline Node& remove(const ElementType& element)
			{
				if (m_values.erase(element))
					return on_value_remove();
				return *this;
			}

			FORCE_INLINE const Set<ElementType>& values() const { return m_values; }
			FORCE_INLINE Node* owner() { return m_owner; }
			FORCE_INLINE const Node* owner() const { return m_owner; }
			FORCE_INLINE Node* child_at(Octree::Index index) { return m_childs[index.index]; }
			FORCE_INLINE const Node* child_at(Octree::Index index) const { return m_childs[index.index]; }
			FORCE_INLINE const Box3f& box() const { return m_box; }
			FORCE_INLINE size_t size() const { return m_size; }
			FORCE_INLINE bool empty() const { return m_size == 0; }

			FORCE_INLINE Node* find(const Box3f& box) { return const_cast<Node*>(const_cast<const Node*>(this)->find(box)); }

			inline const Node* find(const Box3f& box) const
			{
				const Node* node = this;

				while (node && !box.inside(node->m_box)) node = node->m_owner;

				while (node && is_child_of(node->m_box, box))
				{
					node = node->child_at(calc_child_index(node->m_box, box));
				}
				return node;
			}
			friend class Octree;
		};

	private:
		Node* m_root_node = nullptr;

	public:
		Octree() { m_root_node = new Octree::Node(nullptr, Box3f({0.0f, 0.0f, 0.0f}, {1.f, 1.f, 1.f})); }
		FORCE_INLINE Node* root_node() const { return m_root_node; }

		FORCE_INLINE Node* push(const Box3f& box, const ElementType& element)
		{
			Node* node = find_or_create(box);
			return &node->add(element);
		}

		FORCE_INLINE Node* remove(const Box3f& box, const ElementType& element)
		{
			Node* node = find(box);

			if (node)
				return &node->remove(element);

			return node;
		}

		FORCE_INLINE Node* find(const Box3f& box) const { return m_root_node->find(box); }

		FORCE_INLINE Node* find_or_create(const Box3f& box)
		{
			Node* node = m_root_node;

			while (!box.inside(node->m_box))
			{
				Octree::Index index = Node::calc_child_index(box, node->m_box);
				node = new Octree::Node(nullptr, (node->m_box * 2.0f) + (node->m_box.size() / 2.0f) * (!index).factor());
				node->m_childs[index.index] = m_root_node;
				node->m_size                = m_root_node->m_size;
				m_root_node->m_owner        = node;
				m_root_node                 = node;
			}

			while (node && node->m_box.size().x / 2 >= 1.f && Node::is_child_of(node->m_box, box))
			{
				Octree::Index index = Node::calc_child_index(node->m_box, box);
				if (node->m_childs[index.index] == nullptr)
				{
					node->m_childs[index.index] =
					        new Octree::Node(node, (node->m_box * 0.5) + (node->m_box.size() * 0.25f) * index.factor());
				}
				node = node->m_childs[index.index];
			}

			return node;
		}

		FORCE_INLINE size_t size() const { return m_root_node->size(); }
		FORCE_INLINE bool empty() const { return m_root_node->empty(); }

		~Octree() { delete m_root_node; }
	};
}// namespace Engine
