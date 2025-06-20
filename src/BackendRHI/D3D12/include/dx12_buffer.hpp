#pragma once
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <dx12_destroyable.hpp>
#include <dx12_headers.hpp>

namespace Engine
{
	class D3D12Buffer : public D3D12_DeferredDestroyable<RHI_Buffer>
	{
	private:
		ComPtr<ID3D12Resource> m_buffer;
		D3D12_RESOURCE_STATES m_state;
		size_t m_size;

	public:
		D3D12Buffer(size_t size, const byte* data, BufferCreateFlags flags);

		byte* map() override;
		void unmap() override;
		D3D12Buffer& copy(size_t size, const byte* data, size_t offset = 0);
		D3D12Buffer& transition(D3D12_RESOURCE_STATES state);
		RHI_ShaderResourceView* as_srv() override;
		RHI_UnorderedAccessView* as_uav() override;


		inline D3D12_GPU_VIRTUAL_ADDRESS virtual_address() const { return m_buffer->GetGPUVirtualAddress(); }
		inline size_t size() const { return m_size; }
		inline ID3D12Resource* resource() const { return m_buffer.Get(); }
	};


	class D3D12UploadBufferManager;

	class D3D12UploadBuffer : public D3D12Buffer
	{
	private:
		D3D12UploadBufferManager* m_manager;

	public:
		D3D12UploadBuffer(D3D12UploadBufferManager* manager, size_t size, const byte* data);
		~D3D12UploadBuffer();
		void destroy() override;
	};

	class D3D12UploadBufferManager
	{
	private:
		struct Entry {
			size_t pending_frames;
			D3D12UploadBuffer* buffer;
		};

		Set<D3D12UploadBuffer*> m_allocated;
		Vector<Entry> m_free;

		D3D12UploadBufferManager& return_buffer(D3D12UploadBuffer* buffer);

	public:
		~D3D12UploadBufferManager();
		D3D12UploadBuffer* allocate(size_t size, const byte* data);
		friend D3D12UploadBuffer;
	};
}// namespace Engine
