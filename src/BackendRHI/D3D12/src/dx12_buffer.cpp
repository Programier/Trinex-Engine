#include <Core/memory.hpp>
#include <Graphics/render_pools.hpp>
#include <dx12_api.hpp>
#include <dx12_buffer.hpp>
#include <dx12_enums.hpp>

namespace Engine
{
	D3D12Buffer::D3D12Buffer(size_t size, const byte* data, BufferCreateFlags flags) : m_size(size)
	{
		D3D12_HEAP_PROPERTIES heap_props = {};

		if (flags & BufferCreateFlags::CPURead)
			heap_props.Type = D3D12_HEAP_TYPE_READBACK;
		else if (flags & BufferCreateFlags::CPUWrite)
			heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
		else
			heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

		heap_props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_props.CreationNodeMask     = 1;
		heap_props.VisibleNodeMask      = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment           = 0;
		desc.Width               = size;
		desc.Height              = 1;
		desc.DepthOrArraySize    = 1;
		desc.MipLevels           = 1;
		desc.Format              = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count    = 1;
		desc.SampleDesc.Quality  = 0;
		desc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags               = D3D12_RESOURCE_FLAG_NONE;

		if (flags & BufferCreateFlags::UnorderedAccess)
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (data && heap_props.Type == D3D12_HEAP_TYPE_DEFAULT)
		{
			m_state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else
		{
			if (heap_props.Type == D3D12_HEAP_TYPE_UPLOAD)
				m_state = D3D12_RESOURCE_STATE_GENERIC_READ;
			else if (flags & BufferCreateFlags::UnorderedAccess)
				m_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			else if (flags & BufferCreateFlags::ShaderResource)
				m_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			else if (flags & BufferCreateFlags::TransferDst)
				m_state = D3D12_RESOURCE_STATE_COPY_DEST;
			else
				m_state = D3D12_RESOURCE_STATE_COMMON;
		}

		D3D12::api()->device()->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &desc, m_state, nullptr,
		                                                IID_PPV_ARGS(&m_buffer));

		if (data && heap_props.Type != D3D12_HEAP_TYPE_READBACK)
		{
			if (heap_props.Type == D3D12_HEAP_TYPE_UPLOAD)
			{
				copy(size, data);
			}
			else
			{
				auto api      = D3D12::api();
				auto upload   = api->upload_buffer_manager()->allocate(size, data);
				auto cmd_list = D3D12::api()->cmd_list();
				cmd_list->CopyBufferRegion(resource(), 0, upload->resource(), 0, size);
			}
		}
	}

	byte* D3D12Buffer::map()
	{
		return nullptr;
	}

	void D3D12Buffer::unmap() {}

	D3D12Buffer& D3D12Buffer::copy(size_t size, const byte* data, size_t offset)
	{
		void* mapped      = nullptr;
		D3D12_RANGE range = {offset, offset + size};
		m_buffer->Map(0, &range, &mapped);
		memcpy(mapped, data, size);
		m_buffer->Unmap(0, &range);
		return *this;
	}

	D3D12Buffer& D3D12Buffer::transition(D3D12_RESOURCE_STATES state)
	{
		if (m_state != state)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource   = m_buffer.Get();
			barrier.Transition.StateBefore = m_state;
			barrier.Transition.StateAfter  = state;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			D3D12::api()->cmd_list()->ResourceBarrier(1, &barrier);
			m_state = state;
		}
		return *this;
	}

	RHI_ShaderResourceView* D3D12Buffer::as_srv()
	{
		return nullptr;
	}

	RHI_UnorderedAccessView* D3D12Buffer::as_uav()
	{
		return nullptr;
	}

	D3D12UploadBuffer::D3D12UploadBuffer(D3D12UploadBufferManager* manager, size_t size, const byte* data)
	    : D3D12Buffer(size, data, BufferCreateFlags::CPUWrite), m_manager(manager)
	{
		manager->m_allocated.insert(this);
	}

	D3D12UploadBuffer::~D3D12UploadBuffer()
	{
		m_manager->m_allocated.erase(this);
	}

	void D3D12UploadBuffer::destroy()
	{
		m_manager->return_buffer(this);
	}

	D3D12UploadBufferManager::~D3D12UploadBufferManager()
	{
		while (!m_allocated.empty())
		{
			D3D12UploadBuffer* buffer = *m_allocated.begin();
			release(buffer);
		}
	}

	D3D12UploadBufferManager& D3D12UploadBufferManager::return_buffer(D3D12UploadBuffer* buffer)
	{
		Entry entry;
		entry.pending_frames = 60;
		entry.buffer         = buffer;
		m_free.emplace_back(entry);
		return *this;
	}

	D3D12UploadBuffer* D3D12UploadBufferManager::allocate(size_t size, const byte* data)
	{
		size_t free_count = m_free.size();

		auto api = D3D12::api();

		for (size_t i = 0; i < free_count; ++i)
		{
			Entry& entry = m_free[i];

			if (entry.buffer->size() >= size)
			{
				auto buffer = entry.buffer;
				m_free.erase(m_free.begin() + i);
				api->deferred_destroy(buffer);

				buffer->copy(size, data);
				return buffer;
			}
		}

		auto buffer = Engine::allocate<D3D12UploadBuffer>(this, size, data);
		api->deferred_destroy(buffer);
		return buffer;
	}

	RHI_Buffer* D3D12::create_buffer(size_t size, const byte* data, BufferCreateFlags flags)
	{
		return allocate<D3D12Buffer>(size, data, flags);
	}

	D3D12& D3D12::update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data)
	{
		D3D12Buffer* dst = static_cast<D3D12Buffer*>(buffer);

		auto api      = D3D12::api();
		auto upload   = api->upload_buffer_manager()->allocate(size, data);
		auto cmd_list = D3D12::api()->cmd_list();
		cmd_list->CopyBufferRegion(dst->resource(), offset, upload->resource(), 0, size);
		return *this;
	}

	D3D12& D3D12::barrier(RHI_Buffer* buffer, RHIAccess access)
	{
		static_cast<D3D12Buffer*>(buffer)->transition(resource_state_of(access));
		return *this;
	}
}// namespace Engine
