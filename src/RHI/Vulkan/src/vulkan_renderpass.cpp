#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	namespace
	{
		static FORCE_INLINE vk::Format surface_format_of(const RenderSurface* surface)
		{
			return surface ? surface->rhi_object<VulkanSurface>()->format() : vk::Format::eUndefined;
		}

		struct VulkanRenderPassBuilder {
			vk::SubpassDescription m_subpass;
			vk::SubpassDependency m_dependency;

			Vector<vk::AttachmentDescription> m_attachment_descriptions;
			Vector<vk::AttachmentReference> m_color_attachment_references;
			vk::AttachmentReference m_depth_attachment_renference;

			static inline vk::AttachmentDescription create_attachment_desctiption(vk::Format format, vk::ImageLayout layout,
																				  bool has_stencil)
			{
				return vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1,
												 vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
												 has_stencil ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,   //
												 has_stencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,//
												 layout, layout);
			}

			VulkanRenderPassBuilder& create_attachment_descriptions(const RenderSurface** targets)
			{
				for (int i = 0; i < 4 && targets[i]; ++i)
				{
					vk::Format format = surface_format_of(targets[i]);
					m_attachment_descriptions.push_back(
							create_attachment_desctiption(format, vk::ImageLayout::eColorAttachmentOptimal, false));
				}

				if (targets[4])
				{
					const bool has_stencil = is_in<ColorFormat::DepthStencil>(targets[4]->format());
					vk::ImageLayout layout = has_stencil ? vk::ImageLayout::eDepthStencilAttachmentOptimal
														 : vk::ImageLayout::eDepthAttachmentOptimal;

					vk::Format format = surface_format_of(targets[4]);
					m_attachment_descriptions.push_back(create_attachment_desctiption(format, layout, has_stencil));
				}

				return *this;
			}

			VulkanRenderPassBuilder& create_attachment_references(const RenderSurface** targets)
			{
				for (int_t index = 0; index < 4 && targets[index]; ++index)
				{
					m_color_attachment_references.push_back(
							vk::AttachmentReference(index, vk::ImageLayout::eColorAttachmentOptimal));
				}

				if (targets[4])
				{
					vk::ImageLayout layout;

					switch (targets[4]->format())
					{
						case ColorFormat::ShadowDepth:
						case ColorFormat::Depth:
							layout = vk::ImageLayout::eDepthAttachmentOptimal;
							break;

						case ColorFormat::DepthStencil:
							layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
							break;

						default:
							layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
							break;
					}

					m_depth_attachment_renference = vk::AttachmentReference(m_attachment_descriptions.size() - 1, layout);
				}
				return *this;
			}

			vk::RenderPass build(const RenderSurface** targets)
			{
				create_attachment_descriptions(targets);
				create_attachment_references(targets);
				return build();
			}

			vk::RenderPass build()
			{
				info_log("Vulkan", "New Render Pass");

				bool has_depth_attachment = m_depth_attachment_renference.layout != vk::ImageLayout::eUndefined;

				m_subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
												   m_color_attachment_references, {},
												   has_depth_attachment ? &m_depth_attachment_renference : nullptr);


				vk::PipelineStageFlags src_pipeline_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				vk::PipelineStageFlags dst_pipeline_flags =
						vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader |
						vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer;

				vk::AccessFlags src_access_flags =
						vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
				vk::AccessFlags dst_access_flags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eTransferRead;

				if (has_depth_attachment)
				{
					src_pipeline_flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
					src_access_flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
				}

				m_dependency = vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, src_pipeline_flags, dst_pipeline_flags,
													 src_access_flags, dst_access_flags, vk::DependencyFlagBits::eByRegion);

				return API->m_device.createRenderPass(vk::RenderPassCreateInfo(
						vk::RenderPassCreateFlags(), m_attachment_descriptions, m_subpass, m_dependency));
			}
		};

	}// namespace

	TreeMap<VulkanRenderPass::Key, VulkanRenderPass*> VulkanRenderPass::m_render_passes;

	void VulkanRenderPass::Key::init(vk::Format format)
	{
		m_attachments[5] = format;

		for (size_t i = 0; i < 5; ++i)
		{
			m_attachments[i] = vk::Format::eUndefined;
		}
	}

	void VulkanRenderPass::Key::init(const RenderSurface** targets)
	{

		for (int i = 0; i < 5; ++i)
		{
			m_attachments[i] = surface_format_of(targets[i]);
		}

		m_attachments[5] = vk::Format::eUndefined;
	}

	bool VulkanRenderPass::Key::operator<(const Key& key) const
	{
		return std::memcmp(m_attachments, key.m_attachments, sizeof(m_attachments)) < 0;
	}

	VulkanRenderPass* VulkanRenderPass::find_or_create(const RenderSurface** targets)
	{

		Key key;
		key.init(targets);

		VulkanRenderPass*& pass = m_render_passes[key];

		if (pass != nullptr)
			return pass;


		VulkanRenderPassBuilder builder;

		if (auto vk_pass = builder.build(targets))
		{
			(pass = new VulkanRenderPass(vk_pass));
			return pass;
		}

		throw EngineException("Failed to create render pass!");
	}

	VulkanRenderPass* VulkanRenderPass::swapchain_render_pass(vk::Format format)
	{
		Key key;
		key.init(format);

		VulkanRenderPass*& pass = m_render_passes[key];

		if (!pass)
		{
			VulkanRenderPassBuilder builder;

			builder.m_attachment_descriptions.push_back(VulkanRenderPassBuilder::create_attachment_desctiption(
			        format, vk::ImageLayout::eColorAttachmentOptimal, false));

			builder.m_color_attachment_references = {vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)};


			if (auto vk_pass = builder.build())
			{
				pass = new VulkanRenderPass(vk_pass);
				return pass;
			}
			else
			{
				throw EngineException("Failed to create swapchain render pass!");
			}
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

	VulkanRenderPass::VulkanRenderPass(vk::RenderPass rp) : m_render_pass(rp)
	{}

	VulkanRenderPass::~VulkanRenderPass()
	{
		DESTROY_CALL(destroyRenderPass, m_render_pass);
	}
}// namespace Engine
