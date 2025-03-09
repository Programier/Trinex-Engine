#pragma once
#include <Core/build.hpp>
#include <Core/etl/map.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanSurfaceRTV;
	struct VulkanSurfaceDSV;

	struct VulkanRenderPass {
		struct Key {
			vk::Format m_attachments[6];

			void init(vk::Format format);
			void init(VulkanSurfaceRTV** targets, VulkanSurfaceDSV* depth);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderPass*> m_render_passes;

		vk::RenderPass m_render_pass;

		static VulkanRenderPass* find_or_create(VulkanSurfaceRTV** targets, VulkanSurfaceDSV* depth);
		static VulkanRenderPass* swapchain_render_pass(vk::Format format);
		static void destroy_all();

	private:
		VulkanRenderPass(vk::RenderPass pass);
		~VulkanRenderPass();
	};
}// namespace Engine
