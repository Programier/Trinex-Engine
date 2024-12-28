#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
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

	vk::ImageView VulkanTexture::image_view() const
	{
		return m_image_view;
	}

	vk::ImageLayout VulkanTexture::layout() const
	{
		return m_layout;
	}


	vk::ComponentMapping VulkanTexture::swizzle() const
	{
		return m_swizzle;
	}

	MipMapLevel VulkanTexture::base_mipmap()
	{
		return 0;
	}

	VulkanTexture& VulkanTexture::create(const Texture* texture)
	{
		m_layout                  = vk::ImageLayout::eUndefined;
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

		if (texture->class_instance()->is_a<RenderSurface>())
		{
			if (is_depth_stencil_image())
			{
				usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
			}
			else if (is_color_image())
			{
				usage |= vk::ImageUsageFlagBits::eColorAttachment;
			}

			usage |= vk::ImageUsageFlagBits::eTransferSrc;
		}

		vk::ImageCreateInfo info(create_flags(), image_type(), format(), extent(), mipmap_count(), layer_count(),
		                         vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage out_image = VK_NULL_HANDLE;
		auto res          = vmaCreateImage(API->m_allocator, &static_cast<VkImageCreateInfo&>(info), &alloc_info, &out_image,
		                                   &m_allocation, nullptr);
		trinex_check(res == VK_SUCCESS, "Failed to create texture!");
		m_image = out_image;

		// Creating image view
		m_swizzle = vk::ComponentMapping(get_type(texture->swizzle_r), get_type(texture->swizzle_g), get_type(texture->swizzle_b),
		                                 get_type(texture->swizzle_a));
		m_image_view = create_image_view(vk::ImageSubresourceRange(aspect(true), 0, mipmap_count(), 0, layer_count()));


		change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);
		return *this;
	}

	void VulkanTexture::update_texture(const Size2D& size, MipMapLevel level, uint_t layer, const byte* data, size_t data_size)
	{
		if (data == nullptr || data_size == 0)
			return;

		auto buffer = API->m_stagging_manager->allocate(data_size, vk::BufferUsageFlagBits::eTransferSrc);
		buffer->copy(0, data, data_size);

		vk::ImageMemoryBarrier barrier;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout           = m_layout;
		barrier.newLayout           = vk::ImageLayout::eTransferDstOptimal;
		barrier.image               = m_image;
		barrier.subresourceRange    = vk::ImageSubresourceRange(aspect(), level, 1, layer, 1);

		Barrier::transition_image_layout(barrier);
		auto command_buffer = API->begin_single_time_command_buffer();

		vk::BufferImageCopy region(0, 0, 0, vk::ImageSubresourceLayers(aspect(), level, layer, 1), vk::Offset3D(0, 0, 0),
		                           vk::Extent3D(static_cast<uint_t>(size.x), static_cast<uint_t>(size.y), 1));

		command_buffer.copyBufferToImage(buffer->m_buffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);
		API->end_single_time_command_buffer(command_buffer);

		API->m_stagging_manager->release(buffer);

		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = m_layout;
		Barrier::transition_image_layout(barrier);
	}

	void VulkanTexture::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			API->m_state.m_pipeline->bind_texture(this, location);
		}
	}

	void VulkanTexture::bind_combined(RHI_Sampler* sampler, BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			trinex_always_check(sampler, "Sampler can't be null!");
			API->m_state.m_pipeline->bind_texture_combined(this, reinterpret_cast<VulkanSampler*>(sampler), location);
		}
	}

	vk::ImageAspectFlags VulkanTexture::aspect(bool use_for_shader_attachment) const
	{
		if (is_color_image())
		{
			return vk::ImageAspectFlagBits::eColor;
		}
		else if (is_in<parse_engine_format(ColorFormat::DepthStencil)>(format()))
		{
			if (use_for_shader_attachment)
			{
				return vk::ImageAspectFlagBits::eDepth;
			}
			else
			{
				return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			}
		}

		return vk::ImageAspectFlagBits::eDepth;
	}

	bool VulkanTexture::is_color_image() const
	{
		return is_in<ColorFormat::FloatR, ColorFormat::FloatRGBA, ColorFormat::R8, ColorFormat::R8G8B8A8, ColorFormat::BC1,
		             ColorFormat::BC2, ColorFormat::BC3>(engine_format());
	}

	bool VulkanTexture::is_render_target_color_image() const
	{
		return is_in<ColorFormat::R8G8B8A8, ColorFormat::FloatRGBA>(engine_format());
	}

	bool VulkanTexture::is_depth_stencil_image() const
	{
		return is_depth_stencil_image(engine_format());
	}

	bool VulkanTexture::is_depth_stencil_image(ColorFormat format)
	{
		return is_in<ColorFormat::DepthStencil, ColorFormat::Depth, ColorFormat::ShadowDepth>(format);
	}

	vk::ImageView VulkanTexture::create_image_view(const vk::ImageSubresourceRange& range)
	{
		vk::ImageViewCreateInfo view_info({}, m_image, view_type(), format(), m_swizzle, range);
		return API->m_device.createImageView(view_info);
	}


	VulkanTexture& VulkanTexture::layout(vk::ImageLayout layout)
	{
		m_layout = layout;
		return *this;
	}

	vk::ImageLayout VulkanTexture::change_layout(vk::ImageLayout new_layout)
	{
		if (layout() != new_layout)
		{
			auto base_mip = base_mipmap();

			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;

			barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), base_mip, mipmap_count() - base_mip, 0, layer_count());
			m_layout                 = barrier.newLayout;

			Barrier::transition_image_layout(barrier);

			m_layout = new_layout;
		}
		return new_layout;
	}

	vk::ImageLayout VulkanTexture::change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd)
	{
		if (layout() != new_layout)
		{
			auto base_mip = base_mipmap();

			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;

			barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), base_mip, mipmap_count() - base_mip, 0, layer_count());
			m_layout                 = barrier.newLayout;

			Barrier::transition_image_layout(cmd, barrier);

			m_layout = new_layout;
		}
		return new_layout;
	}


	VulkanTexture::~VulkanTexture()
	{
		DESTROY_CALL(destroyImageView, m_image_view);
		vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	VulkanTexture2D& VulkanTexture2D::create(const Texture2D* texture)
	{
		m_texture = texture;
		VulkanTexture::create(texture);

		for (MipMapLevel i = 0, count = texture->mipmap_count(); i < count; ++i)
		{
			if (auto mip = texture->mip(i))
			{
				update_texture(mip->size, i, 0, mip->data.data(), mip->data.size());
			}
		}
		return *this;
	}

	uint_t VulkanTexture2D::layer_count() const
	{
		return 1;
	}

	vk::ImageCreateFlagBits VulkanTexture2D::create_flags() const
	{
		return {};
	}

	vk::ImageViewType VulkanTexture2D::view_type() const
	{
		return vk::ImageViewType::e2D;
	}

	Size2D VulkanTexture2D::size(MipMapLevel level) const
	{
		return m_texture->size(level);
	}

	MipMapLevel VulkanTexture2D::mipmap_count() const
	{
		return m_texture->mipmap_count();
	}

	vk::Format VulkanTexture2D::format() const
	{
		return parse_engine_format(engine_format());
	}

	ColorFormat VulkanTexture2D::engine_format() const
	{
		return m_texture->format();
	}

	vk::ImageType VulkanTexture2D::image_type() const
	{
		return vk::ImageType::e2D;
	}

	vk::Extent3D VulkanTexture2D::extent(MipMapLevel level) const
	{
		auto texture_size = size();
		return vk::Extent3D(texture_size.x, texture_size.y, 1.f);
	}

	VulkanSurface& VulkanSurface::create(const Texture2D* texture)
	{
		m_size = texture->size();
		VulkanTexture2D::create(texture);
		return *this;
	}

	Size2D VulkanSurface::size(MipMapLevel level) const
	{
		return m_size;
	}

	void VulkanSurface::clear_color(const Color& color)
	{
		if (is_color_image())
		{
			auto current_layout = layout();
			auto cmd            = API->current_command_buffer();
			change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

			vk::ClearColorValue value;
			value.setFloat32({color.r, color.g, color.b, color.a});

			vk::ImageSubresourceRange range;
			range.setAspectMask(aspect(false))
			        .setBaseArrayLayer(0)
			        .setBaseMipLevel(0)
			        .setLayerCount(layer_count())
			        .setLevelCount(mipmap_count());


			API->current_command_buffer_handle().clearColorImage(image(), layout(), value, range);
			change_layout(current_layout, cmd->m_cmd);

			cmd->add_object(this);
		}
	}

	void VulkanSurface::clear_depth_stencil(float depth, byte stencil)
	{
		if (is_depth_stencil_image())
		{
			auto current_layout = layout();
			auto cmd            = API->current_command_buffer();
			change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

			vk::ClearDepthStencilValue value;
			value.setDepth(depth).setStencil(stencil);
			vk::ImageSubresourceRange range;
			range.setAspectMask(aspect(false))
			        .setBaseArrayLayer(0)
			        .setBaseMipLevel(0)
			        .setLayerCount(layer_count())
			        .setLevelCount(mipmap_count());


			API->current_command_buffer_handle().clearDepthStencilImage(image(), layout(), value, range);
			change_layout(current_layout, cmd->m_cmd);

			cmd->add_object(this);
		}
	}

	void VulkanSurface::blit(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		auto cmd            = API->current_command_buffer();
		bool in_render_pass = cmd->is_inside_render_pass();

		if (in_render_pass)
			API->end_render_pass();

		auto src = surface->rhi_object<VulkanSurface>();

		auto src_layout = src->layout();
		auto dst_layout = layout();

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal, cmd->m_cmd);
		change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		auto src_end = src_rect.position + src_rect.size;
		auto dst_end = dst_rect.position + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.position.x, src_rect.position.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.position.x, dst_end.y, 0), vk::Offset3D(dst_end.x, dst_rect.position.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(src->aspect(), 0, 0, src->layer_count()));
		blit.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		cmd->m_cmd.blitImage(src->image(), src->layout(), image(), vk::ImageLayout::eTransferDstOptimal, blit, filter_of(filter));

		change_layout(dst_layout, cmd->m_cmd);
		src->change_layout(src_layout, cmd->m_cmd);

		if (in_render_pass)
		{
			API->begin_render_pass();
		}

		cmd->add_object(this);
		cmd->add_object(src);
	}

	VulkanSurface::~VulkanSurface()
	{
		while (!m_render_targets.empty())
		{
			VulkanRenderTarget* rt = *m_render_targets.begin();
			delete rt;
		}
	}

	RHI_Texture2D* VulkanAPI::create_texture_2d(const Texture2D* texture)
	{
		return &(new VulkanTexture2D())->create(texture);
	}

	RHI_Texture2D* VulkanAPI::create_render_surface(const RenderSurface* surface)
	{
		return &(new VulkanSurface())->create(surface);
	}
}// namespace Engine
