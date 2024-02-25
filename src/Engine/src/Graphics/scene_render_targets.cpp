#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
#define TRINEX_WITH_STENCIL_BUFFER 0

    implement_engine_class_default_init(EngineRenderTarget);

    static void make_textures_non_editable(Vector<RenderTarget::Frame*>& frames)
    {
        for (auto& frame : frames)
        {
            if (frame->depth_stencil_attachment)
            {
                frame->depth_stencil_attachment->flags(Object::IsEditable, false);
            }

            for (auto& texture : frame->color_attachments)
            {
                texture->flags(Object::IsSerializable, false);
            }
        }
    }

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

            make_textures_non_editable(m_frames);
        }


        for (RenderTarget::Frame* frame : m_frames)
        {
            if (m_enable_color_initialize)
            {
                for (Texture2D* texture : frame->color_attachments)
                {
                    texture->size = size;
                    texture->init_resource();
                }
            }

            if (frame->depth_stencil_attachment && m_enable_depth_stencil_initialize)
            {
                frame->depth_stencil_attachment->size = size;
                frame->depth_stencil_attachment->init_resource();
            }
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
    static constexpr inline size_t data_buffer_index = 4;

    static constexpr inline size_t gbuffer_color_attachments = 5;

    static ColorFormat base_color_format()
    {
        ColorFormat format = engine_instance->rhi()->base_color_format();
        trinex_always_check(format != ColorFormat::Undefined, "Color format can't be undefined!");
        return format;
    }

    static ColorFormat position_format()
    {
        ColorFormat format = engine_instance->rhi()->position_format();
        trinex_always_check(format != ColorFormat::Undefined, "Position format can't be undefined!");
        return format;
    }

    static ColorFormat normal_format()
    {
        ColorFormat format = engine_instance->rhi()->normal_format();
        trinex_always_check(format != ColorFormat::Undefined, "Normal format can't be undefined!");
        return format;
    }

    static ColorFormat emissive_format()
    {
        ColorFormat format = engine_instance->rhi()->emissive_format();
        trinex_always_check(format != ColorFormat::Undefined, "Specular format can't be undefined!");
        return format;
    }

    static ColorFormat data_buffer_format()
    {
        ColorFormat format = engine_instance->rhi()->data_buffer_format();
        trinex_always_check(format != ColorFormat::Undefined, "Specular format can't be undefined!");
        return format;
    }

    static ColorFormat depth_format()
    {
#if TRINEX_WITH_STENCIL_BUFFER
        ColorFormat format = engine_instance->rhi()->depth_stencil_format();
#else

        ColorFormat format = engine_instance->rhi()->depth_format();
#endif
        trinex_always_check(format != ColorFormat::Undefined, "Color format can't be undefined!");
        return format;
    }

    struct AttachmentTextureInfo {
        ColorFormat (*required_format)() = nullptr;
        const char* name                 = nullptr;
    };


    static AttachmentTextureInfo attachment_texture_info[gbuffer_color_attachments] = {
            {base_color_format, "Base Color"}, {position_format, "Position"},        {normal_format, "Normal"},
            {emissive_format, "Emissive"},     {data_buffer_format, "Data Texture"},

    };


    implement_class(GBuffer, Engine, 0);
    implement_default_initialize_class(GBuffer);

    class GBufferRenderPass : public EngineResource<RenderPass>
    {
    public:
        GBufferRenderPass()
        {
            has_depth_stancil                      = true;
            depth_stencil_attachment.clear_on_bind = true;
            depth_stencil_attachment.format        = depth_format();

            color_attachments.resize(gbuffer_color_attachments);

            for (size_t i = 0; i < gbuffer_color_attachments; i++)
            {
                color_attachments[i].clear_on_bind = true;
                color_attachments[i].format        = attachment_texture_info[i].required_format();
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

        for (auto& ell : pass->color_attachments)
        {
            ell.clear_on_bind = false;
        }

        pass->depth_stencil_attachment.clear_on_bind = false;

        pass->init_resource(true);

        return pass;
    }

    RenderPass* RenderPass::load_clear_gbuffer_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<EngineResource<GBufferRenderPass>>();
        pass->init_resource(true);
        return pass;
    }

    Texture2D* GBuffer::Frame::base_color() const
    {
        return color_attachments[base_color_index].ptr();
    }

    Texture2D* GBuffer::Frame::position() const
    {
        return color_attachments[position_index].ptr();
    }

    Texture2D* GBuffer::Frame::normal() const
    {
        return color_attachments[normal_index].ptr();
    }

    Texture2D* GBuffer::Frame::emissive() const
    {
        return color_attachments[emissive_index].ptr();
    }

    Texture2D* GBuffer::Frame::data_buffer() const
    {
        return color_attachments[data_buffer_index];
    }

    Texture2D* GBuffer::Frame::depth() const
    {
        return depth_stencil_attachment.ptr();
    }


    GBuffer::GBuffer()
    {
        info_log("GBuffer", "Creating GBuffer");

        render_pass = RenderPass::load_render_pass(RenderPassType::GBuffer);

        for (size_t i = 0; i < gbuffer_color_attachments; i++)
        {
            color_clear.push_back(ColorClearValue(0.0f, 0.0f, 0.0f, 1.0f));
        }

        depth_stencil_clear.depth   = 1.f;
        depth_stencil_clear.stencil = 0.0f;

        // Initialize render target frames
        size_t frames_count = engine_instance->rhi()->render_target_buffer_count();
        Index frame_index   = 0;

        while (frame_index < frames_count)
        {
            push_frame(new GBuffer::Frame());
            frame_index++;
        }

        frame_index = 0;

        for (RenderTarget::Frame* frame : m_frames)
        {
            frame->depth_stencil_attachment = Object::new_non_serializable_instance_named<EngineResource<Texture2D>>(
                    Strings::format("Engine::GBuffer::Depth {}", frame_index));
            frame->depth_stencil_attachment->format = render_pass->depth_stencil_attachment.format;
            frame->depth_stencil_attachment->setup_render_target_texture();


            frame->color_attachments.resize(gbuffer_color_attachments);
            for (size_t i = 0; i < gbuffer_color_attachments; i++)
            {
                auto& info         = attachment_texture_info[i];
                Texture2D* texture = Object::new_non_serializable_instance_named<EngineResource<Texture2D>>(
                        Strings::format("Engine::GBuffer::{} {}", info.name, frame_index));

                texture->format = render_pass->color_attachments[i].format;
                texture->setup_render_target_texture();
                frame->color_attachments[i] = texture;
            }

            frame_index++;
        }

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    GBuffer::~GBuffer()
    {
        info_log("GBuffer", "Destroy GBuffer");
    }

    GBuffer::Frame* GBuffer::current_frame() const
    {
        return reinterpret_cast<Frame*>(Super::current_frame());
    }

    GBuffer::Frame* GBuffer::frame(byte index) const
    {
        return reinterpret_cast<Frame*>(Super::frame(index));
    }


    implement_engine_class_default_init(GBufferBaseColorOutput);

    Texture2D* GBufferBaseColorOutput::Frame::texture() const
    {
        return color_attachments[0].ptr();
    }

    GBufferBaseColorOutput::GBufferBaseColorOutput()
    {
        info_log("GBufferBaseColorOutput", "Creating GBufferBaseColorOutput");

        m_enable_color_initialize         = false;
        m_enable_depth_stencil_initialize = false;

        render_pass = RenderPass::load_render_pass(RenderPassType::OneAttachentOutput);
        color_clear.push_back(ColorClearValue(0.0f, 0.0f, 0.0f, 1.0f));

        // Initialize render target frames
        size_t frames_count = engine_instance->rhi()->render_target_buffer_count();
        Index frame_index   = 0;

        while (frame_index < frames_count)
        {
            push_frame(new GBufferBaseColorOutput::Frame());
            frame_index++;
        }

        frame_index = 0;

        for (RenderTarget::Frame* frame : m_frames)
        {
            frame->color_attachments.resize(1);
            frame->color_attachments[0]     = GBuffer::instance()->frame(frame_index)->base_color();
            frame->depth_stencil_attachment = GBuffer::instance()->frame(frame_index)->depth_stencil_attachment;

            frame_index++;
        }

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    GBufferBaseColorOutput::~GBufferBaseColorOutput()
    {
        debug_log("GBufferBaseColorOutput", "Destroy GBufferBaseColorOutput");
    }

    GBufferBaseColorOutput::Frame* GBufferBaseColorOutput::current_frame() const
    {
        return reinterpret_cast<Frame*>(Super::current_frame());
    }

    GBufferBaseColorOutput::Frame* GBufferBaseColorOutput::frame(byte index) const
    {
        return reinterpret_cast<Frame*>(Super::frame(index));
    }

    implement_engine_class_default_init(SceneColorOutput);

    Texture2D* SceneColorOutput::Frame::texture() const
    {
        return color_attachments[0].ptr();
    }

    class OneAttachmentOutputRenderPass : public EngineResource<RenderPass>
    {
    public:
        OneAttachmentOutputRenderPass()
        {
            has_depth_stancil = true;

            // Initialize color attachments
            color_attachments.resize(1);
            color_attachments[0].clear_on_bind = true;
            color_attachments[0].format        = base_color_format();

            depth_stencil_attachment.format        = depth_format();
            depth_stencil_attachment.clear_on_bind = false;
        }

        RenderPassType type() const override
        {
            return RenderPassType::OneAttachentOutput;
        }
    };

    RenderPass* RenderPass::load_one_attachement_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<OneAttachmentOutputRenderPass>();

        for (auto& ell : pass->color_attachments)
        {
            ell.clear_on_bind = false;
        }

        pass->init_resource(true);
        return pass;
    }

    RenderPass* RenderPass::load_clear_one_attachement_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<OneAttachmentOutputRenderPass>();
        pass->init_resource(true);
        return pass;
    }

    SceneColorOutput::SceneColorOutput()
    {
        info_log("SceneColorOutput", "Creating SceneColorOutput");

        m_enable_depth_stencil_initialize = false;

        render_pass = RenderPass::load_render_pass(RenderPassType::OneAttachentOutput);
        color_clear.push_back(ColorClearValue(0.0f, 0.0f, 0.0f, 1.0f));


        // Initialize render target frames
        size_t frames_count = engine_instance->rhi()->render_target_buffer_count();
        Index frame_index   = 0;

        while (frame_index < frames_count)
        {
            push_frame(new SceneColorOutput::Frame());
            frame_index++;
        }

        frame_index = 0;

        for (RenderTarget::Frame* frame : m_frames)
        {
            frame->color_attachments.resize(1);

            Texture2D* texture = Object::new_non_serializable_instance_named<EngineResource<Texture2D>>(
                    Strings::format("Engine::SceneColorOutput::Color {}", frame_index));

            texture->format = render_pass->color_attachments[0].format;
            texture->setup_render_target_texture();
            frame->color_attachments[0] = texture;

            frame->depth_stencil_attachment = GBuffer::instance()->frame(frame_index)->depth_stencil_attachment;
            frame_index++;
        }

        init(WindowManager::instance()->calculate_gbuffer_size());
    }

    SceneColorOutput::~SceneColorOutput()
    {
        debug_log("SceneColorOutput", "Destroy SceneColorOutput");
    }

    SceneColorOutput::Frame* SceneColorOutput::current_frame() const
    {
        return reinterpret_cast<Frame*>(Super::current_frame());
    }

    SceneColorOutput::Frame* SceneColorOutput::frame(byte index) const
    {
        return reinterpret_cast<Frame*>(Super::frame(index));
    }

    ENGINE_EXPORT void update_render_targets_size()
    {
        Size2D new_size = WindowManager::instance()->calculate_gbuffer_size();
        GBuffer::instance()->resize(new_size);
        SceneColorOutput::instance()->resize(new_size);
        GBufferBaseColorOutput::instance()->resize(new_size);
    }
}// namespace Engine
