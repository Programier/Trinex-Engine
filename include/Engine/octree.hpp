#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/set.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	template<typename ElementType>
	class Octree
	{
	public:
		using ValueType = ElementType;
		struct Index {
			byte x : 1 = 0;
			byte y : 1 = 0;
			byte z : 1 = 0;

			Index(bool x, bool y, bool z) : x(x ? 1 : 0), y(y ? 1 : 0), z(z ? 1 : 0)
			{}

			Index(byte index = 0) : x((index >> 2) & 1), y((index >> 1) & 1), z(index & 1)
			{}

			Index(const Index&)            = default;
			Index& operator=(const Index&) = default;

			FORCE_INLINE byte index() const
			{
				return (x << 2) | (y << 1) | z;
			}

			FORCE_INLINE Vector3f factor() const
			{
				return {x == 1 ? 1.0f : -1.0f, y == 1 ? 1.0f : -1.0f, z == 1 ? 1.0f : -1.0f};
			}

			Index operator!() const
			{
				Index res;
				res.x = !x;
				res.y = !y;
				res.z = !z;
				return res;
			}
		};

		class Node
		{
		private:
			Node* m_childs[8];
			AABB_3Df m_box;

			Node()
			{
				for (int i = 0; i < 8; i++) m_childs[i] = nullptr;
			}

			Node(const AABB_3Df& box) : Node()
			{
				m_box = box;
			}

			Node(const Node& node)
			{
				*this = node;
			}

			Node& operator=(const Node& other)
			{
				if (this == &other)
					return *this;

				m_box  = other.m_box;
				values = other.values;

				for (int i = 0; i < 8; i++)
				{
					if (other.m_childs[i])
					{
						if (!m_childs[i])
						{
							m_childs[i] = new Octree::Node(*other.m_childs[i]);
						}
						else
						{
							(*m_childs[i]) = *other.m_childs[i];
						}
					}
					else if (m_childs[i])
					{
						delete m_childs[i];
						m_childs[i] = nullptr;
					}
				}
			}

			~Node()
			{
				for (int i = 0; i < 8; i++)
				{
					if (m_childs[i] != nullptr)
					{
						delete m_childs[i];
						m_childs[i] = nullptr;
					}
				}
			}


			FORCE_INLINE Node*& ref_child_at(Octree::Index index)
			{
				return m_childs[index.index()];
			}

		public:
			TreeSet<ElementType> values;

			FORCE_INLINE Node* child_at(Octree::Index index) const
			{
				return m_childs[index.index()];
			}

			FORCE_INLINE const AABB_3Df& box() const
			{
				return m_box;
			}

			friend class Octree;
		};

	private:
		Node* m_root_node = nullptr;
		float m_min_size  = 1.0f;

		static FORCE_INLINE bool calc_axis_offset_index(float parent, float child)
		{
			return parent < child;
		}

		static FORCE_INLINE Octree::Index calc_child_index(const AABB_3Df& parent, const AABB_3Df& child)
		{
			Octree::Index result;
			auto parent_center = parent.center();
			auto child_center  = child.center();
			result.x           = calc_axis_offset_index(parent_center.x, child_center.x);
			result.y           = calc_axis_offset_index(parent_center.y, child_center.y);
			result.z           = calc_axis_offset_index(parent_center.z, child_center.z);
			return result;
		}

		static FORCE_INLINE bool is_child_of(const AABB_3Df& parent, const AABB_3Df& child)
		{
			return child.inside(parent) && !child.contains(parent.center());
		}

	public:
		Octree(float min_size = 1.0f) : m_min_size(min_size)
		{
			m_root_node = new Octree::Node(AABB_3Df({0.0f, 0.0f, 0.0f}, {min_size, min_size, min_size}));
		}

		FORCE_INLINE Node* root_node() const
		{
			return m_root_node;
		}

		FORCE_INLINE Node* push(const AABB_3Df& box, const ElementType& element)
		{
			Node* node = find_or_create(box);
			node->values.insert(element);
			return node;
		}

		FORCE_INLINE Node* remove(const AABB_3Df& box, const ElementType& element)
		{
			Node* node = find(box);
			if (node)
			{
				node->values.erase(element);
			}

			return node;
		}

		FORCE_INLINE Node* find(const AABB_3Df& box) const
		{
			Node* node = m_root_node;

			if (!box.inside(node->m_box))
				return nullptr;

			while (node && is_child_of(node->m_box, box))
			{
				node = node->child_at(calc_child_index(node->m_box, box));
			}
			return node;
		}

		FORCE_INLINE Node* find_or_create(const AABB_3Df& box)
		{
			Node* node = m_root_node;

			while (!box.inside(node->m_box))
			{
				Octree::Index index = calc_child_index(box, node->m_box);
				node                = new Octree::Node((node->m_box * 2.0f) + (node->m_box.size() / 2.0f) * (!index).factor());
				node->m_childs[index.index()] = m_root_node;
				m_root_node                   = node;
			}

			while (node && node->m_box.size().x / 2 >= m_min_size && is_child_of(node->m_box, box))
			{
				Octree::Index index = calc_child_index(node->m_box, box);
				if (node->m_childs[index.index()] == nullptr)
				{
					node->m_childs[index.index()] =
					        new Octree::Node((node->m_box * 0.5) + (node->m_box.size() * 0.25f) * index.factor());
				}
				node = node->m_childs[index.index()];
			}

			return node;
		}

		~Octree()
		{
			delete m_root_node;
		}
	};
}// namespace Engine
