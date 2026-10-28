#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/function.hpp>
#include <Core/reflection/type.hpp>

namespace Trinex::Refl
{
	class OffsetProperty : public Property
	{
	private:
		usize m_offset;

	public:
		OffsetProperty(Name name, usize offset, Type* type) : Property(name, type), m_offset(offset) {}

		void* address(void* context) override
		{
			return context == nullptr ? nullptr : static_cast<void*>(static_cast<u8*>(context) + m_offset);
		}

		const void* load(const void* context, void* temporary = nullptr) override
		{
			(void) temporary;
			return context == nullptr ? nullptr : static_cast<const void*>(static_cast<const u8*>(context) + m_offset);
		}

		void store(void* context, void* object) override
		{
			if (void* destination = address(context))
				this->type()->copy_assign(destination, object);
		}
	};

	static u32 append_type(const Type* type, u32& size, u32& alignment)
	{
		alignment = Math::max(alignment, type->alignment());

		const u32 offset = align_up(size, type->alignment());
		size             = offset + type->size();

		return offset;
	}

	Function::Function(Name name, Invoker invoker, Ref<Property> result, Array<Ref<Property>> args, u64 flags, Object* owner)
	    : Object(name, flags), m_invoker(invoker), m_result(etl::move(result)), m_args(etl::move(args)), m_size(0), m_alignment(0)
	{
		trinex_verify(m_invoker);

		for (const auto& arg : m_args)
		{
			trinex_verify(arg);
			append_type(arg->type(), m_size, m_alignment);
		}

		if (m_result)
		{
			append_type(m_result->type(), m_size, m_alignment);
		}

		if (owner)
		{
			Object::owner(owner);
		}
	}

	Function::Function(Name name, Invoker invoker, Type* result, Type** args, usize count, u64 flags, Object* owner)
	    : Object(name, flags), m_invoker(invoker), m_size(0), m_alignment(0)
	{
		trinex_verify(m_invoker);

		m_args.reserve(count);

		for (usize i = 0; i < count; ++i)
		{
			Type* arg = args[i];
			trinex_verify(arg);

			u32 offset = append_type(arg, m_size, m_alignment);
			m_args.emplace_back(Ref<OffsetProperty>::make("", offset, arg));
		}

		if (result && !result->is_void())
		{
			u32 offset = append_type(result, m_size, m_alignment);
			m_result   = Ref<OffsetProperty>::make("result", offset, result);
		}

		if (owner)
		{
			Object::owner(owner);
		}
	}
} // namespace Trinex::Refl
