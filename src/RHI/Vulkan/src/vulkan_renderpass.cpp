#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{

	TreeMap<VulkanRenderPass::Key, VulkanRenderPass*> VulkanRenderPass::m_render_passes;

	void VulkanRenderPass::Key::init(vk::Format format)
	{
		m_color_attachments[RHI_MAX_RT_BINDED] = format;

		for (size_t i = 0; i < RHI_MAX_RT_BINDED; ++i)
		{
			m_color_attachments[i] = vk::Format::eUndefined;
		}

		m_depth_stencil = vk::Format::eUndefined;
	}

	void VulkanRenderPass::Key::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		size_t index = 0;

		for (size_t count = color_attachments.size(); index < count; ++index)
		{
			m_color_attachments[index] = color_attachments[index]->rhi_object<VulkanSurface>()->format();
		}

		for (; index <= RHI_MAX_RT_BINDED; ++index)
		{
			m_color_attachments[index] = vk::Format::eUndefined;
		}

		m_depth_stencil = depth_stencil ? depth_stencil->rhi_object<VulkanSurface>()->format() : vk::Format::eUndefined;
	}

	bool VulkanRenderPass::Key::operator<(const Key& key) const
	{
		return std::memcmp(this, &key, sizeof(*this)) < 0;
	}

	static constexpr inline vk::AttachmentDescription create_attachment_desctiption(vk::Format format, vk::ImageLayout layout,
																					bool has_stencil)
	{
		return vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1,
										 vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
										 has_stencil ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,	//
										 has_stencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,//
										 layout, layout);
	}

	VulkanRenderPass* VulkanRenderPass::find_or_create(const Span<RenderSurface*>& color_attachments,
													   RenderSurface* depth_stencil)
	{
		Key key;
		key.init(color_attachments, depth_stencil);

		VulkanRenderPass*& pass = m_render_passes[key];

		if (pass != nullptr)
			return pass;

		pass = new VulkanRenderPass();
		pass->init(color_attachments, depth_stencil);

		return pass;
	}

	VulkanRenderPass* VulkanRenderPass::swapchain_render_pass(vk::Format format)
	{
		Key key;
		key.init(format);

		VulkanRenderPass*& pass = m_render_passes[key];

		if (!pass)
		{
			pass = new VulkanRenderPass();

			pass->m_has_depth_attachment = false;
			pass->m_attachment_descriptions.push_back(
					create_attachment_desctiption(format, vk::ImageLayout::eColorAttachmentOptimal, false));

			pass->m_color_attachment_references = {
					vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
			};

			pass->create();
		}

		return pass;
	}

	void VulkanRenderPass::destroy_all()
	{
		for (auto& render_pass : m_render_passes)
		{
			delete render_pass.second;
		}

		m_render_passes.clear();
	}

	VulkanRenderPass& VulkanRenderPass::create_attachment_descriptions(const Span<RenderSurface*>& color_attachments,
																	   RenderSurface* depth_stencil)
	{
		for (const RenderSurface* surface : color_attachments)
		{
			vk::Format format = surface->rhi_object<VulkanTexture>()->format();
			m_attachment_descriptions.push_back(
					create_attachment_desctiption(format, vk::ImageLayout::eColorAttachmentOptimal, false));
		}

		if (depth_stencil)
		{
			const bool has_stencil = is_in<ColorFormat::DepthStencil>(depth_stencil->format());
			vk::ImageLayout layout =
					has_stencil ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimal;

			m_has_depth_attachment = true;
			vk::Format format	   = depth_stencil->rhi_object<VulkanTexture>()->format();
			m_attachment_descriptions.push_back(create_attachment_desctiption(format, layout, has_stencil));
		}

		return *this;
	}

	VulkanRenderPass& VulkanRenderPass::create_attachment_references(const Span<RenderSurface*>& color_attachments,
																	 RenderSurface* depth_stencil)
	{
		for (int_t index = 0, count = color_attachments.size(); index < count; ++index)
		{
			m_color_attachment_references.push_back(vk::AttachmentReference(index, vk::ImageLayout::eColorAttachmentOptimal));
		}

		if (depth_stencil)
		{
			vk::ImageLayout layout;

			switch (depth_stencil->format())
			{
				case ColorFormat::ShadowDepth:
				case ColorFormat::FilteredShadowDepth:
				case ColorFormat::D32F:
					layout = vk::ImageLayout::eDepthAttachmentOptimal;
					break;

				case ColorFormat::DepthStencil:
					layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
					break;

				default:
					layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
					break;
			}

			m_depth_attachment_renference = vk::AttachmentReference(color_attachments.size(), layout);
		}
		return *this;
	}

	VulkanRenderPass& VulkanRenderPass::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		create_attachment_descriptions(color_attachments, depth_stencil);
		create_attachment_references(color_attachments, depth_stencil);
		return create();
	}

	VulkanRenderPass& VulkanRenderPass::create()
	{
		info_log("Vulkan", "New Render Pass");
		m_subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
										   m_color_attachment_references, {},
										   m_has_depth_attachment ? &m_depth_attachment_renference : nullptr);


		vk::PipelineStageFlags src_pipeline_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::PipelineStageFlags dst_pipeline_flags =
				vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader |
				vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer;

		vk::AccessFlags src_access_flags = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
		vk::AccessFlags dst_access_flags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eTransferRead;

		if (m_has_depth_attachment)
		{
			src_pipeline_flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
			src_access_flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		}

		m_dependency = vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, src_pipeline_flags, dst_pipeline_flags, src_access_flags,
											 dst_access_flags, vk::DependencyFlagBits::eByRegion);

		m_render_pass = API->m_device.createRenderPass(
				vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), m_attachment_descriptions, m_subpass, m_dependency));

		return *this;
	}

	uint_t VulkanRenderPass::attachments_count() const
	{
		return m_attachment_descriptions.size();
	}

	uint_t VulkanRenderPass::color_attachments_count() const
	{
		uint_t attachments = attachments_count();

		if (m_has_depth_attachment)
		{
			--attachments;
		}
		return attachments;
	}

	VulkanRenderPass& VulkanRenderPass::destroy()
	{
		DESTROY_CALL(destroyRenderPass, m_render_pass);

		m_attachment_descriptions.clear();
		m_color_attachment_references.clear();
		m_has_depth_attachment = false;
		return *this;
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		destroy();
	}
}// namespace Engine
