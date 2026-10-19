#include <Core/reflection/new/native.hpp>
#include <Core/reflection/new/of.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	template<typename T>
	static Type* type_of_internal(Name name)
	{
		static NativeTypeVTable<T, Type::VTable> vtable;
		static Type type(name, sizeof(T), alignof(T), 0, &vtable, Object::root());
		return &type;
	}

	template<>
	trinex_api Type* TypeOf<void>::resolve()
	{
		return nullptr;
	}

	template<>
	trinex_api Type* TypeOf<bool>::resolve()
	{
		return type_of_internal<bool>("bool");
	}

	template<>
	trinex_api Type* TypeOf<i8>::resolve()
	{
		return type_of_internal<i8>("i8");
	}

	template<>
	trinex_api Type* TypeOf<u8>::resolve()
	{
		return type_of_internal<u8>("u8");
	}

	template<>
	trinex_api Type* TypeOf<i16>::resolve()
	{
		return type_of_internal<i16>("i16");
	}

	template<>
	trinex_api Type* TypeOf<u16>::resolve()
	{
		return type_of_internal<u16>("u16");
	}

	template<>
	trinex_api Type* TypeOf<i32>::resolve()
	{
		return type_of_internal<i32>("i32");
	}

	template<>
	trinex_api Type* TypeOf<u32>::resolve()
	{
		return type_of_internal<u32>("u32");
	}

	template<>
	trinex_api Type* TypeOf<i64>::resolve()
	{
		return type_of_internal<i64>("i64");
	}

	template<>
	trinex_api Type* TypeOf<u64>::resolve()
	{
		return type_of_internal<u64>("u64");
	}

	template<>
	trinex_api Type* TypeOf<f32>::resolve()
	{
		return type_of_internal<f32>("f32");
	}

	template<>
	trinex_api Type* TypeOf<f64>::resolve()
	{
		return type_of_internal<f64>("f64");
	}

	trinex_on_reflection_init()
	{
		TypeOf<bool>::resolve();
		TypeOf<i8>::resolve();
		TypeOf<u8>::resolve();
		TypeOf<i16>::resolve();
		TypeOf<u16>::resolve();
		TypeOf<i32>::resolve();
		TypeOf<u32>::resolve();
		TypeOf<i64>::resolve();
		TypeOf<u64>::resolve();
		TypeOf<f32>::resolve();
		TypeOf<f64>::resolve();
	}
}// namespace Trinex::Refl::New
