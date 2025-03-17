#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanTexture {
	private:
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		vk::Image m_image;
		vk::ImageLayout m_layout;
		TextureCreateFlags m_flags;

	public:
		virtual uint_t layer_count() const          = 0;
		virtual vk::ImageViewType view_type() const = 0;
		virtual uint8_t mipmap_count() const        = 0;
		virtual vk::Format format() const           = 0;
		virtual ColorFormat engine_format() const   = 0;
		virtual vk::ImageType image_type() const    = 0;
		virtual vk::Extent3D extent() const         = 0;
		virtual RHI_Object* owner()                 = 0;

		vk::Image image() const;
		vk::ImageLayout layout() const;
		vk::ImageAspectFlags aspect() const;

		VulkanTexture& create(vk::ImageCreateFlagBits flags, vk::ImageUsageFlags usage, TextureCreateFlags create_flags);
		void change_layout(vk::ImageLayout new_layout);

		RHI_ShaderResourceView* create_srv();
		RHI_UnorderedAccessView* create_uav();
		RHI_RenderTargetView* create_rtv();
		RHI_DepthStencilView* create_dsv();
		~VulkanTexture();
	};

	struct VulkanTextureSRV : public RHI_DefaultDestroyable<RHI_ShaderResourceView> {
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureSRV();

		void bind(BindLocation location) override;
		void bind_combined(byte location, struct RHI_Sampler* sampler) override;
	};

	struct VulkanTextureUAV : public RHI_DefaultDestroyable<RHI_UnorderedAccessView> {
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureUAV();

		void bind(BindLocation location) override;
	};

	struct VulkanTextureRTV : RHI_DefaultDestroyable<RHI_RenderTargetView> {
		Set<struct VulkanRenderTarget*> m_render_targets;
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureRTV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureRTV();

		void clear(const Color& color) override;
		void blit(RHI_RenderTargetView* texture, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_texture->layout(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout) { return m_texture->change_layout(layout); }
	};

	struct VulkanTextureDSV : RHI_DefaultDestroyable<RHI_DepthStencilView> {
		Set<struct VulkanRenderTarget*> m_render_targets;

		VulkanTexture* m_texture;
		vk::ImageView m_view;
		bool m_with_stencil;

		VulkanTextureDSV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureDSV();

		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* texture, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_texture->layout(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout) { return m_texture->change_layout(layout); }
	};

	struct VulkanTexture2D : RHI_DefaultDestroyable<RHI_Texture2D>, VulkanTexture {
		ColorFormat m_format;
		Vector2u m_size;
		uint8_t m_mips;

	public:
		VulkanTexture2D& create(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags);

		uint_t layer_count() const override;
		vk::ImageViewType view_type() const override;
		uint8_t mipmap_count() const override;
		vk::Format format() const override;
		ColorFormat engine_format() const override;
		vk::ImageType image_type() const override;
		vk::Extent3D extent() const override;
		RHI_Object* owner() override;

		void update(byte mip, const Rect2D& rect, const byte* data, size_t data_size) override;
		inline RHI_ShaderResourceView* create_srv() override { return VulkanTexture::create_srv(); }
		inline RHI_UnorderedAccessView* create_uav() override { return VulkanTexture::create_uav(); }
		inline RHI_RenderTargetView* create_rtv() override { return VulkanTexture::create_rtv(); }
		inline RHI_DepthStencilView* create_dsv() override { return VulkanTexture::create_dsv(); }
	};
}// namespace Engine
