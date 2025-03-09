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

	vk::ImageLayout VulkanTexture::layout() const
	{
		return m_layout;
	}

	VulkanTexture& VulkanTexture::create(vk::ImageCreateFlagBits flags, vk::ImageUsageFlags usage)
	{
		m_layout = vk::ImageLayout::eUndefined;
		usage |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;

		vk::ImageCreateInfo info(flags, image_type(), format(), extent(), mipmap_count(), layer_count(),
		                         vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage out_image = VK_NULL_HANDLE;
		auto res          = vmaCreateImage(API->m_allocator, &static_cast<VkImageCreateInfo&>(info), &alloc_info, &out_image,
		                                   &m_allocation, nullptr);
		trinex_check(res == VK_SUCCESS, "Failed to create texture!");
		m_image = out_image;
		change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);
		return *this;
	}

	void VulkanTexture::update_texture(const Vector2u& size, MipMapLevel level, uint_t layer, const byte* data, size_t data_size)
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

	VulkanTexture& VulkanTexture::layout(vk::ImageLayout layout)
	{
		m_layout = layout;
		return *this;
	}

	void VulkanTexture::change_layout(vk::ImageLayout new_layout)
	{
		if (layout() != new_layout)
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

	void VulkanTexture::change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd)
	{
		if (layout() != new_layout)
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;

			barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), 0, mipmap_count(), 0, layer_count());
			m_layout                 = barrier.newLayout;

			Barrier::transition_image_layout(cmd, barrier);

			m_layout = new_layout;
		}
	}


	VulkanTexture::~VulkanTexture()
	{
		vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	VulkanTexture2D& VulkanTexture2D::create(const Texture2D* texture)
	{
		m_texture = texture;
		VulkanTexture::create({}, {});

		size_t index = 0;
		for (auto& mip : texture->mips)
		{
			update_texture(mip.size, index, 0, mip.data.data(), mip.data.size());
		}
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

	Vector2u VulkanTexture2D::size(MipMapLevel level) const
	{
		return m_texture->size(level);
	}

	MipMapLevel VulkanTexture2D::mipmap_count() const
	{
		return m_texture->mips.size();
	}

	vk::Format VulkanTexture2D::format() const
	{
		return parse_engine_format(engine_format());
	}

	ColorFormat VulkanTexture2D::engine_format() const
	{
		return m_texture->format;
	}

	vk::ImageType VulkanTexture2D::image_type() const
	{
		return vk::ImageType::e2D;
	}

	vk::Extent3D VulkanTexture2D::extent(MipMapLevel level) const
	{
		auto texture_size = size(level);
		return vk::Extent3D(texture_size.x, texture_size.y, 1.f);
	}

	RHI_ShaderResourceView* VulkanTexture2D::create_srv()
	{
		vk::ImageSubresourceRange range(aspect(), 0, mipmap_count(), 0, layer_count());
		vk::ImageViewCreateInfo view_info({}, image(), view_type(), format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);
		return new VulkanTextureSRV(this, view);
	}

	RHI_UnorderedAccessView* VulkanTexture2D::create_uav()
	{
		return nullptr;
	}

	VulkanTextureSRV::VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		m_texture->add_reference();
	}

	VulkanTextureSRV::~VulkanTextureSRV()
	{
		DESTROY_CALL(destroyImageView, m_view);
		m_texture->release();
	}

	void VulkanTextureSRV::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
			API->m_state.m_pipeline->bind_texture(this, location);
	}

	void VulkanTextureSRV::bind_combined(byte location, struct RHI_Sampler* sampler)
	{
		if (API->m_state.m_pipeline)
			API->m_state.m_pipeline->bind_texture_combined(this, static_cast<VulkanSampler*>(sampler), location);
	}

	VulkanTextureUAV::VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		texture->add_reference();
	}

	VulkanTextureUAV::~VulkanTextureUAV()
	{
		DESTROY_CALL(destroyImageView, m_view);
		m_texture->release();
	}

	void VulkanTextureUAV::bind(BindLocation location) {}

	RHI_Texture* VulkanAPI::create_texture_2d(const Texture2D* texture)
	{
		return &(new VulkanTexture2D())->create(texture);
	}
}// namespace Engine
