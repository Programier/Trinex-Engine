#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanRenderPass;
	class VulkanContext;

	class VulkanRenderTarget
	{
	private:
	public:
		struct Key {
			class VulkanTextureRTV* m_surfaces[4];
			class VulkanTextureDSV* m_depth;

			void init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderTarget*> m_render_targets;

		Key m_key;
		vk::Framebuffer m_framebuffer;
		VulkanRenderPass* m_render_pass = nullptr;
		uint16_t m_width;
		uint16_t m_height;

		VulkanRenderTarget();

	public:
		static VulkanRenderTarget* find_or_create(VulkanTextureRTV** targets, VulkanTextureDSV* depth);

		VulkanRenderTarget& init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
		inline vk::Framebuffer framebuffer() const { return m_framebuffer; }
		inline VulkanRenderPass* render_pass() const { return m_render_pass; }
		inline uint16_t width() const { return m_width; }
		inline uint16_t height() const { return m_height; }
		Vector2u16 size() const { return {m_width, m_height}; }
		~VulkanRenderTarget();
	};
}// namespace Engine
