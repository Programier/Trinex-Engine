#pragma once
#include <Core/etl/map.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanTextureRTV;
	class VulkanTextureDSV;

	class VulkanRenderPass
	{
	private:
		struct Key {
			vk::Format m_attachments[6];

			void init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderPass*> m_render_passes;
		vk::RenderPass m_render_pass;

	public:
		static VulkanRenderPass* find_or_create(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
		static void destroy_all();

		inline vk::RenderPass render_pass() const { return m_render_pass; }

	private:
		VulkanRenderPass(vk::RenderPass pass);
		~VulkanRenderPass();
	};
}// namespace Engine
