#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/storage.hpp>

namespace Trinex
{
	template<typename Tag>
	struct RHIResourceHandle : Storage<8, 8> {
		constexpr RHIResourceHandle() : Storage<8, 8>({0}) {}
		constexpr RHIResourceHandle(u64 value) : Storage<8, 8>({0}) { as<u64>() = value; }
		constexpr RHIResourceHandle(const RHIResourceHandle&)            = default;
		constexpr RHIResourceHandle& operator=(const RHIResourceHandle&) = default;

		using Storage<8, 8>::as;
		using Storage<8, 8>::construct;
		using Storage<8, 8>::destroy;
	};

	enum class DescriptorTag
	{
	};

	enum class DeviceAddressTag
	{
	};

	using RHIDescriptor    = RHIResourceHandle<DescriptorTag>;
	using RHIDeviceAddress = RHIResourceHandle<DeviceAddressTag>;
}// namespace Trinex
