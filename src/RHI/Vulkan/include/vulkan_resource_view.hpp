#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	struct VulkanRenderTarget;
	struct VulkanSampler;
	struct VulkanTexture;
	struct VulkanBuffer;

	struct VulkanResourceView {
		static void release(vk::ImageView view);
	};

	struct VulkanSRV : public RHI_ShaderResourceView {
		virtual RHI_Object* owner()                                                                      = 0;
		virtual VulkanSRV& update_descriptor(vk::WriteDescriptorSet& descriptor, VulkanSampler* sampler) = 0;
	};

	struct VulkanUAV : public RHI_UnorderedAccessView {
		virtual RHI_Object* owner()                                              = 0;
		virtual VulkanUAV& update_descriptor(vk::WriteDescriptorSet& descriptor) = 0;
	};

	struct VulkanRTV : public RHI_RenderTargetView {
		virtual RHI_Object* owner() = 0;
	};

	struct VulkanDSV : public RHI_DepthStencilView {
		virtual RHI_Object* owner() = 0;
	};


	template<typename Super>
	struct VulkanTextureView : Super {
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureView(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view) {}
		~VulkanTextureView() { VulkanResourceView::release(m_view); }
		inline RHI_Object* owner() override { return m_texture->owner(); }
	};

	struct VulkanTextureSRV : public VulkanTextureView<VulkanSRV> {
		using VulkanTextureView::VulkanTextureView;
		VulkanTextureSRV& update_descriptor(vk::WriteDescriptorSet& descriptor, VulkanSampler* sampler) override;
	};

	struct VulkanTextureUAV : VulkanTextureView<VulkanUAV> {
		using VulkanTextureView::VulkanTextureView;
		VulkanTextureUAV& update_descriptor(vk::WriteDescriptorSet& descriptor) override;
	};

	struct VulkanTextureRTV : VulkanTextureView<VulkanRTV> {
		Set<struct VulkanRenderTarget*> m_render_targets;

		using VulkanTextureView::VulkanTextureView;
		~VulkanTextureRTV();

		void clear(const LinearColor& color) override;
		void blit(RHI_RenderTargetView* texture, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_texture->layout(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout) { return m_texture->change_layout(layout); }
	};

	struct VulkanTextureDSV : VulkanTextureView<VulkanDSV> {
		Set<struct VulkanRenderTarget*> m_render_targets;

		using VulkanTextureView::VulkanTextureView;
		~VulkanTextureDSV();

		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* texture, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_texture->layout(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout) { return m_texture->change_layout(layout); }
	};

	struct VulkanBufferSRV : public VulkanSRV {
		VulkanBuffer* m_buffer;

		VulkanBufferSRV(VulkanBuffer* buffer) : m_buffer(buffer) {}
		VulkanBufferSRV& update_descriptor_base(vk::WriteDescriptorSet& descriptor, vk::DescriptorType type);
		RHI_Object* owner() override;
	};

	template<vk::DescriptorType type>
	struct TypedVulkanBufferSRV : public VulkanBufferSRV {
		using VulkanBufferSRV::VulkanBufferSRV;

		VulkanBufferSRV& update_descriptor(vk::WriteDescriptorSet& descriptor, VulkanSampler* sampler) override
		{
			return update_descriptor_base(descriptor, type);
		}
	};

	struct VulkanBufferUAV : public VulkanUAV {
		VulkanBuffer* m_buffer;

		VulkanBufferUAV(VulkanBuffer* buffer) : m_buffer(buffer) {}
		VulkanBufferUAV& update_descriptor_base(vk::WriteDescriptorSet& descriptor, vk::DescriptorType type);
		RHI_Object* owner() override;
	};

	template<vk::DescriptorType type>
	struct TypedVulkanBufferUAV : public VulkanBufferUAV {
		using VulkanBufferUAV::VulkanBufferUAV;

		VulkanBufferUAV& update_descriptor(vk::WriteDescriptorSet& descriptor) override
		{
			return update_descriptor_base(descriptor, type);
		}
	};
}// namespace Engine
