#pragma once
#include <Core/reflection/property.hpp>

namespace Engine::Refl
{
	template<typename Super, bool dereference = false>
	class ScriptProperty : public Super
	{
	private:
		uint_t m_offset;

	public:
		using Super::m_flags;

		template<typename... Args>
		ScriptProperty(uint_t offset, Args... args) : Super(args...), m_offset(offset)
		{}

		void* address(void* context) override
		{
			if constexpr (dereference)
				return *reinterpret_cast<byte**>(reinterpret_cast<byte*>(context) + m_offset);
			else
				return reinterpret_cast<byte*>(context) + m_offset;
		}

		const void* address(const void* context) const override
		{
			if constexpr (dereference)
				return *reinterpret_cast<const byte* const*>(reinterpret_cast<const byte*>(context) + m_offset);
			else
				return reinterpret_cast<const byte*>(context) + m_offset;
		}
	};

	using ScriptBooleanProperty = ScriptProperty<BooleanProperty>;

	template<typename T>
	class ScriptIntegerProperty : public ScriptProperty<IntegerProperty>
	{
	public:
		using ScriptProperty::ScriptProperty;

		size_t size() const override { return sizeof(T); }

		bool is_signed() const override { return std::is_signed_v<T>; }
	};

	template<typename T>
	class ScriptFloatProperty : public ScriptProperty<FloatProperty>
	{
	public:
		using ScriptProperty::ScriptProperty;

		size_t size() const override { return sizeof(T); }
	};

	class ScriptEnumProperty : public ScriptProperty<EnumProperty>
	{
		Enum* m_enum_instance;

	public:
		inline ScriptEnumProperty(size_t offset, Enum* enum_instance, BitMask flags = 0)
			: ScriptProperty<EnumProperty>(offset, flags), m_enum_instance(enum_instance)
		{}

		inline Enum* enum_instance() const override { return m_enum_instance; }
		size_t size() const override;
	};

	class ScriptObjectProperty : public ScriptProperty<ObjectProperty, false>
	{
		Class* m_instance;

	public:
		ScriptObjectProperty(uint_t offset, Class* instance);
		Class* class_instance() const override;
	};

	class ScriptStructProperty : public ScriptProperty<StructProperty, true>
	{
		Struct* m_instance;

	public:
		ScriptStructProperty(uint_t offset, Struct* instance);

		Struct* struct_instance() const override;
		size_t size() const override;
	};

	using ScriptStringProperty = ScriptProperty<StringProperty, true>;

	// VectorProperty;
	// NameProperty;
	// ObjectProperty;
	// ArrayProperty;
}// namespace Engine::Refl
