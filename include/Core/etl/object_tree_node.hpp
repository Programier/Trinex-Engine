#pragma once
#include <Core/etl/vector.hpp>
#include <Core/types/name.hpp>

namespace Trinex
{
	class Object;

	class ENGINE_EXPORT ObjectTreeNodeStatics
	{
	private:
		using iterator       = Object**;
		using const_iterator = Object* const*;
		static bool lower_bound(iterator begin, iterator end, iterator& out, Object* object, Refl::Class* check_class);
		static Object* find(const_iterator begin, const_iterator end, StringView full_name);
		static Object* find(const_iterator begin, const_iterator end, Name name);


		template<typename Super, typename Element = Object>
		class DataHolder : public Super
		{
		public:
			using Container = Vector<Element*>;

		protected:
			Container m_childs;

		public:
			const Container& childs() const { return m_childs; }

			DataHolder& release_childs()
			{
				while (!m_childs.empty())
				{
					m_childs.back()->owner(nullptr);
				}

				return *this;
			}

			~DataHolder()
			{
				for (Element* element : m_childs)
				{
					element->owner(nullptr);
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
		Object* register_child(Object* child, u32& index) override
		{
			Object** begin = reinterpret_cast<Object**>(Holder::m_childs.begin());
			Object** end   = reinterpret_cast<Object**>(Holder::m_childs.end());
			Object** place = nullptr;

			if (!ObjectTreeNodeStatics::lower_bound(begin, end, place, child, Element::static_reflection()))
				return Super::register_child(child, index);

			Holder::m_childs.insert(reinterpret_cast<Element**>(place), Super::template instance_cast<Element>(child));
			return this;
		}

		bool unregister_child(Object* child) override
		{
			Object** begin = reinterpret_cast<Object**>(Holder::m_childs.begin());
			Object** end   = reinterpret_cast<Object**>(Holder::m_childs.end());
			Object** place = nullptr;

			if (!ObjectTreeNodeStatics::lower_bound(begin, end, place, child, Element::static_reflection()))
				return Super::unregister_child(child);

			if (place != end)
			{
				Holder::m_childs.erase(reinterpret_cast<Element**>(place));
				return true;
			}

			return Super::unregister_child(child);
		}

	public:
		Object* find_child_object(StringView name) const override
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_childs.begin());
			It end   = reinterpret_cast<It>(Holder::m_childs.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name);
		}

		Object* find_child_object(Name name) const
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_childs.begin());
			It end   = reinterpret_cast<It>(Holder::m_childs.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name.to_string());
		}
	};
}// namespace Trinex
