#pragma once
#include <Core/etl/vector.hpp>
#include <dx12_headers.hpp>


namespace Engine
{
	struct RHIObject;

	class D3D12CommandAlloctor
	{
		struct Entry {
			Entry* m_next;
			Vector<RHIObject*> m_pending_destroy;
			ComPtr<ID3D12CommandAllocator> m_allocator;
			ComPtr<ID3D12Fence> m_fence;
			uint64_t m_last_signaled;

			Entry();
		};

		Entry* m_first     = nullptr;
		Entry** m_push_ptr = nullptr;
		Entry* m_current   = nullptr;

		Entry* current_entry();

	public:
		D3D12CommandAlloctor();
		~D3D12CommandAlloctor();

		D3D12CommandAlloctor& submit();

		inline ID3D12CommandAllocator* allocator() { return current_entry()->m_allocator.Get(); }
		inline D3D12CommandAlloctor& add_object_to_destroy(RHIObject* object)
		{
			current_entry()->m_pending_destroy.push_back(object);
			return *this;
		}
	};
}// namespace Engine
