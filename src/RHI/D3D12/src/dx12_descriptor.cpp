#include <Core/memory.hpp>
#include <dx12_api.hpp>
#include <dx12_descriptor.hpp>

namespace Engine
{
	static constexpr D3D12_DESCRIPTOR_HEAP_DESC s_rtv_config = {
	        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	        1024,
	        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	        0,
	};

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC s_dsv_config = {
	        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
	        64,
	        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	        0,
	};

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC s_sampler_config = {
	        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	        128,
	        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	        0,
	};

	static constexpr D3D12_DESCRIPTOR_HEAP_DESC s_resource_config = {
	        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	        512 * 1024,
	        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	        0,
	};

	D3D12DescritorHeap::D3D12DescritorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc) : m_count(desc.NumDescriptors), m_allocated(0)
	{
		auto device = D3D12::api()->device();
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
		m_descriptor_size = device->GetDescriptorHandleIncrementSize(desc.Type);
	}

	D3D12Descriptor D3D12DescritorHeap::allocate()
	{
		if (!m_free_offsets.empty())
		{
			D3D12Descriptor descriptor(this, m_free_offsets.back());
			m_free_offsets.pop_back();
			return descriptor;
		}

		if (m_allocated < m_count)
		{
			D3D12Descriptor descriptor(this, m_allocated * m_descriptor_size);
			++m_allocated;
			return descriptor;
		}

		return D3D12Descriptor(nullptr, 0);
	}

	D3D12DescritorHeap& D3D12DescritorHeap::release(const D3D12Descriptor& descriptor)
	{
		if (descriptor.heap() == this)
		{
			m_free_offsets.push_back(descriptor.offset());
		}
		return *this;
	}

	static void destroy_heaps(Vector<D3D12DescritorHeap*>& heaps)
	{
		for (D3D12DescritorHeap* heap : heaps)
		{
			release(heap);
		}
		heaps.clear();
	}

	D3D12DescritorManager::~D3D12DescritorManager()
	{
		destroy_heaps(m_rtv_heaps);
		destroy_heaps(m_dsv_heaps);
		destroy_heaps(m_sampler_heaps);
		destroy_heaps(m_resource_heaps);
	}

	D3D12Descriptor D3D12DescritorManager::allocate(Vector<D3D12DescritorHeap*>& heaps, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
	{
		for (D3D12DescritorHeap* heap : heaps)
		{
			auto descriptor = heap->allocate();
			if (descriptor.is_valid())
				return descriptor;
		}

		D3D12DescritorHeap* heap = Engine::allocate<D3D12DescritorHeap>(desc);
		heaps.push_back(heap);
		++m_heaps;
		return heap->allocate();
	}

	D3D12Descriptor D3D12DescritorManager::allocate_rtv()
	{
		return allocate(m_rtv_heaps, s_rtv_config);
	}

	D3D12Descriptor D3D12DescritorManager::allocate_dsv()
	{
		return allocate(m_dsv_heaps, s_dsv_config);
	}

	D3D12Descriptor D3D12DescritorManager::allocate_sampler()
	{
		return allocate(m_sampler_heaps, s_sampler_config);
	}

	D3D12Descriptor D3D12DescritorManager::allocate_resource()
	{
		return allocate(m_resource_heaps, s_resource_config);
	}

	D3D12DescritorManager& D3D12DescritorManager::bind()
	{
		if (m_heaps != 0)
		{
		}
		return *this;
	}
}// namespace Engine
