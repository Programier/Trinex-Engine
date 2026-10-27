#pragma once

#include <Core/reflection/struct.hpp>

#include <Core/reflection/class.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api Class final : public Struct
	{
		trinex_reflection_body();

	public:
		class VTable : public Struct::VTable
		{
		};

		enum Flags : u64
		{
			IsAsset      = 1ull << 16,
			IsSingleton  = 1ull << 17,
			IsScriptable = 1ull << 18,
		};

	public:
		Class(Name name, u32 size, u32 alignment, u64 flags = 0, Array<Ref<Struct>> bases = {}, const VTable* vtable = nullptr,
		      Object* owner = nullptr);

		Class* reflection() const override { return class_of<Class>(); }

		inline bool is_asset() const { return has_flags(IsAsset); }
		inline bool is_singleton() const { return has_flags(IsSingleton); }
		inline bool is_scriptable() const { return has_flags(IsScriptable); }
		inline const VTable* vtable() const { return static_cast<const VTable*>(Struct::vtable()); }
	};
} // namespace Trinex::Refl
