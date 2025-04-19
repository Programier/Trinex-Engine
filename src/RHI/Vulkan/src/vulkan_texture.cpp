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
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
	vk::Image VulkanTexture::image() const
	{
		return m_image;
	}

	vk::ImageLayout VulkanTexture::layout() const
	{
		return m_layout;
	}

	VulkanTexture& VulkanTexture::create(vk::ImageCreateFlagBits flags, vk::ImageUsageFlags usage,
	                                     TextureCreateFlags create_flags)
	{
		m_layout = vk::ImageLayout::eUndefined;
		m_flags  = create_flags;

		usage |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

		if ((create_flags & TextureCreateFlags::ShaderResource) == TextureCreateFlags::ShaderResource)
			usage |= vk::ImageUsageFlagBits::eSampled;

		if ((create_flags & TextureCreateFlags::UnorderedAccess) == TextureCreateFlags::UnorderedAccess)
			usage |= vk::ImageUsageFlagBits::eStorage;

		if ((create_flags & TextureCreateFlags::RenderTarget) == TextureCreateFlags::RenderTarget)
			usage |= vk::ImageUsageFlagBits::eColorAttachment;

		if ((create_flags & TextureCreateFlags::DepthStencilTarget) == TextureCreateFlags::DepthStencilTarget)
			usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

		vk::ImageCreateInfo info(flags, image_type(), format(), extent(), mipmap_count(), layer_count(),
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

	vk::ImageAspectFlags VulkanTexture::aspect() const
	{
		ColorFormat format = engine_format();
		if (format.is_color())
		{
			return vk::ImageAspectFlagBits::eColor;
		}
		else if (format.is_depth_stencil())
		{
			return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		}
		return vk::ImageAspectFlagBits::eDepth;
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

	RHI_ShaderResourceView* VulkanTexture::create_srv()
	{
		vk::ImageSubresourceRange range(VulkanEnums::srv_aspect(aspect()), 0, mipmap_count(), 0, layer_count());

		if (range.aspectMask == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageViewCreateInfo view_info({}, image(), view_type(), format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureSRV(this, view);
	}

	RHI_UnorderedAccessView* VulkanTexture::create_uav()
	{
		vk::ImageSubresourceRange range(VulkanEnums::uav_aspect(aspect()), 0, mipmap_count(), 0, layer_count());

		if (range.aspectMask == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageViewCreateInfo view_info({}, image(), view_type(), format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureUAV(this, view);
	}

	RHI_RenderTargetView* VulkanTexture::create_rtv()
	{
		vk::ImageSubresourceRange range(VulkanEnums::rtv_aspect(aspect()), 0, 1, 0, 1);

		if (range.aspectMask == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureRTV(this, view);
	}

	RHI_DepthStencilView* VulkanTexture::create_dsv()
	{
		vk::ImageSubresourceRange range(VulkanEnums::dsv_aspect(aspect()), 0, 1, 0, 1);

		if (range.aspectMask == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureDSV(this, view);
	}

	VulkanTexture::~VulkanTexture()
	{
		vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	VulkanTextureSRV::VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		m_texture->owner()->add_reference();
	}

	VulkanTextureSRV::~VulkanTextureSRV()
	{
		DESTROY_CALL(destroyImageView, m_view);
		m_texture->owner()->release();
	}

	void VulkanTextureSRV::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			m_texture->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);
			API->m_state.m_pipeline->bind_texture(this, location);
		}
	}

	void VulkanTextureSRV::bind_combined(byte location, struct RHI_Sampler* sampler)
	{
		if (API->m_state.m_pipeline)
		{
			m_texture->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);
			API->m_state.m_pipeline->bind_texture_combined(this, static_cast<VulkanSampler*>(sampler), location);
		}
	}

	VulkanTextureUAV::VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		texture->owner()->add_reference();
	}

	VulkanTextureUAV::~VulkanTextureUAV()
	{
		DESTROY_CALL(destroyImageView, m_view);
		m_texture->owner()->release();
	}

	void VulkanTextureUAV::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			m_texture->change_layout(vk::ImageLayout::eGeneral);
			API->m_state.m_pipeline->bind_texture(this, location);
		}
	}

	VulkanTextureRTV::VulkanTextureRTV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		texture->owner()->add_reference();
	}

	VulkanTextureRTV::~VulkanTextureRTV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		DESTROY_CALL(destroyImageView, m_view);
		m_texture->owner()->release();
	}

	void VulkanTextureRTV::clear(const LinearColor& color)
	{
		auto cmd = API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue value;
		value.setFloat32({color.r, color.g, color.b, color.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(0)
		        .setBaseMipLevel(0)
		        .setLayerCount(1)
		        .setLevelCount(1);

		API->current_command_buffer_handle().clearColorImage(image(), layout(), value, range);
		cmd->add_object(this);
	}

	template<typename Target>
	static void blit_target(Target* src, Target* dst, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter,
	                        vk::ImageAspectFlagBits aspect)
	{
		auto cmd = API->end_render_pass();
		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		auto src_end = src_rect.pos + src_rect.size;
		auto dst_end = dst_rect.pos + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.pos.x, src_rect.pos.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.pos.x, dst_rect.pos.y, 0), vk::Offset3D(dst_end.x, dst_end.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));
		blit.setDstSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));

		cmd->m_cmd.blitImage(src->image(), src->layout(), dst->image(), vk::ImageLayout::eTransferDstOptimal, blit,
		                     filter_of(filter));

		cmd->add_object(dst);
		cmd->add_object(src);
	}

	void VulkanTextureRTV::blit(RHI_RenderTargetView* texture, const Rect2D& src_rect, const Rect2D& dst_rect,
	                            SamplerFilter filter)
	{
		auto src = static_cast<VulkanTextureRTV*>(texture);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eColor);
	}

	VulkanTextureDSV::VulkanTextureDSV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		texture->owner()->add_reference();
	}

	VulkanTextureDSV::~VulkanTextureDSV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		DESTROY_CALL(destroyImageView, m_view);
		m_texture->owner()->release();
	}

	void VulkanTextureDSV::clear(float depth, byte stencil)
	{
		auto cmd = API->end_render_pass();

		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearDepthStencilValue value;
		value.setDepth(depth).setStencil(stencil);
		vk::ImageSubresourceRange range;
		range.setAspectMask(m_texture->aspect()).setBaseArrayLayer(0).setBaseMipLevel(0).setLayerCount(1).setLevelCount(1);

		API->current_command_buffer_handle().clearDepthStencilImage(image(), layout(), value, range);
		cmd->add_object(this);
	}

	void VulkanTextureDSV::blit(RHI_DepthStencilView* texture, const Rect2D& src_rect, const Rect2D& dst_rect,
	                            SamplerFilter filter)
	{
		auto src = static_cast<VulkanTextureDSV*>(texture);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eDepth);
	}

	VulkanTexture2D& VulkanTexture2D::create(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		m_format = format;
		m_size   = size;
		m_mips   = mips;

		VulkanTexture::create({}, {}, flags);
		return *this;
	}

	uint_t VulkanTexture2D::layer_count() const
	{
		return 1;
	}

	vk::ImageViewType VulkanTexture2D::view_type() const
	{
		return vk::ImageViewType::e2D;
	}

	uint8_t VulkanTexture2D::mipmap_count() const
	{
		return m_mips;
	}

	vk::Format VulkanTexture2D::format() const
	{
		return VulkanEnums::from_color_format(engine_format());
	}

	ColorFormat VulkanTexture2D::engine_format() const
	{
		return m_format;
	}

	vk::ImageType VulkanTexture2D::image_type() const
	{
		return vk::ImageType::e2D;
	}

	vk::Extent3D VulkanTexture2D::extent() const
	{
		return vk::Extent3D(m_size.x, m_size.y, 1);
	}

	RHI_Object* VulkanTexture2D::owner()
	{
		return this;
	}

	void VulkanTexture2D::update(byte mip, const Rect2D& rect, const byte* data, size_t data_size)
	{
		if (data == nullptr || data_size == 0)
			return;

		auto buffer = API->m_stagging_manager->allocate(data_size, vk::BufferUsageFlagBits::eTransferSrc);
		buffer->copy(0, data, data_size);

		auto command_buffer = API->current_command_buffer();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(aspect(), mip, 0, 1),
		                           vk::Offset3D(rect.pos.x, rect.pos.y, 0), vk::Extent3D(rect.size.x, rect.size.y, 1));

		command_buffer->m_cmd.copyBufferToImage(buffer->m_buffer, image(), vk::ImageLayout::eTransferDstOptimal, region);
		command_buffer->add_object(buffer);
	}

	RHI_Texture2D* VulkanAPI::create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		return &(new VulkanTexture2D())->create(format, size, mips, flags);
	}
}// namespace Engine
