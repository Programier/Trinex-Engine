#pragma once
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanTextureRTV;
	class VulkanTextureDSV;

	class VulkanRenderPass
	{
	public:
		struct Key {
			vk::Format m_attachments[6];

			void init(VulkanTextureRTV** targets, VulkanTextureDSV* depth);
			inline bool operator<(const Key& key) const
			{
				for (size_t i = 0; i < 6; i++)
				{
					if (m_attachments[i] < key.m_attachments[i])
						return true;
					if (m_attachments[i] > key.m_attachments[i])
						return false;
				}
				return false;
			}
		};

	private:
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
