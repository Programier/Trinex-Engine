#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanRenderTargetBase {
		vk::Framebuffer m_framebuffer;
		class VulkanRenderPass* m_render_pass = nullptr;
		Vector2u m_size;

		VulkanRenderTargetBase& post_init(vk::ImageView* image_views, uint32_t count);
		VulkanRenderTargetBase& size(uint32_t width, uint32_t height);

		void bind();
		virtual VulkanRenderTargetBase& lock_surfaces() = 0;
		virtual ~VulkanRenderTargetBase();
	};

	struct VulkanRenderTarget : VulkanRenderTargetBase {
		struct Key {
			struct VulkanTextureRTV* m_surfaces[4];
			struct VulkanTextureDSV* m_depth;

			void init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderTarget*> m_render_targets;
		Key m_key;

		static VulkanRenderTarget* find_or_create(VulkanTextureRTV** targets, VulkanTextureDSV* depth);

		VulkanRenderTarget();
		VulkanRenderTarget& init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
		VulkanRenderTargetBase& lock_surfaces() override;
		~VulkanRenderTarget();
	};
}// namespace Engine
