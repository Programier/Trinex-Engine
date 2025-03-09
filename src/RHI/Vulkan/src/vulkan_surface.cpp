#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_surface.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
	struct VulkanSurfaceTexture : VulkanTexture {
		ColorFormat m_format;
		vk::Format m_vk_format;
		Vector2u m_size;

		void create(ColorFormat format, Vector2u size)
		{
			m_format    = format;
			m_vk_format = parse_engine_format(format);
			m_size      = size;

			vk::ImageUsageFlags usage = {};

			if (format.is_color())
			{
				usage = vk::ImageUsageFlagBits::eColorAttachment;
			}
			else if (format.is_depth())
			{
				usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			}
			VulkanTexture::create({}, usage);
		}

		RHI_ShaderResourceView* create_srv() override
		{
			vk::ImageSubresourceRange range(aspect(), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			return new VulkanTextureSRV(this, view);
		}

		RHI_UnorderedAccessView* create_uav() override
		{
			vk::ImageSubresourceRange range(aspect(), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			return new VulkanTextureUAV(this, view);
		}

		uint_t layer_count() const override { return 1; }
		vk::ImageViewType view_type() const override { return vk::ImageViewType::e2D; }
		Vector2u size(MipMapLevel level = 0) const override { return m_size; }
		MipMapLevel mipmap_count() const override { return 1; }
		vk::Format format() const override { return m_vk_format; }
		ColorFormat engine_format() const override { return m_format; }
		vk::ImageType image_type() const override { return vk::ImageType::e2D; }
		vk::Extent3D extent(MipMapLevel level = 0) const override { return vk::Extent3D(m_size.x, m_size.y, 1); }
	};

	VulkanSurface& VulkanSurface::create(ColorFormat format, Vector2u size)
	{
		auto surface = new VulkanSurfaceTexture();
		m_texture    = surface;
		surface->create(format, size);
		return *this;
	}

	RHI_RenderTargetView* VulkanSurface::create_rtv()
	{
		if (m_texture->engine_format().is_color())
		{
			vk::ImageSubresourceRange range(m_texture->aspect(), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			return new VulkanSurfaceRTV(this, view);
		}
		return nullptr;
	}

	RHI_DepthStencilView* VulkanSurface::create_dsv()
	{
		if (m_texture->engine_format().is_depth())
		{
			vk::ImageSubresourceRange range(m_texture->aspect(), 0, 1, 0, 1);
			vk::ImageViewCreateInfo view_info({}, image(), vk::ImageViewType::e2D, format(), {}, range);
			vk::ImageView view = API->m_device.createImageView(view_info);
			return new VulkanSurfaceDSV(this, view);
		}
		return nullptr;
	}

	RHI_ShaderResourceView* VulkanSurface::create_srv()
	{
		return m_texture->create_srv();
	}

	RHI_UnorderedAccessView* VulkanSurface::create_uav()
	{
		return m_texture->create_uav();
	}

	Vector2u VulkanSurface::size() const
	{
		return m_texture->size();
	}

	vk::Format VulkanSurface::format() const
	{
		return m_texture->format();
	}

	vk::ImageLayout VulkanSurface::layout() const
	{
		return m_texture->layout();
	}

	vk::Image VulkanSurface::image() const
	{
		return m_texture->image();
	}

	void VulkanSurface::change_layout(vk::ImageLayout layout, vk::CommandBuffer& cmd)
	{
		m_texture->change_layout(layout, cmd);
	}

	VulkanSurface::~VulkanSurface()
	{
		m_texture->release();
	}

	VulkanSurfaceRTV::VulkanSurfaceRTV(VulkanSurface* surface, vk::ImageView view) : m_surface(surface), m_view(view)
	{
		surface->add_reference();
	}

	VulkanSurfaceRTV::~VulkanSurfaceRTV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		DESTROY_CALL(destroyImageView, m_view);
		m_surface->release();
	}

	void VulkanSurfaceRTV::clear(const Color& color)
	{
		auto current_layout = layout();
		auto cmd            = API->current_command_buffer();

		if (cmd->is_inside_render_pass())
			API->end_render_pass();

		change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		vk::ClearColorValue value;
		value.setFloat32({color.r, color.g, color.b, color.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

		API->current_command_buffer_handle().clearColorImage(image(), layout(), value, range);
		change_layout(current_layout, cmd->m_cmd);

		cmd->add_object(this);
	}

	template<typename Target>
	static void blit_target(Target* src, Target* dst, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter,
							vk::ImageAspectFlagBits aspect)
	{
		auto cmd            = API->current_command_buffer();
		bool in_render_pass = cmd->is_inside_render_pass();

		if (in_render_pass)
			API->end_render_pass();

		auto src_layout = src->layout();
		auto dst_layout = dst->layout();

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal, cmd->m_cmd);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		auto src_end = src_rect.pos + Vector2i(src_rect.size);
		auto dst_end = dst_rect.pos + Vector2i(dst_rect.size);

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.pos.x, src_rect.pos.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.pos.x, dst_end.y, 0), vk::Offset3D(dst_end.x, dst_rect.pos.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));
		blit.setDstSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));

		cmd->m_cmd.blitImage(src->image(), src->layout(), dst->image(), vk::ImageLayout::eTransferDstOptimal, blit,
							 filter_of(filter));
		dst->change_layout(dst_layout, cmd->m_cmd);
		src->change_layout(src_layout, cmd->m_cmd);

		if (in_render_pass)
		{
			API->begin_render_pass();
		}

		cmd->add_object(dst);
		cmd->add_object(src);
	}

	void VulkanSurfaceRTV::blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								SamplerFilter filter)
	{
		auto src = static_cast<VulkanSurfaceRTV*>(surface);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eColor);
	}

	VulkanSurfaceDSV::VulkanSurfaceDSV(VulkanSurface* surface, vk::ImageView view) : m_surface(surface), m_view(view)
	{
		surface->add_reference();
	}

	VulkanSurfaceDSV::~VulkanSurfaceDSV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		DESTROY_CALL(destroyImageView, m_view);
		m_surface->release();
	}

	void VulkanSurfaceDSV::clear(float depth, byte stencil)
	{
		auto current_layout = layout();
		auto cmd            = API->current_command_buffer();

		if (cmd->is_inside_render_pass())
			API->end_render_pass();

		change_layout(vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		vk::ClearDepthStencilValue value;
		value.setDepth(depth).setStencil(stencil);
		vk::ImageSubresourceRange range;
		range.setAspectMask(m_surface->m_texture->aspect())
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

		API->current_command_buffer_handle().clearDepthStencilImage(image(), layout(), value, range);
		change_layout(current_layout, cmd->m_cmd);

		cmd->add_object(this);
	}

	void VulkanSurfaceDSV::blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								SamplerFilter filter)
	{
		auto src = static_cast<VulkanSurfaceDSV*>(surface);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eDepth);
	}

	RHI_Surface* VulkanAPI::create_render_surface(ColorFormat format, Vector2u size)
	{
		auto surface = new VulkanSurface();
		surface->create(format, size);
		return surface;
	}
}// namespace Engine
