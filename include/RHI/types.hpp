#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/storage.hpp>

namespace Trinex
{
	template<typename Tag>
	struct RHIResourceHandle : Storage<8, 8>
	{
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

	template<typename T>
	class RHIArrayView
	{
	private:
		const T* m_data = nullptr;
		usize m_size    = 0;

	public:
		constexpr RHIArrayView() noexcept = default;
		constexpr RHIArrayView(null) noexcept {}
		constexpr RHIArrayView(const T* data, usize size) noexcept : m_data(data), m_size(size) {}
		constexpr RHIArrayView(const T& value) noexcept : m_data(&value), m_size(1) {}
		RHIArrayView(const T&&) = delete;
		template<usize N>
		constexpr RHIArrayView(const T (&array)[N]) noexcept : m_data(array), m_size(N)
		{}

		template<usize N>
		RHIArrayView(const T (&&)[N]) = delete;

		constexpr const T* data() const noexcept { return m_data; }
		constexpr u32 size() const noexcept { return m_size; }
		constexpr bool empty() const noexcept { return m_size == 0; }
		constexpr const T& operator[](usize index) const noexcept { return m_data[index]; }
		constexpr const T* begin() const noexcept { return m_data; }
		constexpr const T* end() const noexcept { return m_data ? m_data + m_size : nullptr; }
	};

} // namespace Trinex
