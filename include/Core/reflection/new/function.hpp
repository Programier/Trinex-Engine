#pragma once

#include <Core/etl/vector.hpp>
#include <Core/reflection/new/object.hpp>


namespace Trinex::Refl::New
{
	class Type;

	class Function : public Object
	{
	public:
		struct Argument {
			const Type* type = nullptr;
			void* data       = nullptr;
		};

		enum class InvokeResult : u8
		{
			Success,
			MissingVTable,
			InvalidContext,
			InvalidArgumentCount,
			InvalidArgumentType,
			InvalidArgumentStorage,
			MissingReturnStorage,
		};

		class VTable
		{
		public:
			virtual ~VTable() = default;
			virtual InvokeResult invoke(void* context, const Argument* args, usize argument_count, void* return_value) const = 0;
		};

		struct Parameter {
			enum Flags : u64
			{
				IsConst    = 1ull << 0,
				IsRef      = 1ull << 1,
				IsOut      = 1ull << 2,
				IsVariadic = 1ull << 3,
			};

			Name name;
			const Type* type = nullptr;
			u64 flags        = 0;

			bool has_flags(u64 value) const { return (flags & value) == value; }
			bool is_const() const { return has_flags(IsConst); }
			bool is_ref() const { return has_flags(IsRef); }
			bool is_out() const { return has_flags(IsOut); }
			bool is_variadic() const { return has_flags(IsVariadic); }
		};

		enum Flags : u64
		{
			IsConst       = 1ull << 8,
			IsStatic      = 1ull << 9,
			IsVirtual     = 1ull << 10,
			IsPureVirtual = 1ull << 11,
			IsNoexcept    = 1ull << 12,
			IsConstructor = 1ull << 13,
			IsDestructor  = 1ull << 14,
		};

	private:
		const Type* m_type;
		Vector<Parameter> m_args;
		const VTable* m_vtable;

	public:
		Function(Name name, const Type* type, Vector<Parameter> args = {}, u64 flags = 0, const VTable* vtable = nullptr);

		inline const Type* type() const { return m_type; }
		inline const Vector<Parameter>& args() const { return m_args; }
		inline const VTable* vtable() const { return m_vtable; }
		inline bool can_invoke() const { return m_vtable != nullptr; }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_static() const { return has_flags(IsStatic); }
		inline bool is_virtual() const { return has_flags(IsVirtual); }
		inline bool is_pure_virtual() const { return has_flags(IsPureVirtual); }
		inline bool is_noexcept() const { return has_flags(IsNoexcept); }
		inline bool is_constructor() const { return has_flags(IsConstructor); }
		inline bool is_destructor() const { return has_flags(IsDestructor); }

		InvokeResult invoke(void* context, const Argument* args = nullptr, usize argument_count = 0,
		                    void* return_value = nullptr) const;
	};
}// namespace Trinex::Refl::New
