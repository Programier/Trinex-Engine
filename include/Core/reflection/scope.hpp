#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/reflection/object.hpp>

#include <Core/reflection/scope.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api Scope : public Object
	{
		trinex_reflection_body();

	private:
		struct Compare
		{
			using is_transparent = void;

			inline bool operator()(Name lhs, Name rhs) const { return lhs.id() < rhs.id(); }
			inline bool operator()(const Ref<Object>& lhs, Name rhs) const { return lhs->name().id() < rhs.id(); }
			inline bool operator()(Name lhs, const Ref<Object>& rhs) const { return lhs.id() < rhs->name().id(); }

			inline bool operator()(const Ref<Object>& lhs, const Ref<Object>& rhs) const
			{
				return lhs->name().id() < rhs->name().id();
			}
		};

	private:
		FlatSet<Ref<Object>, Compare> m_members;

	protected:
		bool register_child(Object* object) override;
		bool unregister_child(Object* object) override;

	public:
		Scope(Name name, u64 flags = 0, Object* owner = nullptr);

		static Scope* root();
		static Scope* trinex();
		static Scope* create(InitializerList<Name> path);

		Object* find(Name name, const Class* base = nullptr) const override;
		Class* reflection() const override { return class_of<Scope>(); }
		inline const Array<Ref<Object>>& members() const { return m_members.container(); }
	};
} // namespace Trinex::Refl
