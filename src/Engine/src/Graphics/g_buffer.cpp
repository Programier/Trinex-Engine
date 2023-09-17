#include <Core/engine_config.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/texture_2D.hpp>
#include <Window/monitor.hpp>

namespace Engine
{
    static GBuffer* _M_g_buffer = nullptr;


    static void create_texture(Pointer<Texture2D>& texture, const Size2D& size, ColorFormat format, const String& name)
    {
        texture = Object::new_instance<Texture2D>();
        texture->name(name);
        TextureCreateInfo& info = texture->resources(true)->info;

        info.size   = size;
        info.format = format;
        texture->create();

        texture->delete_resources();
    }

    static void init_buffer_data(GBufferData& data, const Size2D& size, int index)
    {
#define new_name(n) Strings::format(n " Texture {}", index)

        create_texture(data.albedo, size, ColorFormat::R8G8B8A8Unorm, new_name("Albedo"));
        create_texture(data.position, size, ColorFormat::R16G16B16A16Sfloat, new_name("Position"));
        create_texture(data.normal, size, ColorFormat::R16G16B16A16Sfloat, new_name("Normal"));
        create_texture(data.specular, size, ColorFormat::R8G8B8A8Unorm, new_name("Specular"));
        create_texture(data.depth, size, ColorFormat::D32SfloatS8Uint, new_name("Depth"));
#undef new_name
    }

    ENGINE_EXPORT void GBuffer::init_g_buffer()
    {
        _M_g_buffer = Object::new_instance<GBuffer>();

        FrameBufferCreateInfo info;

        info.size = {
                glm::clamp(Monitor::width(), engine_config.min_g_buffer_width, engine_config.max_g_buffer_width),
                glm::clamp(Monitor::height(), engine_config.min_g_buffer_height, engine_config.max_g_buffer_height),
        };

        info.buffers.resize(3);

        FrameBufferAttachmentClearData color_clear_data;
        color_clear_data.clear_on_bind     = 1;
        color_clear_data.clear_value.color = ColorClearValue(0.0, 0.0, 0.0, 1.0);

        FrameBufferAttachmentClearData depth_stencil_clear_data;
        depth_stencil_clear_data.clear_on_bind             = 1;
        depth_stencil_clear_data.clear_value.depth_stencil = DepthStencilClearValue({1.0, 0});

        for (int i = 0; i < 3; i++)
        {
            GBufferData& data = _M_g_buffer->_M_buffer_data[i];
            init_buffer_data(data, info.size, i);


            info.buffers[i].color_attachments.resize(4);

            info.buffers[i].color_attachments[0].texture_id = data.albedo->id();
            info.buffers[i].color_attachments[1].texture_id = data.position->id();
            info.buffers[i].color_attachments[2].texture_id = data.normal->id();
            info.buffers[i].color_attachments[3].texture_id = data.specular->id();

            FrameBufferAttachment depth_attachment;
            depth_attachment.texture_id              = data.depth->id();
            info.buffers[i].depth_stencil_attachment = depth_attachment;
        }

        for (int i = 0; i < 4; i++)
        {
            info.color_clear_data.push_back(color_clear_data);
        }

        info.depth_stencil_clear_data = depth_stencil_clear_data;
        _M_g_buffer->create(info);
    }


    GBuffer::GBuffer()
    {
        trinex_flag(TrinexObjectFlags::IsSerializable, false);
    }

    ENGINE_EXPORT GBuffer* GBuffer::instance()
    {
        return _M_g_buffer;
    }

    void GBuffer::swap_buffer()
    {
        index = (index + 1) % 3;
    }

    const GBufferData& GBuffer::buffer_data()
    {
        return _M_buffer_data[index];
    }

    const GBufferData& GBuffer::previous_buffer_data()
    {
        return _M_buffer_data[(index + 1) % 3];
    }

    GBuffer& GBuffer::bind()
    {
        BasicFrameBuffer::bind(index);
        return *this;
    }
}// namespace Engine
