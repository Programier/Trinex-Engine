#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_resource_view.hpp>

namespace Engine
{
	namespace
	{
		static FORCE_INLINE vk::Format surface_format_of(const VulkanTextureRTV* surface)
		{
			return surface ? surface->format() : vk::Format::eUndefined;
		}

		static FORCE_INLINE vk::Format surface_format_of(const VulkanTextureDSV* surface)
		{
			return surface ? surface->format() : vk::Format::eUndefined;
		}

		struct VulkanRenderPassBuilder {
			vk::SubpassDescription m_subpass;

			vk::AttachmentDescription m_descriptions[5];
			vk::AttachmentReference m_references[5];

			uint_t m_attachments_count = 0;

			static inline vk::AttachmentDescription create_desctiption(vk::Format format, vk::ImageLayout layout,
			                                                           bool has_stencil)
			{
				return vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1,
				                                 vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
				                                 has_stencil ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,   //
				                                 has_stencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,//
				                                 layout, layout);
			}

			VulkanRenderPassBuilder& create_attachment_descriptions(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
			{
				for (int i = 0; i < 4; ++i)
				{
					if (targets[i])
					{
						vk::Format format = surface_format_of(targets[i]);
						m_descriptions[m_attachments_count] =
						        create_desctiption(format, vk::ImageLayout::eColorAttachmentOptimal, false);
						++m_attachments_count;
					}
				}

				if (depth)
				{
					vk::ImageLayout layout              = vk::ImageLayout::eDepthStencilAttachmentOptimal;
					vk::Format format                   = surface_format_of(depth);
					m_descriptions[m_attachments_count] = create_desctiption(
					        format, layout, format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD32SfloatS8Uint);
					++m_attachments_count;
				}

				return *this;
			}

			VulkanRenderPassBuilder& create_attachment_references(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
			{
				for (int_t index = 0, attachment = 0; index < 4; ++index)
				{
					if (targets[index])
					{
						m_references[index] = vk::AttachmentReference(attachment++, vk::ImageLayout::eColorAttachmentOptimal);
					}
					else
					{
						// clang-format off
						m_references[index] = vk::AttachmentReference(VK_ATTACHMENT_UNUSED, vk::ImageLayout::eColorAttachmentOptimal);
						// clang-format on
					}
				}

				if (depth)
				{
					vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
					m_references[4]        = vk::AttachmentReference(m_attachments_count - 1, layout);
				}
				return *this;
			}

			vk::RenderPass build(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
			{
				create_attachment_descriptions(targets, depth);
				create_attachment_references(targets, depth);
				return build();
			}

			vk::RenderPass build()
			{
				info_log("Vulkan", "New Render Pass");

				bool has_depth_attachment = m_references[4].layout != vk::ImageLayout::eUndefined;

				m_subpass = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 4, m_references, nullptr,
				                                   has_depth_attachment ? m_references + 4 : nullptr);

				vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(), m_attachments_count, m_descriptions, 1, &m_subpass);
				return API->m_device.createRenderPass(info);
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

	void VulkanRenderPass::Key::init(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		m_attachments[0] = surface_format_of(targets[0]);
		m_attachments[1] = surface_format_of(targets[1]);
		m_attachments[2] = surface_format_of(targets[2]);
		m_attachments[3] = surface_format_of(targets[3]);
		m_attachments[4] = surface_format_of(depth);

		m_attachments[5] = vk::Format::eUndefined;
	}

	bool VulkanRenderPass::Key::operator<(const Key& key) const
	{
		return std::memcmp(m_attachments, key.m_attachments, sizeof(m_attachments)) < 0;
	}

	VulkanRenderPass* VulkanRenderPass::find_or_create(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		Key key;
		key.init(targets, depth);

		VulkanRenderPass*& pass = m_render_passes[key];

		if (pass != nullptr)
			return pass;


		VulkanRenderPassBuilder builder;

		if (auto vk_pass = builder.build(targets, depth))
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

			builder.m_descriptions[0] =
			        VulkanRenderPassBuilder::create_desctiption(format, vk::ImageLayout::eColorAttachmentOptimal, false);
			builder.m_attachments_count = 1;

			builder.m_references[0] = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
			builder.m_references[1] = vk::AttachmentReference(VK_ATTACHMENT_UNUSED, vk::ImageLayout::eColorAttachmentOptimal);
			builder.m_references[2] = vk::AttachmentReference(VK_ATTACHMENT_UNUSED, vk::ImageLayout::eColorAttachmentOptimal);
			builder.m_references[3] = vk::AttachmentReference(VK_ATTACHMENT_UNUSED, vk::ImageLayout::eColorAttachmentOptimal);

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

	VulkanRenderPass::VulkanRenderPass(vk::RenderPass rp) : m_render_pass(rp) {}

	VulkanRenderPass::~VulkanRenderPass()
	{
		DESTROY_CALL(destroyRenderPass, m_render_pass);
	}
}// namespace Engine
