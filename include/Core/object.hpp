#pragma once
#include <Core/ref_counted.hpp>

#include <Core/object.generated.hpp>

namespace Trinex
{
	trinex_reflect()
	class trinex_api Object : public RefCounted
	{
		trinex_reflection_body();

	private:
		Refl::Class* m_reflection;

	public:
		bool is(const Refl::Class* base) const;

		trinex_function()
		inline Refl::Class* reflection() const { return m_reflection; }

		template<typename T>
		inline bool is() const
		{
			return is(Refl::type_of<T>());
		}

		template<typename T>
		T* cast()
		{
			return is<T>() ? static_cast<T*>(this) : nullptr;
		}

		template<typename T>
		const T* cast() const
		{
			return is<T>() ? static_cast<const T*>(this) : nullptr;
		}

		template<typename T>
		static T* cast(Object* object)
		{
			if (object == nullptr)
				return false;

			return object->cast<T>();
		}

		template<typename T>
		static const T* cast(const Object* object)
		{
			if (object == nullptr)
				return false;

			return object->cast<T>();
		}
	};
} // namespace Trinex
