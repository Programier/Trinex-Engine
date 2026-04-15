#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/storage.hpp>

namespace Trinex
{
	template<typename Tag, typename Base>
	struct RHIResourceHandle : Storage<8, alignof(Base)> {
		static_assert(sizeof(Base) == 8 || sizeof(Base) == 4);

		constexpr RHIResourceHandle() : Storage<8, alignof(Base)>({0}) {}
		constexpr RHIResourceHandle(Base init) : Storage<8, alignof(Base)>({0}) { value<0>() = init; }
		constexpr RHIResourceHandle(const RHIResourceHandle&)            = default;
		constexpr RHIResourceHandle& operator=(const RHIResourceHandle&) = default;

		template<usize idx = 0>
		Base& value()
		{
			return Storage<8, alignof(Base)>::template as<Base, idx>();
		}

		template<usize idx = 0>
		const Base& value() const
		{
			return Storage<8, alignof(Base)>::template as<Base, idx>();
		}
	};

	enum class DescriptorTag
	{
	};

	enum class DeviceAddressTag
	{
	};

	using RHIDescriptor    = RHIResourceHandle<DescriptorTag, u32>;
	using RHIDeviceAddress = RHIResourceHandle<DeviceAddressTag, u64>;
}// namespace Trinex
