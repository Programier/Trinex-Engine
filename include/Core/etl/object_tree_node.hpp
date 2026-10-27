#pragma once
#include <Core/etl/array.hpp>
#include <Core/types/name.hpp>

namespace Trinex
{
	class DeprecatedObject;

	class trinex_api ObjectTreeNodeStatics
	{
	private:
		using iterator       = DeprecatedObject**;
		using const_iterator = DeprecatedObject* const*;
		static bool lower_bound(iterator begin, iterator end, iterator& out, DeprecatedObject* object, Refl::Deprecated::Class* check_class);
		static DeprecatedObject* find(const_iterator begin, const_iterator end, StringView full_name);
		static DeprecatedObject* find(const_iterator begin, const_iterator end, Name name);

		template<typename Super, typename Element = DeprecatedObject>
		class DataHolder : public Super
		{
		public:
			using Container = Array<Element*>;

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

	template<typename Super, typename Element = DeprecatedObject>
	class ObjectTreeNode : public ObjectTreeNodeStatics::DataHolder<Super, Element>
	{
		using Holder = ObjectTreeNodeStatics::DataHolder<Super, Element>;

	protected:
		DeprecatedObject* register_child(DeprecatedObject* child, u32& index) override
		{
			DeprecatedObject** begin = reinterpret_cast<DeprecatedObject**>(Holder::m_childs.begin());
			DeprecatedObject** end   = reinterpret_cast<DeprecatedObject**>(Holder::m_childs.end());
			DeprecatedObject** place = nullptr;

			if (!ObjectTreeNodeStatics::lower_bound(begin, end, place, child, Element::static_reflection()))
				return Super::register_child(child, index);

			Holder::m_childs.insert(reinterpret_cast<Element**>(place), Super::template instance_cast<Element>(child));
			return this;
		}

		bool unregister_child(DeprecatedObject* child) override
		{
			DeprecatedObject** begin = reinterpret_cast<DeprecatedObject**>(Holder::m_childs.begin());
			DeprecatedObject** end   = reinterpret_cast<DeprecatedObject**>(Holder::m_childs.end());
			DeprecatedObject** place = nullptr;

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
		DeprecatedObject* find_child_object(StringView name) const override
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_childs.begin());
			It end   = reinterpret_cast<It>(Holder::m_childs.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name);
		}

		DeprecatedObject* find_child_object(Name name) const
		{
			using It = ObjectTreeNodeStatics::const_iterator;
			It begin = reinterpret_cast<It>(Holder::m_childs.begin());
			It end   = reinterpret_cast<It>(Holder::m_childs.end());

			if (auto result = ObjectTreeNodeStatics::find(begin, end, name))
				return result;

			return Super::find_child_object(name.to_string());
		}
	};
} // namespace Trinex
