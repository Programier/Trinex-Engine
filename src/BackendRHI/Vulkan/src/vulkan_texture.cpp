#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
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
	static FORCE_INLINE Value static_find_view(VulkanTexture::View<Value>* head, const VulkanTexture::ViewDesc& desc)
	{
		while (head)
		{
			if (head->desc == desc)
			{
				return head->value;
			}

			head = head->next;
		}

		return nullptr;
	}

	template<typename Value>
	static FORCE_INLINE void static_destroy_view(VulkanTexture::View<Value>* head)
	{
		while (head)
		{
			VulkanTexture::View<Value>* next = head->next;
			release(head->value);
			release(head);
			head = next;
		}
	}

	template<typename T>
	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from_generic(const T* view, const VulkanTexture* texture)
	{
		ViewDesc desc;
		desc.view_type = view->view_type == RHITextureType::Undefined ? texture->image_view_type()
		                                                              : VulkanEnums::image_view_type_of(view->view_type);

		desc.format = view->format == RHIColorFormat::Undefined ? texture->format() : VulkanEnums::format_of(view->format);

		desc.first_mip         = 0;
		desc.first_array_slice = 0;
		desc.mip_levels        = texture->mipmap_count();
		desc.array_size        = texture->layer_count();
		return desc;
	}


	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescSRV* view, const VulkanTexture* texture)
	{
		ViewDesc desc = from_generic(view, texture);
		return desc;
	}

	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescUAV* view, const VulkanTexture* texture)
	{
		ViewDesc desc = from_generic(view, texture);
		return desc;
	}

	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescRTV* view, const VulkanTexture* texture)
	{
		ViewDesc desc = from_generic(view, texture);
		return desc;
	}

	VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescDSV* view, const VulkanTexture* texture)
	{
		ViewDesc desc = from_generic(view, texture);
		return desc;
	}

	VulkanTexture& VulkanTexture::create(RHIColorFormat color_format, Vector3u size, uint32_t mips, RHITextureCreateFlags flags)
	{
		m_aspect     = VulkanEnums::aspect_of(color_format);
		m_format     = VulkanEnums::format_of(color_format);
		m_extent     = vk::Extent3D{size.x, size.y, size.z};
		m_mips_count = mips;

		switch (image_view_type())
		{
			case vk::ImageViewType::e1DArray:
			case vk::ImageViewType::e2DArray:
			case vk::ImageViewType::eCube:
			case vk::ImageViewType::eCubeArray: m_layers_count = size.z; break;
			default: m_layers_count = 1; break;
		}

		m_layout = vk::ImageLayout::eUndefined;
		m_flags  = flags;

		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

		if ((flags & RHITextureCreateFlags::ShaderResource) == RHITextureCreateFlags::ShaderResource)
			usage |= vk::ImageUsageFlagBits::eSampled;

		if ((flags & RHITextureCreateFlags::UnorderedAccess) == RHITextureCreateFlags::UnorderedAccess)
			usage |= vk::ImageUsageFlagBits::eStorage;

		if ((flags & RHITextureCreateFlags::RenderTarget) == RHITextureCreateFlags::RenderTarget)
			usage |= vk::ImageUsageFlagBits::eColorAttachment;

		if ((flags & RHITextureCreateFlags::DepthStencilTarget) == RHITextureCreateFlags::DepthStencilTarget)
			usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

		vk::ImageCreateInfo info({}, image_type(), format(), extent(), mipmap_count(), layer_count(), vk::SampleCountFlagBits::e1,
		                         vk::ImageTiling::eOptimal, usage);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage out_image = VK_NULL_HANDLE;
		auto res          = vmaCreateImage(API->m_allocator, &static_cast<VkImageCreateInfo&>(info), &alloc_info, &out_image,
		                                   &m_allocation, nullptr);
		trinex_check(res == VK_SUCCESS, "Failed to create texture!");
		m_image = out_image;

		return create_views();
	}

	VulkanTexture& VulkanTexture::create_views()
	{
		if (m_flags & RHITextureCreateFlags::ShaderResource)
		{
			vk::ImageSubresourceRange range(VulkanEnums::srv_aspect(aspect()), 0, mipmap_count(), 0, layer_count());
			vk::ImageViewCreateInfo view_info({}, image(), image_view_type(), format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			m_srv.value        = new VulkanTextureSRV(this, view);
		}

		if (m_flags & RHITextureCreateFlags::UnorderedAccess)
		{
			vk::ImageSubresourceRange range(VulkanEnums::uav_aspect(aspect()), 0, mipmap_count(), 0, layer_count());
			vk::ImageViewCreateInfo view_info({}, image(), image_view_type(), format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			m_uav.value        = new VulkanTextureUAV(this, view);
		}

		if (m_flags & RHITextureCreateFlags::RenderTarget)
		{
			vk::ImageSubresourceRange range(VulkanEnums::rtv_aspect(aspect()), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			m_rtv.value        = new VulkanTextureRTV(this, view);
		}

		if (m_flags & RHITextureCreateFlags::DepthStencilTarget)
		{
			vk::ImageSubresourceRange range(VulkanEnums::dsv_aspect(aspect()), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			m_dsv.value        = new VulkanTextureDSV(this, view);
		}

		return *this;
	}

	VulkanTexture& VulkanTexture::update(const RHITextureUpdateDesc& desc)
	{
		if (desc.data == nullptr || desc.size == 0)
			return *this;

		auto buffer = API->m_stagging_manager->allocate(desc.size, RHIBufferCreateFlags::TransferSrc);
		buffer->copy(0, static_cast<const byte*>(desc.data), desc.size);

		auto command_buffer = API->current_command_buffer();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy region(0, desc.buffer_width, desc.buffer_height,
		                           vk::ImageSubresourceLayers(aspect(), desc.mip_level, desc.array_slice, 1),
		                           vk::Offset3D(desc.offset.x, desc.offset.y, desc.offset.z),
		                           vk::Extent3D(desc.extent.x, desc.extent.y, desc.extent.z));

		command_buffer->copyBufferToImage(buffer->buffer(), image(), vk::ImageLayout::eTransferDstOptimal, region);
		return *this;
	}

	void VulkanTexture::change_layout(vk::ImageLayout new_layout)
	{
		if (layout() != new_layout || new_layout == vk::ImageLayout::eTransferDstOptimal)
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;

			barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), 0, mipmap_count(), 0, layer_count());
			m_layout                 = barrier.newLayout;

			Barrier::transition_image_layout(barrier);

			m_layout = new_layout;
		}
	}

	RHI_ShaderResourceView* VulkanTexture::as_srv(const RHITextureDescSRV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::ShaderResource))
			return nullptr;

		if (desc == nullptr)
			return m_srv.value;

		ViewDesc view_desc = ViewDesc::from(desc, this);

		if (auto view = static_find_view(m_srv.next, view_desc))
			return view;

		vk::ImageSubresourceRange range(VulkanEnums::srv_aspect(aspect()), view_desc.first_mip, view_desc.mip_levels,
		                                view_desc.first_array_slice, view_desc.array_size);
		vk::ImageViewCreateInfo view_info({}, image(), view_desc.view_type, view_desc.format, {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureSRV(this, view);
	}

	RHI_UnorderedAccessView* VulkanTexture::as_uav(const RHITextureDescUAV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::UnorderedAccess))
			return nullptr;

		if (desc == nullptr)
			return m_uav.value;

		ViewDesc view_desc = ViewDesc::from(desc, this);

		if (auto view = static_find_view(m_uav.next, view_desc))
			return view;

		vk::ImageSubresourceRange range(VulkanEnums::uav_aspect(aspect()), view_desc.first_mip, view_desc.mip_levels,
		                                view_desc.first_array_slice, view_desc.array_size);
		vk::ImageViewCreateInfo view_info({}, image(), view_desc.view_type, view_desc.format, {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureUAV(this, view);
	}

	RHI_RenderTargetView* VulkanTexture::as_rtv(const RHITextureDescRTV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::RenderTarget))
			return nullptr;

		if (desc == nullptr)
			return m_rtv.value;

		ViewDesc view_desc = ViewDesc::from(desc, this);

		if (auto view = static_find_view(m_rtv.next, view_desc))
			return view;

		vk::ImageSubresourceRange range(VulkanEnums::rtv_aspect(aspect()), view_desc.first_mip, view_desc.mip_levels,
		                                view_desc.first_array_slice, view_desc.array_size);
		vk::ImageViewCreateInfo view_info({}, image(), view_desc.view_type, view_desc.format, {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureRTV(this, view);
	}

	RHI_DepthStencilView* VulkanTexture::as_dsv(const RHITextureDescDSV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::DepthStencilTarget))
			return nullptr;

		if (desc == nullptr)
			return m_dsv.value;

		ViewDesc view_desc = ViewDesc::from(desc, this);

		if (auto view = static_find_view(m_dsv.next, view_desc))
			return view;

		vk::ImageSubresourceRange range(VulkanEnums::dsv_aspect(aspect()), view_desc.first_mip, view_desc.mip_levels,
		                                view_desc.first_array_slice, view_desc.array_size);
		vk::ImageViewCreateInfo view_info({}, image(), view_desc.view_type, view_desc.format, {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureDSV(this, view);
	}

	byte* VulkanTexture::map(RHIMappingAccess access, const RHIMappingRange* range)
	{
		byte* memory    = nullptr;
		VkResult result = vmaMapMemory(API->m_allocator, m_allocation, reinterpret_cast<void**>(&memory));

		if (result != VK_SUCCESS || memory == nullptr)
			return nullptr;

		if (range)
		{
			if (access != RHIMappingAccess::Write)
			{
				VmaAllocationInfo info;
				vmaGetAllocationInfo(API->m_allocator, m_allocation, &info);

				vk::MappedMemoryRange invalidate_range(info.deviceMemory, info.offset + range->offset, range->size);
				API->m_device.invalidateMappedMemoryRanges(invalidate_range);
			}

			return memory + range->offset;
		}

		return memory;
	}

	void VulkanTexture::unmap(const RHIMappingRange* range)
	{
		if (range)
		{
			VmaAllocationInfo info;
			vmaGetAllocationInfo(API->m_allocator, m_allocation, &info);
			vk::MappedMemoryRange flust_range(info.deviceMemory, info.offset + range->offset, range->size);
			API->m_device.flushMappedMemoryRanges(flust_range);
		}

		vmaUnmapMemory(API->m_allocator, m_allocation);
	}

	VulkanTexture::~VulkanTexture()
	{
		if (m_srv.value)
			Engine::release(m_srv.value);

		if (m_uav.value)
			Engine::release(m_uav.value);

		if (m_rtv.value)
			Engine::release(m_rtv.value);

		if (m_dsv.value)
			Engine::release(m_dsv.value);

		static_destroy_view(m_srv.next);
		static_destroy_view(m_uav.next);
		static_destroy_view(m_rtv.next);
		static_destroy_view(m_dsv.next);

		vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	RHI_Texture* VulkanAPI::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
	                                       RHITextureCreateFlags flags)
	{
		VulkanTexture* texture = nullptr;

		switch (type)
		{
			case RHITextureType::Texture1D:
				size    = {size.x, 1, 1};
				texture = new VulkanTypedTexture<vk::ImageViewType::e1D>();
				break;

			case RHITextureType::Texture1DArray:
				size    = {size.x, 1, glm::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e1DArray>();
				break;

			case RHITextureType::Texture2D:
				size    = {size.x, size.y, 1};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2D>();
				break;

			case RHITextureType::Texture2DArray:
				size    = {size.x, size.y, glm::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2DArray>();
				break;

			case RHITextureType::TextureCube:
				size    = {size.x, size.y, 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCube>();
				break;

			case RHITextureType::TextureCubeArray:
				size    = {size.x, size.y, glm::max((size.z + 5) / 6, 1u) * 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCubeArray>();
				break;

			case RHITextureType::Texture3D: texture = new VulkanTypedTexture<vk::ImageViewType::e3D>(); break;
			default: break;
		}

		if (texture)
		{
			texture->create(format, size, glm::max(mips, 1u), flags);
		}
		return texture;
	}


	VulkanAPI& VulkanAPI::update_texture(RHI_Texture* texture, const RHITextureUpdateDesc& desc)
	{
		static_cast<VulkanTexture*>(texture)->update(desc);
		return *this;
	}

	VulkanAPI& VulkanAPI::barrier(RHI_Texture* texture, RHIAccess dst_access)
	{
		static_cast<VulkanTexture*>(texture)->change_layout(VulkanEnums::image_layout_of(dst_access));
		return *this;
	}
}// namespace Engine
