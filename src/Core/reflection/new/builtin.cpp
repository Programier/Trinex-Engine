#include <Core/reflection/new/of.hpp>
#include <Core/reflection/new/type.hpp>
#include <Core/reflection/new/vtable.hpp>

namespace Trinex::Refl::New
{
	template<typename T>
	static const Type* type_of_internal(Name name)
	{
		static NativeTypeVTable<T, Type::VTable> vtable;
		static Type type(name, sizeof(T), alignof(T), 0, &vtable, Object::root());
		return &type;
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<bool>()
	{
		return type_of_internal<bool>("bool");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<i8>()
	{
		return type_of_internal<i8>("i8");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<u8>()
	{
		return type_of_internal<u8>("u8");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<i16>()
	{
		return type_of_internal<i16>("i16");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<u16>()
	{
		return type_of_internal<u16>("u16");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<i32>()
	{
		return type_of_internal<i32>("i32");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<u32>()
	{
		return type_of_internal<u32>("u32");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<i64>()
	{
		return type_of_internal<i64>("i64");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<u64>()
	{
		return type_of_internal<u64>("u64");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<f32>()
	{
		return type_of_internal<f32>("f32");
	}

	template<>
	FORCE_ENGINE_EXPORT const Type* type_of<f64>()
	{
		return type_of_internal<f64>("f64");
	}

	trinex_on_reflection_init()
	{
		type_of<bool>();
		type_of<i8>();
		type_of<u8>();
		type_of<i16>();
		type_of<u16>();
		type_of<i32>();
		type_of<u32>();
		type_of<i64>();
		type_of<u64>();
		type_of<f32>();
		type_of<f64>();
	}
}// namespace Trinex::Refl::New
