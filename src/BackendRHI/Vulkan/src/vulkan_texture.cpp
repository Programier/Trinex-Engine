#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	template<typename Value>
	static inline Value* static_as_view(VulkanTexture* texture, Vector<VulkanTexture::View<Value>>& views,
	                                    const VulkanTexture::ViewDesc& desc, vk::ImageAspectFlags aspect)
	{
		for (auto& view : views)
		{
			if (view.desc == desc)
				return view.value;
		}

		aspect &= texture->aspect();

		if (aspect == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageSubresourceRange range(aspect, desc.first_mip, desc.mip_levels, desc.first_array_slice, desc.array_size);
		vk::ImageViewCreateInfo view_info({}, texture->image(), VulkanEnums::image_view_type_of(desc.view_type),
		                                  texture->format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);

		auto& node = views.emplace_back();
		node.desc  = desc;

		node.value = trx_new Value(texture, view, desc);
		return node.value;
	}

	template<typename Value>
	static FORCE_INLINE void static_destroy_view(Vector<VulkanTexture::View<Value>>& views)
	{
		for (auto& view : views)
		{
			trx_delete view.value;
		}
	}

	template<typename T>
	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from_base(const T* view, const VulkanTexture* texture)
	{
		ViewDesc desc;

		desc.first_array_slice = Math::min<uint16_t>(view->base_slice, texture->layer_count() - 1);
		desc.array_size        = Math::min<uint16_t>(view->slice_count, texture->layer_count() - desc.first_array_slice);

		desc.view_type = view->view_type == RHITextureType::Undefined ? texture->texture_type() : view->view_type;
		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescSRV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescSRV>();

		ViewDesc desc   = from_base(view, texture);
		desc.first_mip  = Math::min<uint8_t>(view->base_mip, texture->mipmap_count() - 1);
		desc.mip_levels = Math::min<uint8_t>(view->mip_count, texture->mipmap_count() - desc.first_mip);
		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescUAV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescUAV>();

		ViewDesc desc   = from_base(view, texture);
		desc.first_mip  = Math::min<uint8_t>(view->base_mip, texture->mipmap_count() - 1);
		desc.mip_levels = 1;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescRTV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescRTV>();

		ViewDesc desc   = from_base(view, texture);
		desc.first_mip  = Math::min<uint8_t>(view->base_mip, texture->mipmap_count() - 1);
		desc.mip_levels = 1;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescDSV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescDSV>();

		ViewDesc desc = from_base(view, texture);

		desc.first_mip  = Math::min<uint8_t>(view->base_mip, texture->mipmap_count() - 1);
		desc.mip_levels = 1;


		return desc;
	}

	VulkanTexture& VulkanTexture::create(RHIColorFormat format, Vector3u size, uint_t layers, uint32_t mips,
	                                     RHITextureCreateFlags flags)
	{
		return create(VulkanEnums::format_of(format), size, layers, mips, flags);
	}

	VulkanTexture& VulkanTexture::create(vk::Format format, Vector3u size, uint_t layers, uint32_t mips,
	                                     RHITextureCreateFlags flags)
	{
		m_format       = format;
		m_extent       = vk::Extent3D{size.x, size.y, size.z};
		m_mips_count   = mips;
		m_layers_count = layers;
		m_access       = RHIAccess::Undefined;
		m_flags        = flags;

		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

		if ((flags & RHITextureCreateFlags::ShaderResource) == RHITextureCreateFlags::ShaderResource)
			usage |= vk::ImageUsageFlagBits::eSampled;

		if ((flags & RHITextureCreateFlags::UnorderedAccess) == RHITextureCreateFlags::UnorderedAccess)
			usage |= vk::ImageUsageFlagBits::eStorage;

		if ((flags & RHITextureCreateFlags::RenderTarget) == RHITextureCreateFlags::RenderTarget)
			usage |= vk::ImageUsageFlagBits::eColorAttachment;

		if ((flags & RHITextureCreateFlags::DepthStencilTarget) == RHITextureCreateFlags::DepthStencilTarget)
			usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

		vk::ImageCreateFlags image_flags = {};

		if (is_cube_compatible())
		{
			image_flags |= vk::ImageCreateFlagBits::eCubeCompatible;
		}

		vk::ImageCreateInfo info(image_flags, image_type(), format, extent(), mipmap_count(), layer_count(),
		                         vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage out_image = VK_NULL_HANDLE;
		auto res          = vmaCreateImage(API->m_allocator, &static_cast<VkImageCreateInfo&>(info), &alloc_info, &out_image,
		                                   &m_allocation, nullptr);
		trinex_check(res == VK_SUCCESS, "Failed to create texture!");
		m_image = out_image;

		return *this;
	}

	VulkanTexture& VulkanTexture::barrier(VulkanContext* ctx, RHIAccess access)
	{
		if ((m_access & access) == access && !(access & RHIAccess::WritableMask))
			return *this;

		vk::ImageMemoryBarrier barrier;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout           = VulkanEnums::image_layout_of(m_access);
		barrier.srcAccessMask       = VulkanEnums::access_of(m_access);
		barrier.newLayout           = VulkanEnums::image_layout_of(access);
		barrier.dstAccessMask       = VulkanEnums::access_of(access);
		barrier.image               = m_image;

		barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), 0, mipmap_count(), 0, layer_count());

		auto src_stage = VulkanEnums::pipeline_stage_of(m_access);
		auto dst_stage = VulkanEnums::pipeline_stage_of(access);
		ctx->end_render_pass()->pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);

		m_access = access;
		return *this;
	}

	RHIShaderResourceView* VulkanTexture::as_srv(RHITextureDescSRV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::ShaderResource))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor | vk::ImageAspectFlagBits::eDepth;
		return static_as_view(this, m_srv, ViewDesc::from(desc, this), mask);
	}

	RHIUnorderedAccessView* VulkanTexture::as_uav(RHITextureDescUAV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::UnorderedAccess))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_uav, ViewDesc::from(desc, this), mask);
	}

	RHIRenderTargetView* VulkanTexture::as_rtv(RHITextureDescRTV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::RenderTarget))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_rtv, ViewDesc::from(desc, this), mask);
	}

	RHIDepthStencilView* VulkanTexture::as_dsv(RHITextureDescDSV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::DepthStencilTarget))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		return static_as_view(this, m_dsv, ViewDesc::from(desc, this), mask);
	}

	VulkanTexture::~VulkanTexture()
	{
		static_destroy_view(m_srv);
		static_destroy_view(m_uav);
		static_destroy_view(m_rtv);
		static_destroy_view(m_dsv);

		if (m_allocation)
			vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	RHITexture* VulkanAPI::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
	                                      RHITextureCreateFlags flags)
	{
		VulkanTexture* texture = nullptr;

		uint_t layers = 1;

		switch (type)
		{
			case RHITextureType::Texture1D:
				size    = {size.x, 1, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::e1D, RHITextureType::Texture1D>();
				break;

			case RHITextureType::Texture1DArray:
				layers  = Math::max(size.z, 1u);
				size    = {size.x, 1, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::e1DArray, RHITextureType::Texture1DArray>();
				break;

			case RHITextureType::Texture2D:
				size    = {size.x, size.y, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::e2D, RHITextureType::Texture2D>();
				break;

			case RHITextureType::Texture2DArray:
				layers  = Math::max(size.z, 1u);
				size    = {size.x, size.y, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::e2DArray, RHITextureType::Texture2DArray>();
				break;

			case RHITextureType::TextureCube:
				layers  = 6;
				size    = {size.x, size.y, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::eCube, RHITextureType::TextureCube>();
				break;

			case RHITextureType::TextureCubeArray:
				layers  = Math::max((size.z + 5) / 6, 1u) * 6;
				size    = {size.x, size.y, 1};
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::eCubeArray, RHITextureType::TextureCubeArray>();
				break;

			case RHITextureType::Texture3D:
				texture = trx_new VulkanTypedTexture<vk::ImageViewType::e3D, RHITextureType::Texture3D>();
				break;
			default: break;
		}

		if (texture)
		{
			texture->create(format, size, layers, Math::max(mips, 1u), flags);
		}
		return texture;
	}

	VulkanContext& VulkanContext::update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data,
	                                             size_t size, size_t buffer_width, size_t buffer_height)
	{
		auto buffer = API->m_stagging_manager->allocate(size, RHIBufferCreateFlags::TransferSrc);
		buffer->copy(this, 0, static_cast<const byte*>(data), size);

		VulkanTexture* vulkan_texture = static_cast<VulkanTexture*>(texture);

		vk::BufferImageCopy copy_region(0, buffer_width, buffer_height,
		                                vk::ImageSubresourceLayers(vulkan_texture->aspect(), region.mip, region.slice, 1),
		                                vk::Offset3D(region.offset.x, region.offset.y, region.offset.z),
		                                vk::Extent3D(region.extent.x, region.extent.y, region.extent.z));

		m_cmd->copyBufferToImage(buffer->buffer(), vulkan_texture->image(), vk::ImageLayout::eTransferDstOptimal, copy_region);
		m_cmd->add_stagging(buffer);
		return *this;
	}

	VulkanContext& VulkanContext::copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice,
	                                                     const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
	                                                     size_t buffer_offset)
	{
		VulkanTexture* src = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* dst  = static_cast<VulkanBuffer*>(buffer);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(src->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		m_cmd->copyImageToBuffer(src->image(), vk::ImageLayout::eTransferSrcOptimal, dst->buffer(), region);
		return *this;
	}

	VulkanContext& VulkanContext::copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture,
	                                                     uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
	                                                     const Vector3u& extent)
	{
		VulkanTexture* dst = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* src  = static_cast<VulkanBuffer*>(buffer);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(dst->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		m_cmd->copyBufferToImage(src->buffer(), dst->image(), vk::ImageLayout::eTransferDstOptimal, region);
		return *this;
	}

	VulkanContext& VulkanContext::copy_texture_to_texture(RHITexture* src_texture, const RHITextureRegion& src_region,
	                                                      RHITexture* dst_texture, const RHITextureRegion& dst_region)
	{
		end_render_pass();
		VulkanTexture* src = static_cast<VulkanTexture*>(src_texture);
		VulkanTexture* dst = static_cast<VulkanTexture*>(dst_texture);

		vk::ImageSubresourceLayers src_subresource(src->aspect(), src_region.mip, src_region.slice, 1);
		vk::ImageSubresourceLayers dst_subresource(dst->aspect(), dst_region.mip, dst_region.slice, 1);

		if (src_region.extent == dst_region.extent && src->format() == dst->format())
		{
			vk::ImageCopy region(src_subresource, vk::Offset3D(src_region.offset.x, src_region.offset.y, src_region.offset.z),
			                     dst_subresource, vk::Offset3D(dst_region.offset.x, dst_region.offset.y, dst_region.offset.z),
			                     vk::Extent3D(src_region.extent.x, src_region.extent.y, src_region.extent.z));

			m_cmd->copyImage(src->image(), vk::ImageLayout::eTransferSrcOptimal, dst->image(),
			                 vk::ImageLayout::eTransferDstOptimal, region);
		}
		else
		{
			Vector3u src_region_end = src_region.offset + src_region.extent;
			Vector3u dst_region_end = dst_region.offset + dst_region.extent;

			std::array<vk::Offset3D, 2> src_offsets = {
			        vk::Offset3D(src_region.offset.x, src_region.offset.y, src_region.offset.z),
			        vk::Offset3D(src_region_end.x, src_region_end.y, src_region_end.z),
			};

			std::array<vk::Offset3D, 2> dst_offsets = {
			        vk::Offset3D(dst_region.offset.x, dst_region.offset.y, dst_region.offset.z),
			        vk::Offset3D(dst_region_end.x, dst_region_end.y, dst_region_end.z),
			};

			vk::ImageBlit blit(src_subresource, src_offsets, dst_subresource, dst_offsets);

			m_cmd->blitImage(src->image(), vk::ImageLayout::eTransferSrcOptimal, dst->image(),
			                 vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
		}

		return *this;
	}


	VulkanContext& VulkanContext::barrier(RHITexture* texture, RHIAccess access)
	{
		static_cast<VulkanTexture*>(texture)->barrier(this, access);
		return *this;
	}
}// namespace Engine
