#include <Core/archive.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/math/math.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	RHITextureView::RHITextureView(RHITexture* texture, RHITextureType type, uint16_t base_slice, uint16_t slice_count,
	                               uint16_t base_mip, uint16_t mip_count)
	    : m_texture(texture), m_base_slice(base_slice), m_slice_count(slice_count), m_base_mip(base_mip), m_mip_count(mip_count)
	{}

	template<typename T>
	T RHITextureView::initialize_description(const T* desc)
	{
		T view = desc ? *desc : T();

		if (view.view_type == RHITextureType::Undefined)
			view.view_type = static_cast<RHITextureType::Enum>(m_type);

		view.base_slice += m_base_slice;
		view.base_mip += m_base_mip;
		view.slice_count = Math::min(m_slice_count, view.slice_count);
		return view;
	}

	void RHITextureView::destroy() {}

	RHIRenderTargetView* RHITextureView::as_rtv(RHITextureDescRTV* desc)
	{
		RHITextureDescRTV view = initialize_description(desc);
		return m_texture->as_rtv(&view);
	}

	RHIDepthStencilView* RHITextureView::as_dsv(RHITextureDescDSV* desc)
	{
		RHITextureDescDSV view = initialize_description(desc);
		return m_texture->as_dsv(&view);
	}

	RHIShaderResourceView* RHITextureView::as_srv(RHITextureDescSRV* desc)
	{
		RHITextureDescSRV view = initialize_description(desc);
		view.mip_count         = Math::min(m_mip_count, view.mip_count);
		return m_texture->as_srv(&view);
	}

	RHIUnorderedAccessView* RHITextureView::as_uav(RHITextureDescUAV* desc)
	{
		RHITextureDescUAV view = initialize_description(desc);
		return m_texture->as_uav(&view);
	}

	bool RHIBuffer::serialize(Archive& ar)
	{
		size_t buffer_size = size();

		if (ar.is_reading())
		{
			if (byte* data = map(RHIMappingAccess::Write))
			{
				const bool status = ar.read_data(data, buffer_size);
				unmap();
				return status;
			}

			StackByteAllocator::Mark mark;
			byte* data = StackByteAllocator::allocate(buffer_size);

			if (!ar.read_data(data, buffer_size))
			{
				return false;
			}

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				ctx->barrier(this, RHIAccess::TransferDst);
				ctx->update_buffer(this, 0, buffer_size, data);
			}
			RHIContextPool::global_instance()->end_context(ctx);

			return true;
		}
		else
		{
			if (byte* data = map(RHIMappingAccess::Read))
			{
				const bool status = ar.write_data(data, buffer_size);
				unmap();
				return status;
			}

			const auto flags  = RHIBufferCreateFlags::CPURead | RHIBufferCreateFlags::TransferDst;
			RHIBuffer* buffer = RHIBufferPool::global_instance()->request_buffer(buffer_size, flags);

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				ctx->barrier(this, RHIAccess::TransferSrc);
				ctx->barrier(buffer, RHIAccess::TransferDst);
				ctx->copy_buffer_to_buffer(this, buffer, buffer_size, 0, 0);
			}
			RHIContextPool::global_instance()->end_context(ctx);
			rhi->idle();

			const byte* data  = buffer->map(RHIMappingAccess::Read);
			const bool status = ar.write_data(data, buffer_size);
			buffer->unmap();

			RHIBufferPool::global_instance()->return_buffer(buffer);

			return status;
		}
	}
}// namespace Engine
