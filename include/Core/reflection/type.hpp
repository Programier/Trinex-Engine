#pragma once
#include <Core/etl/invoker.hpp>
#include <Core/reflection/object.hpp>

#include <Core/reflection/type.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api Type : public Object
	{
		trinex_reflection_body();

	public:
		struct VTable
		{
			void (*construct)(const Type* type, void* dst)                       = nullptr;
			void (*destruct)(const Type* type, void* value)                      = nullptr;
			void (*copy_construct)(const Type* type, void* dst, const void* src) = nullptr;
			void (*move_construct)(const Type* type, void* dst, void* src)       = nullptr;
			void (*copy_assign)(const Type* type, void* dst, const void* src)    = nullptr;
			void (*move_assign)(const Type* type, void* dst, void* src)          = nullptr;
			void (*swap)(const Type* type, void* lhs, void* rhs)                 = nullptr;
			void (*relocate)(const Type* type, void* dst, void* src)             = nullptr;
			bool (*equals)(const Type* type, const void* lhs, const void* rhs)   = nullptr;
			i32 (*compare)(const Type* type, const void* lhs, const void* rhs)   = nullptr;
			void (*serialize)(const Type* type, void* self, Archive& ar)         = nullptr;
			usize (*parse)(const Type* type, void* self, StringView value)       = nullptr;
			bool (*format)(const Type* type, const void* self, String& value)    = nullptr;
			usize (*parse_indirect)(const Type* type, void* result, StringView value) = nullptr;
			bool (*format_indirect)(const Type* type, const void* object, String& value) = nullptr;
		};

	private:
		u32 m_size;
		u32 m_alignment;
		const VTable* m_vtable;

		bool invoke_construct(void* mem, Type* const* args, usize count, void* context) const;
		void* invoke_allocate(Type* const* args, usize count, void* context) const;

	public:
		enum Flags : u64
		{
			IsNative                = 1ull << 0,
			IsTriviallyCopyable     = 1ull << 1,
			IsTriviallyDestructible = 1ull << 2,
			HasDefaultConstructor   = 1ull << 3,
		};

		Type(Name name, u32 size, u32 alignment, u64 flags = 0, const VTable* vtable = nullptr, Object* owner = nullptr);

		void* allocate();
		bool release(void* object);

		bool construct(void* value) const;
		bool destruct(void* value) const;
		bool copy_construct(void* dst, const void* src) const;
		bool move_construct(void* dst, void* src) const;
		bool copy_assign(void* dst, const void* src) const;
		bool move_assign(void* dst, void* src) const;
		bool swap(void* lhs, void* rhs) const;
		bool relocate(void* dst, void* src) const;

		bool equals(const void* lhs, const void* rhs) const;
		bool compare(const void* lhs, const void* rhs, i32& result) const;

		bool serialize(void* self, Archive& ar);
		usize parse(void* self, StringView value) const;
		bool format(const void* self, String& value) const;
		usize parse_indirect(void* result, StringView value) const;
		bool format_indirect(const void* object, String& value) const;

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

		template<typename... Args>
		bool construct(void* mem, Args&&... args) const
		{
			if (mem == nullptr)
				return false;

			Type* types[] = {type_of<void*>(), type_of<Args>()...};
			etl::InvokeContext<void, void*, Args...> context(mem, static_cast<Args&&>(args)...);
			return invoke_construct(mem, types, sizeof...(Args) + 1, context);
		}

		template<typename... Args>
		void* allocate(Args&&... args) const
		{
			Type* types[] = {type_of<void*>(), type_of<Args>()...};
			etl::InvokeContext<void, void*, Args...> context(nullptr, static_cast<Args&&>(args)...);
			return invoke_allocate(types, sizeof...(Args) + 1, context);
		}
	};
} // namespace Trinex::Refl
