#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanTexture : RHI_DefaultDestroyable<RHI_Texture2D> {
	private:
		VmaAllocation m_allocation = VK_NULL_HANDLE;

		vk::Image m_image;
		vk::ImageView m_image_view;
		vk::ImageLayout m_layout;
		vk::ComponentMapping m_swizzle;

	public:
		virtual uint_t layer_count() const                       = 0;
		virtual vk::ImageCreateFlagBits create_flags() const     = 0;
		virtual vk::ImageViewType view_type() const              = 0;
		virtual Size2D size(MipMapLevel level = 0) const         = 0;
		virtual MipMapLevel mipmap_count() const                 = 0;
		virtual vk::Format format() const                        = 0;
		virtual ColorFormat engine_format() const                = 0;
		virtual vk::ImageType image_type() const                 = 0;
		virtual vk::Extent3D extent(MipMapLevel level = 0) const = 0;

		vk::Image image() const;
		vk::ImageView image_view() const;
		vk::ImageLayout layout() const;
		vk::ComponentMapping swizzle() const;
		MipMapLevel base_mipmap();
		vk::ImageAspectFlags aspect(bool use_for_shader_attachment = false) const;

		VulkanTexture& create(const class Texture* texture);

		void update_texture(const Size2D& size, MipMapLevel level, uint_t layer, const byte* data, size_t data_size);

		void bind(BindLocation location) override;
		void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
		bool is_color_image() const;
		bool is_render_target_color_image() const;
		bool is_depth_stencil_image() const;
		static bool is_depth_stencil_image(ColorFormat);

		vk::ImageView create_image_view(const vk::ImageSubresourceRange& range);
		VulkanTexture& layout(vk::ImageLayout layout);
		vk::ImageLayout change_layout(vk::ImageLayout new_layout);
		vk::ImageLayout change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd);

		~VulkanTexture();
	};

	struct VulkanTexture2D : VulkanTexture {
		const Texture2D* m_texture;

	public:
		VulkanTexture2D& create(const Texture2D* texture);

		uint_t layer_count() const override;
		vk::ImageCreateFlagBits create_flags() const override;
		vk::ImageViewType view_type() const override;
		Size2D size(MipMapLevel level = 0) const override;
		MipMapLevel mipmap_count() const override;
		vk::Format format() const override;
		ColorFormat engine_format() const override;
		vk::ImageType image_type() const override;
		vk::Extent3D extent(MipMapLevel level = 0) const override;
	};

	struct VulkanSurface : public VulkanTexture2D {
		Set<struct VulkanRenderTarget*> m_render_targets;
		Size2D m_size;

		VulkanSurface& create(const Texture2D* texture);
		Size2D size(MipMapLevel level = 0) const override;
		void clear_color(const Color& color) override;
		void clear_depth_stencil(float depth, byte stencil) override;
		void blit(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		~VulkanSurface();
	};
}// namespace Engine
