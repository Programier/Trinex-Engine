#pragma once
#include <Core/ref_counted.hpp>

namespace Trinex
{
	class ENGINE_EXPORT Blob : public RefCounted
	{
	public:
		virtual ~Blob() = default;

		virtual usize size() const     = 0;
		virtual u8* data()             = 0;
		virtual const u8* data() const = 0;

		inline bool empty() const { return size() == 0; }
		inline u8* begin() { return data(); }
		inline u8* end() { return data() + size(); }
		inline const u8* begin() const { return data(); }
		inline const u8* end() const { return data() + size(); }

		template<typename T>
		T as()
		{
			using Value = typename T::value_type;

			trinex_assert(size() % sizeof(Value) == 0);
			trinex_assert(reinterpret_cast<u64>(data()) % alignof(Value) == 0);
			return T{reinterpret_cast<Value*>(data()), size() / sizeof(Value)};
		}

		template<typename T>
		T as() const
		{
			using Value = typename T::value_type;

			trinex_assert(size() % sizeof(Value) == 0);
			trinex_assert(reinterpret_cast<u64>(data()) % alignof(Value) == 0);
			return T{reinterpret_cast<const Value*>(data()), size() / sizeof(Value)};
		}
	};
}// namespace Trinex
