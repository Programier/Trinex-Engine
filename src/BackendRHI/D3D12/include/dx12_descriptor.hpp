#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <dx12_headers.hpp>

namespace Engine
{
	struct D3D12Descriptor;

	class D3D12DescritorHeap
	{
		ComPtr<ID3D12DescriptorHeap> m_heap;
		Vector<uint32_t> m_free_offsets;
		uint32_t m_count;
		uint32_t m_allocated;
		uint32_t m_descriptor_size;

	public:
		D3D12DescritorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);

		D3D12Descriptor allocate();
		D3D12DescritorHeap& release(const D3D12Descriptor& descriptor);

		inline D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle() const { return m_heap->GetCPUDescriptorHandleForHeapStart(); }
		inline D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle() const { return m_heap->GetGPUDescriptorHandleForHeapStart(); }
	};

	struct D3D12Descriptor {
	private:
		D3D12DescritorHeap* m_heap;
		uint32_t m_offset = 0;

	public:
		inline D3D12Descriptor(D3D12DescritorHeap* heap = nullptr, size_t offset = 0) : m_heap(heap), m_offset(offset) {}

		inline D3D12DescritorHeap* heap() const { return m_heap; }
		inline uint32_t offset() const { return m_offset; }
		inline bool is_valid() const { return m_heap != nullptr; }

		inline D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle() const
		{
			auto handle = m_heap->cpu_handle();
			handle.ptr += m_offset;
			return handle;
		}

		inline D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle() const
		{
			auto handle = m_heap->gpu_handle();
			handle.ptr += m_offset;
			return handle;
		}

		inline void release()
		{
			if (m_heap)
			{
				m_heap->release(*this);
				m_heap = nullptr;
			}
		}
	};

	class D3D12DescritorManager
	{
		Vector<D3D12DescritorHeap*> m_rtv_heaps;
		Vector<D3D12DescritorHeap*> m_dsv_heaps;
		Vector<D3D12DescritorHeap*> m_sampler_heaps;
		Vector<D3D12DescritorHeap*> m_resource_heaps;
		size_t m_heaps = 0;

		D3D12Descriptor allocate(Vector<D3D12DescritorHeap*>& heaps, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	public:
		~D3D12DescritorManager();

		D3D12Descriptor allocate_rtv();
		D3D12Descriptor allocate_dsv();
		D3D12Descriptor allocate_sampler();
		D3D12Descriptor allocate_resource();
		D3D12DescritorManager& bind();
	};
}// namespace Engine
