#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Window/window.hpp>

namespace Engine
{
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

    static ColorFormatFeatures color_format_кequirements()
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
                                         const String& type)
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

    GBuffer* GBuffer::_M_instance = nullptr;

    GBuffer::GBuffer()
    {
        info_log("GBuffer", "Creating GBuffer");

        _M_color_formats.push_back(find_color_format(required_albedo_formats(), color_format_кequirements(), "albedo"));
        _M_color_formats.push_back(
                find_color_format(required_position_formats(), color_format_кequirements(), "position"));
        _M_color_formats.push_back(find_color_format(required_normal_formats(), color_format_кequirements(), "normal"));
        _M_color_formats.push_back(
                find_color_format(required_specular_formats(), color_format_кequirements(), "specular"));
        _M_color_formats.push_back(find_color_format(required_depth_formats(), depth_format_кequirements(), "depth"));


        render_pass = Object::new_instance_named<RenderPass>("Engine::GBuffer::RenderPass");

        render_pass->has_depth_stancil                      = true;
        render_pass->depth_stencil_attachment.clear_on_bind = true;
        render_pass->depth_stencil_attachment.format        = _M_color_formats.back();
        render_pass->depth_stencil_attachment.mip_level     = 0;

        for (int i = 0; i < 4; i++)
        {
            RenderPass::Attachment attachment;
            attachment.clear_on_bind = true;
            attachment.mip_level     = 0;
            attachment.format        = _M_color_formats[i];
            render_pass->color_attachments.push_back(attachment);
        }

        render_pass->rhi_create();

        init();
    }

    GBuffer::~GBuffer()
    {
        info_log("GBuffer", "Destroy GBuffer");
    }


    void GBuffer::init()
    {
        size = Window::instance()->size();


        const char* names[]            = {"Albedo", "Position", "Normal", "Specular", "Depth"};
        Pointer<Texture2D>* textures[] = {&albedo, &position, &normal, &specular, &depth};

        for (int i = 0; i < 5; i++)
        {
            Texture2D* current_texture =
                    Object::new_instance_named<Texture2D>(Strings::format("Engine::GBuffer::{}", names[i]));
            (*(textures[i])) = current_texture;

            current_texture->size   = size;
            current_texture->format = _M_color_formats[i];
            current_texture->setup_render_target_texture();
            current_texture->rhi_create();
        }

        RenderTarget::Attachment attachment;

        for (int i = 0; i < 4; i++)
        {
            attachment.texture   = textures[i]->ptr();
            attachment.mip_level = 0;
            color_attachments.push_back(attachment);
            color_clear.push_back(ColorClearValue(0.0f, 0.0f, 0.0f, 1.0f));
        }

        depth_stencil_attachment.mip_level = 0;
        depth_stencil_attachment.texture   = depth.ptr();
        rhi_create();
    }

    GBuffer& GBuffer::resize()
    {
        return *this;
    }
}// namespace Engine
