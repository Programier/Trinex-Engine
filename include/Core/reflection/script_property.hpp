#pragma once
#include <Core/reflection/property.hpp>

namespace Trinex::Refl
{
	template<typename Super, bool dereference = false>
	class ScriptProperty : public Super
	{
	private:
		u32 m_offset;

	public:
		using Super::m_flags;

		template<typename... Args>
		ScriptProperty(u32 offset, Args... args) : Super(args...), m_offset(offset)
		{}

		void* address(void* context) override
		{
			if constexpr (dereference)
				return *reinterpret_cast<u8**>(reinterpret_cast<u8*>(context) + m_offset);
			else
				return reinterpret_cast<u8*>(context) + m_offset;
		}

		const void* address(const void* context) const override
		{
			if constexpr (dereference)
				return *reinterpret_cast<const u8* const*>(reinterpret_cast<const u8*>(context) + m_offset);
			else
				return reinterpret_cast<const u8*>(context) + m_offset;
		}
	};

	using ScriptBooleanProperty = ScriptProperty<BooleanProperty>;

	template<typename T>
	class ScriptIntegerProperty : public ScriptProperty<IntegerProperty>
	{
	public:
		using ScriptProperty::ScriptProperty;

		usize size() const override { return sizeof(T); }
		usize alignment() const override { return alignof(T); }

		bool is_signed() const override { return std::is_signed_v<T>; }
	};

	template<typename T>
	class ScriptFloatProperty : public ScriptProperty<FloatProperty>
	{
	public:
		using ScriptProperty::ScriptProperty;

		usize size() const override { return sizeof(T); }
		usize alignment() const override { return alignof(T); }
	};

	class ScriptEnumProperty : public ScriptProperty<EnumProperty>
	{
		Enum* m_enum_instance;

	public:
		inline ScriptEnumProperty(usize offset, Enum* enum_instance, BitMask flags = 0)
		    : ScriptProperty<EnumProperty>(offset, flags), m_enum_instance(enum_instance)
		{}

		inline Enum* enum_instance() const override { return m_enum_instance; }
		usize size() const override;
		usize alignment() const override;
	};

	class ScriptObjectProperty : public ScriptProperty<ObjectProperty, false>
	{
		Class* m_instance;

	public:
		ScriptObjectProperty(u32 offset, Class* instance);
		Class* class_instance() const override;
	};

	class ScriptStructProperty : public ScriptProperty<StructProperty, true>
	{
		Struct* m_instance;

	public:
		ScriptStructProperty(u32 offset, Struct* instance);

		Struct* struct_instance() const override;
		usize size() const override;
		usize alignment() const override;
	};

	using ScriptStringProperty = ScriptProperty<StringProperty, true>;

	// VectorProperty;
	// NameProperty;
	// ObjectProperty;
	// ArrayProperty;
}// namespace Trinex::Refl
