#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/render_target_texture.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
#define TRINEX_WITH_STENCIL_BUFFER 0

    implement_engine_class_default_init(EngineRenderTarget);


    void EngineRenderTarget::init(const Size2D& new_size, bool is_reinit)
    {
        if (size.x >= new_size.x && size.y >= new_size.y)
            return;

        info_log("EngineRenderTarget", "{%f, %f} -> {%f, %f}", size.x, size.y, new_size.x, new_size.y);

        // Initilize/reinitialzie textures
        if (is_reinit)
        {
            Size2D scale_factor = new_size / size;
            m_viewport.size *= scale_factor;
            m_viewport.pos *= scale_factor;

            m_scissor.size *= scale_factor;
            m_scissor.pos *= scale_factor;

            size = new_size;
        }
        else
        {
            size                 = new_size;
            m_viewport.size      = size;
            m_viewport.pos       = {0, 0};
            m_viewport.min_depth = 0.0f;
            m_viewport.max_depth = 1.0f;

            m_scissor.pos  = {0, 0};
            m_scissor.size = size;
        }

        if (m_enable_color_initialize)
        {
            for (auto& attachment : color_attachments)
            {
                attachment->size = size;
                attachment->init_resource();
            }
        }

        if (depth_stencil_attachment && m_enable_depth_stencil_initialize)
        {
            depth_stencil_attachment->size = size;
            depth_stencil_attachment->init_resource();
        }

        init_resource();
    }

    EngineRenderTarget& EngineRenderTarget::resize(const Size2D& new_size)
    {
        init(new_size, true);
        return *this;
    }

    bool EngineRenderTarget::is_engine_resource() const
    {
        return true;
    }


    static constexpr inline size_t base_color_index  = 0;
    static constexpr inline size_t position_index    = 1;
    static constexpr inline size_t normal_index      = 2;
    static constexpr inline size_t emissive_index    = 3;
    static constexpr inline size_t msra_buffer_index = 4;

    static constexpr inline size_t gbuffer_color_attachments = 5;

    static ColorFormat base_color_format()
    {
        return ColorFormat::R8G8B8A8;
    }

    static ColorFormat position_format()
    {
        return ColorFormat::FloatRGBA;
    }

    static ColorFormat normal_format()
    {
        return ColorFormat::FloatRGBA;
    }

    static ColorFormat emissive_format()
    {
        return ColorFormat::R8G8B8A8;
    }

    static ColorFormat msra_buffer_format()
    {
        return ColorFormat::R8G8B8A8;
    }

    static ColorFormat depth_format()
    {
        return ColorFormat::DepthStencil;
    }

    struct AttachmentTextureInfo {
        ColorFormat (*required_format)() = nullptr;
        const char* name                 = nullptr;
    };


    static AttachmentTextureInfo attachment_texture_info[gbuffer_color_attachments] = {
            {base_color_format, "Base Color"}, {position_format, "Position"},        {normal_format, "Normal"},
            {emissive_format, "Emissive"},     {msra_buffer_format, "MSRA Texture"},

    };


    implement_class(GBuffer, Engine, 0);
    implement_default_initialize_class(GBuffer);

    class GBufferRenderPass : public EngineResource<RenderPass>
    {
    public:
        GBufferRenderPass()
        {
            depth_stencil_attachment = depth_format();
            color_attachments.resize(gbuffer_color_attachments);

            for (size_t i = 0; i < gbuffer_color_attachments; i++)
            {
                color_attachments[i] = attachment_texture_info[i].required_format();
            }
        }

        RenderPassType type() const override
        {
            return RenderPassType::GBuffer;
        }
    };


    RenderPass* RenderPass::load_gbuffer_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<EngineResource<GBufferRenderPass>>();
        pass->init_resource(true);
        return pass;
    }

    GBuffer::GBuffer()
    {
        info_log("GBuffer", "Creating GBuffer");

        render_pass = RenderPass::load_render_pass(RenderPassType::GBuffer);
        color_attachments.resize(gbuffer_color_attachments);

        depth_stencil_attachment =
                Object::new_non_serializable_instance_named<EngineResource<RenderTargetTexture>>("Engine::GBuffer::Depth");
        depth_stencil_attachment->format = render_pass->depth_stencil_attachment;

        for (size_t i = 0; i < gbuffer_color_attachments; i++)
        {
            auto& info                   = attachment_texture_info[i];
            RenderTargetTexture* texture = Object::new_non_serializable_instance_named<EngineResource<RenderTargetTexture>>(
                    Strings::format("Engine::GBuffer::{}", info.name));
            texture->format      = render_pass->color_attachments[i];
            color_attachments[i] = texture;
        }

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    RenderTargetTexture* GBuffer::base_color() const
    {
        return color_attachments[base_color_index].ptr();
    }

    RenderTargetTexture* GBuffer::position() const
    {
        return color_attachments[position_index].ptr();
    }

    RenderTargetTexture* GBuffer::normal() const
    {
        return color_attachments[normal_index].ptr();
    }

    RenderTargetTexture* GBuffer::emissive() const
    {
        return color_attachments[emissive_index].ptr();
    }

    RenderTargetTexture* GBuffer::msra_buffer() const
    {
        return color_attachments[msra_buffer_index].ptr();
    }

    RenderTargetTexture* GBuffer::depth() const
    {
        return depth_stencil_attachment.ptr();
    }

    GBuffer::~GBuffer()
    {
        info_log("GBuffer", "Destroy GBuffer");
    }

    implement_engine_class_default_init(GBufferBaseColorOutput);


    GBufferBaseColorOutput::GBufferBaseColorOutput()
    {
        info_log("GBufferBaseColorOutput", "Creating GBufferBaseColorOutput");

        m_enable_color_initialize         = false;
        m_enable_depth_stencil_initialize = false;

        render_pass = RenderPass::load_render_pass(RenderPassType::SceneColor);
        color_attachments.resize(1);
        color_attachments[0] = GBuffer::instance()->base_color();

        depth_stencil_attachment = GBuffer::instance()->depth();

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    RenderTargetTexture* GBufferBaseColorOutput::texture() const
    {
        return color_attachments[0].ptr();
    }

    GBufferBaseColorOutput::~GBufferBaseColorOutput()
    {
        debug_log("GBufferBaseColorOutput", "Destroy GBufferBaseColorOutput");
    }

    implement_engine_class_default_init(SceneColorOutput);

    class SceneColorRenderPass : public EngineResource<RenderPass>
    {
    public:
        SceneColorRenderPass()
        {
            // Initialize color attachments
            color_attachments.resize(1);
            color_attachments[0] = base_color_format();

            depth_stencil_attachment = depth_format();
        }

        RenderPassType type() const override
        {
            return RenderPassType::SceneColor;
        }
    };

    RenderPass* RenderPass::load_scene_color_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<SceneColorRenderPass>();
        pass->init_resource(true);
        return pass;
    }

    SceneColorOutput::SceneColorOutput()
    {
        info_log("SceneColorOutput", "Creating SceneColorOutput");

        m_enable_color_initialize         = true;
        m_enable_depth_stencil_initialize = false;

        render_pass = RenderPass::load_render_pass(RenderPassType::SceneColor);
        color_attachments.resize(1);

        RenderTargetTexture* texture = Object::new_non_serializable_instance_named<EngineResource<RenderTargetTexture>>(
                "Engine::SceneColorOutput::Color");
        texture->format = render_pass->color_attachments[0];

        color_attachments[0]     = texture;
        depth_stencil_attachment = GBuffer::instance()->depth();

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    RenderTargetTexture* SceneColorOutput::texture() const
    {
        return color_attachments[0].ptr();
    }

    SceneColorOutput::~SceneColorOutput()
    {
        debug_log("SceneColorOutput", "Destroy SceneColorOutput");
    }

    ENGINE_EXPORT void update_render_targets_size()
    {
        Size2D new_size = WindowManager::instance()->calculate_gbuffer_size();
        GBuffer::instance()->resize(new_size);
        SceneColorOutput::instance()->resize(new_size);
        GBufferBaseColorOutput::instance()->resize(new_size);
    }
}// namespace Engine
