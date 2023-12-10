#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{

    static constexpr inline size_t albedo_index   = 0;
    static constexpr inline size_t position_index = 1;
    static constexpr inline size_t normal_index   = 2;
    static constexpr inline size_t specular_index = 3;

    static constexpr inline size_t gbuffer_color_attachments = 4;


    implement_class(GBuffer, "Engine");
    implement_default_initialize_class(GBuffer);

    static Vector<ColorFormat> required_albedo_formats()
    {
        return {ColorFormat::R8G8B8A8Unorm, ColorFormat::R8G8B8A8Snorm, ColorFormat::R8G8B8A8Uint};
    }

    static Vector<ColorFormat> required_position_formats()
    {
        return {ColorFormat::R16G16B16A16Sfloat, ColorFormat::R16G16B16A16Sfloat, ColorFormat::R32G32B32Sfloat,
                ColorFormat::R32G32B32A32Sfloat};
    }

    static Vector<ColorFormat> required_normal_formats()
    {
        return {ColorFormat::R16G16B16A16Sfloat, ColorFormat::R16G16B16A16Sfloat, ColorFormat::R32G32B32Sfloat,
                ColorFormat::R32G32B32A32Sfloat};
    }

    static Vector<ColorFormat> required_specular_formats()
    {
        return {ColorFormat::R8G8B8A8Unorm, ColorFormat::R8G8B8A8Snorm, ColorFormat::R8G8B8A8Uint};
    }

    static Vector<ColorFormat> required_depth_formats()
    {
        return {ColorFormat::D32SfloatS8Uint, ColorFormat::D24UnormS8Uint, ColorFormat::D16UnormS8Uint};
    }

    static ColorFormatFeatures color_format_requirements()
    {
        ColorFormatFeatures features;
        features.is_supported             = true;
        features.support_color_attachment = true;
        return features;
    }

    static ColorFormatFeatures depth_format_кequirements()
    {
        ColorFormatFeatures features;
        features.is_supported          = true;
        features.support_depth_stencil = true;
        return features;
    }


    static ColorFormat find_color_format(const Vector<ColorFormat>& formats, ColorFormatFeatures features,
                                         const char* type)
    {
        for (const ColorFormat& format : formats)
        {
            if (ColorFormatInfo::info_of(format).features().contains(features))
            {
                return format;
            }
        }

        throw EngineException(Strings::format("Cannot find format for {} gbuffer texture", type));
    }


    struct AttachmentTextureInfo {
        Vector<ColorFormat> (*required_formats)() = nullptr;
        const char* name                          = nullptr;
    };

    static AttachmentTextureInfo attachment_texture_info[gbuffer_color_attachments] = {
            {required_albedo_formats, "Albedo"},
            {required_position_formats, "Position"},
            {required_normal_formats, "Normal"},
            {required_specular_formats, "Specular"},
    };


    GBuffer::GBuffer()
    {
        info_log("GBuffer", "Creating GBuffer");

        // Creating render pass
        render_pass = Object::new_instance_named<RenderPass>("Engine::GBuffer::RenderPass");

        render_pass->has_depth_stancil                      = true;
        render_pass->depth_stencil_attachment.clear_on_bind = true;
        render_pass->depth_stencil_attachment.format =
                find_color_format(required_depth_formats(), depth_format_кequirements(), "depth");

        // Initialize color attachments
        render_pass->color_attachments.resize(gbuffer_color_attachments);

        for (size_t i = 0; i < gbuffer_color_attachments; i++)
        {
            render_pass->color_attachments[i].clear_on_bind = true;
            render_pass->color_attachments[i].format =
                    find_color_format(attachment_texture_info[i].required_formats(), color_format_requirements(),
                                      attachment_texture_info[i].name);

            color_clear.push_back(ColorClearValue(0.0f, 0.0f, 0.0f, 1.0f));
        }

        depth_stencil_clear.depth   = 1.f;
        depth_stencil_clear.stencil = 0.0f;

        render_pass->init_resource();

        // Initialize render target frames
        size_t frames_count = engine_instance->rhi()->render_target_buffer_count();
        Index frame_index   = 0;

        while (frame_index < frames_count)
        {
            push_frame(new GBuffer::Frame());
            frame_index++;
        }

        frame_index = 0;

        for (RenderTarget::Frame* frame : _M_frames)
        {
            frame->depth_stencil_attachment =
                    Object::new_instance_named<Texture2D>(Strings::format("Engine::GBuffer::Depth {}", frame_index));
            frame->depth_stencil_attachment->format = render_pass->depth_stencil_attachment.format;
            frame->depth_stencil_attachment->setup_render_target_texture();


            frame->color_attachments.resize(gbuffer_color_attachments);
            for (size_t i = 0; i < gbuffer_color_attachments; i++)
            {
                auto& info         = attachment_texture_info[i];
                Texture2D* texture = Object::new_instance_named<Texture2D>(
                        Strings::format("Engine::GBuffer::{} {}", info.name, frame_index));

                texture->format = render_pass->color_attachments[i].format;
                texture->setup_render_target_texture();
                frame->color_attachments[i] = texture;
            }

            frame_index++;
        }

        init();
    }

    GBuffer::~GBuffer()
    {
        info_log("GBuffer", "Destroy GBuffer");
    }

    void GBuffer::init(bool is_reinit)
    {
        // Initilize/reinitialzie textures
        Size2D new_size = WindowManager::instance()->calculate_gbuffer_size();
        if (is_reinit)
        {
            Size2D scale_factor = new_size / size;
            _M_viewport.size *= scale_factor;
            _M_viewport.pos *= scale_factor;

            _M_scissor.size *= scale_factor;
            _M_scissor.pos *= scale_factor;

            size = new_size;
        }
        else
        {
            size                  = new_size;
            _M_viewport.size      = size;
            _M_viewport.pos       = {0, 0};
            _M_viewport.min_depth = 0.0f;
            _M_viewport.max_depth = 1.0f;

            _M_scissor.pos  = {0, 0};
            _M_scissor.size = size;
        }


        for (RenderTarget::Frame* frame : _M_frames)
        {
            for (Texture2D* texture : frame->color_attachments)
            {
                texture->size = size;
                texture->init_resource();
            }

            frame->depth_stencil_attachment->size = size;
            frame->depth_stencil_attachment->init_resource();
        }

        init_resource();
    }


    struct InitGBuffer : public ExecutableObject {

        int_t execute() override
        {
            GBuffer::instance()->resize();
            return sizeof(InitGBuffer);
        }
    };

    GBuffer& GBuffer::resize()
    {
        init(true);
        return *this;
    }
}// namespace Engine
