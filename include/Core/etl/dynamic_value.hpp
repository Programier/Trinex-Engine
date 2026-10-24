#pragma once
#include <Core/etl/storage.hpp>
#include <Core/reflection/new/of.hpp>

namespace Trinex
{
	class trinex_api DynamicValue
	{
	private:
		Storage<32, 16> m_storage;
		Refl::New::Type* m_type = nullptr;

	public:
		DynamicValue() = default;
		explicit DynamicValue(Refl::New::Type* type);

		template<typename T>
		DynamicValue(const T& value)
		{
			assign(Refl::New::type_of<T>(), &value);
		}

		DynamicValue(const DynamicValue& other);
		DynamicValue(DynamicValue&& other);
		~DynamicValue();

		DynamicValue& operator=(const DynamicValue& other);
		DynamicValue& operator=(DynamicValue&& other);

		template<typename T>
		DynamicValue& operator=(const T& value)
		{
			assign(Refl::New::type_of<T>(), &value);
			return *this;
		}

		bool construct(Refl::New::Type* type);
		bool assign(Refl::New::Type* type, const void* value);
		void reset();

		template<typename T>
		bool is() const
		{
			return m_type == Refl::New::type_of<T>();
		}

		template<typename T>
		T* cast()
		{
			return is<T>() ? static_cast<T*>(data()) : nullptr;
		}

		template<typename T>
		const T* cast() const
		{
			return is<T>() ? static_cast<const T*>(data()) : nullptr;
		}

		inline void* data() { return m_storage.data; }
		inline const void* data() const { return m_storage.data; }
		inline Refl::New::Type* type() const { return m_type; }
		inline explicit operator bool() const { return m_type != nullptr; }
	};

}// namespace Trinex
