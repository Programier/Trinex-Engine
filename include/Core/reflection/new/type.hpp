#pragma once
#include <Core/reflection/new/object.hpp>

#include <Core/reflection/new/type.generated.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect()
	class trinex_api Type : public Object
	{
		trinex_reflection_body();

	public:
		struct VTable
		{
			void (*construct)(const Type* type, void* dst, usize count)                       = nullptr;
			void (*destruct)(const Type* type, void* value, usize count)                      = nullptr;
			void (*copy_construct)(const Type* type, void* dst, const void* src, usize count) = nullptr;
			void (*move_construct)(const Type* type, void* dst, void* src, usize count)       = nullptr;
			void (*copy_assign)(const Type* type, void* dst, const void* src, usize count)    = nullptr;
			void (*move_assign)(const Type* type, void* dst, void* src, usize count)          = nullptr;
			void (*swap)(const Type* type, void* lhs, void* rhs, usize count)                 = nullptr;
			void (*relocate)(const Type* type, void* dst, void* src, usize count)             = nullptr;
			bool (*equals)(const Type* type, const void* lhs, const void* rhs)                = nullptr;
			i32 (*compare)(const Type* type, const void* lhs, const void* rhs)                = nullptr;
			usize (*parse)(const Type* type, void* self, StringView value)                    = nullptr;
			bool (*format)(const Type* type, const void* self, String& value)                 = nullptr;
		};

	private:
		u32 m_size;
		u32 m_alignment;
		const VTable* m_vtable;

	public:
		enum Flags : u64
		{
			IsNative                = 1ull << 0,
			IsTriviallyCopyable     = 1ull << 1,
			IsTriviallyDestructible = 1ull << 2,
			HasDefaultConstructor   = 1ull << 3,
		};

		Type(Name name, u32 size, u32 alignment, u64 flags = 0, const VTable* vtable = nullptr, Object* owner = nullptr);

		bool construct(void* dst, usize count = 1) const;
		bool destruct(void* value, usize count = 1) const;
		bool copy_construct(void* dst, const void* src, usize count = 1) const;
		bool move_construct(void* dst, void* src, usize count = 1) const;
		bool copy_assign(void* dst, const void* src, usize count = 1) const;
		bool move_assign(void* dst, void* src, usize count = 1) const;
		bool swap(void* lhs, void* rhs, usize count = 1) const;
		bool relocate(void* dst, void* src, usize count = 1) const;

		bool equals(const void* lhs, const void* rhs) const;
		bool compare(const void* lhs, const void* rhs, i32& result) const;

		usize parse(void* self, StringView value) const;
		bool format(const void* self, String& value) const;

		Class* reflection() const override { return class_of<Type>(); }

		inline u32 size() const { return m_size; }
		inline u32 alignment() const { return m_alignment; }
		inline const VTable* vtable() const { return m_vtable; }
		inline bool has_vtable() const { return m_vtable != nullptr; }
		inline bool is_void() const { return m_size == 0 || m_alignment == 0; }
		inline bool is_native() const { return has_flags(IsNative); }
		inline bool is_trivially_copyable() const { return has_flags(IsTriviallyCopyable); }
		inline bool is_trivially_destructible() const { return has_flags(IsTriviallyDestructible); }
		inline bool has_default_constructor() const { return has_flags(HasDefaultConstructor); }
	};
} // namespace Trinex::Refl::New
