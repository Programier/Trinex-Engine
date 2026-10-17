#pragma once
#include <Core/etl/flat_set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/reflection/new/object.hpp>

namespace Trinex::Refl::New
{
	class Scope : public Object
	{
	private:
		struct Compare {
			using is_transparent = void;

			inline bool operator()(Name lhs, Name rhs) const { return lhs.id() < rhs.id(); }
			inline bool operator()(const Ref<Object>& lhs, Name rhs) const { return lhs->name().id() < rhs.id(); }
			inline bool operator()(Name lhs, const Ref<Object>& rhs) const { return lhs.id() < rhs->name().id(); }

			inline bool operator()(const Ref<Object>& lhs, const Ref<Object>& rhs) const
			{
				return lhs->name().id() < rhs->name().id();
			}
		};

		FlatSet<Ref<Object>, Compare> m_members;

	protected:
		bool register_child(Object* object) override;
		bool unregister_child(Object* object) override;

	public:
		using Object::Object;

		Object* find(Name name) const override;
		const Vector<Ref<Object>>& members() const { return m_members.container(); }
	};
}// namespace Trinex::Refl::New
