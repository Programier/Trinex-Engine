#pragma once

#include <Core/reflection/new/struct.hpp>

namespace Trinex::Refl::New
{
	class trinex_api Class final : public Struct
	{
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
		Class(Name name, u32 size, u32 alignment, u64 flags = 0, const Class* base = nullptr, const VTable* vtable = nullptr,
		      Object* owner = nullptr);

		inline const Class* base() const { return static_cast<const Class*>(Struct::base()); }
		inline bool is_asset() const { return has_flags(IsAsset); }
		inline bool is_singleton() const { return has_flags(IsSingleton); }
		inline bool is_scriptable() const { return has_flags(IsScriptable); }
		inline const VTable* vtable() const { return static_cast<const VTable*>(Struct::vtable()); }
	};
}// namespace Trinex::Refl::New
