#pragma once
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>

namespace Engine
{
	class Object;

	class ENGINE_EXPORT ObjectTreeNodeStatics
	{
	private:
		using iterator       = Object**;
		using const_iterator = Object* const*;
		static bool lower_bound(iterator begin, iterator end, iterator& out, Object* object, Refl::Class* check_class,
		                        const char* process);
		static Object* find(const_iterator begin, const_iterator end, StringView full_name);
		static Object* find(const_iterator begin, const_iterator end, Name name);


		template<typename Super, typename Element = Object>
		class DataHolder : public Super
		{
		public:
			using ChildsArray = Vector<Element*>;

		protected:
			ChildsArray m_child_objects;

			bool register_child(Object* child) override { return true; }

			bool unregister_child(Object* child) override { return true; }

		public:
			const ChildsArray& child_objects() const { return m_child_objects; }

			~DataHolder()
			{
				for (Element* child : m_child_objects)
				{
					child->owner(nullptr);
				}
			}
		};


		template<typename Super, typename Element>
		friend class ObjectTreeNode;
	};

	template<typename Super, typename Element = Object>
	class ObjectTreeNode : public ObjectTreeNodeStatics::DataHolder<Super, Element>
	{
		using Holder = ObjectTreeNodeStatics::DataHolder<Super, Element>;

	protected:
		virtual Refl::Class* object_tree_child_class() const = 0;

		bool register_child(Object* child) override
		{
			Object** begin = reinterpret_cast<Object**>(Holder::m_child_objects.begin());
			Object** end   = reinterpret_cast<Object**>(Holder::m_child_objects.end());
			Object** place = nullptr;

			if (!ObjectTreeNodeStatics::lower_bound(begin, end, place, child, object_tree_child_class(), "register"))
				return false;

			Holder::m_child_objects.insert(reinterpret_cast<Element**>(place), Super::template instance_cast<Element>(child));
			return true;
		}

		bool unregister_child(Object* child) override
		{
			Object** begin = reinterpret_cast<Object**>(Holder::m_child_objects.begin());
			Object** end   = reinterpret_cast<Object**>(Holder::m_child_objects.end());
			Object** place = nullptr;

			if (!ObjectTreeNodeStatics::lower_bound(begin, end, place, child, object_tree_child_class(), "unregister"))
				return false;

			if (place != end)
			{
				Holder::m_child_objects.erase(reinterpret_cast<Element**>(place));
				return true;
			}

			return false;
		}

	public:
		Object* find_child_object(StringView name) const override
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_child_objects.begin());
			It end   = reinterpret_cast<It>(Holder::m_child_objects.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name);
		}

		Object* find_child_object(Name name) const
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_child_objects.begin());
			It end   = reinterpret_cast<It>(Holder::m_child_objects.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name.to_string());
		}
	};
}// namespace Engine
