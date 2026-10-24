#pragma once

#include <Core/reflection/new/of.hpp>

namespace Trinex::Refl::New
{
	template<typename Self>
	class Binder
	{
	private:
		static void bind(Type*& field) { field = type_of<Self>(); }
		static void bind(Struct*& field) { field = struct_of<Self>(); }
		static void bind(Class*& field) { field = class_of<Self>(); }
		static void bind(Type* (*&field)()) { field = &type_of<Self>; }
		static void bind(Struct* (*&field)()) { field = &struct_of<Self>; }
		static void bind(Class* (*&field)()) { field = &class_of<Self>; }

		static void bind(Type* (*&field)(const void*) )
		{
			field = [](const void*) -> Type* { return type_of<Self>(); };
		}

		static void bind(Struct* (*&field)(const void*) )
		{
			field = [](const void*) -> Struct* { return struct_of<Self>(); };
		}

		static void bind(Class* (*&field)(const void*) )
		{
			field = [](const void*) -> Class* { return class_of<Self>(); };
		}

	public:
		Binder(Self* instance)
		{
			if constexpr (requires { instance->m_reflection; })
			{
				bind(instance->m_reflection);
			}
			else if constexpr (requires { instance->reflection; })
			{
				bind(instance->reflection);
			}
		}
	};
}// namespace Trinex::Refl::New
