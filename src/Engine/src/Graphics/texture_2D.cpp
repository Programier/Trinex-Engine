#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Image/image.hpp>

namespace Engine
{
    implement_class(Texture2D, "Engine");
    implement_default_initialize_class(Texture2D);

    Texture2D::Texture2D() = default;

    Texture2D& Texture2D::rhi_create()
    {
        const byte* data = image.empty() ? nullptr : image.data();
        return rhi_create(data);
    }

    Texture2D& Texture2D::rhi_create(const byte* data)
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_texture(this, data));
        return *this;
    }

    Texture2D& Texture2D::update(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data)
    {
        if (_M_rhi_object)
        {
            rhi_object<RHI_Texture>()->update_texture_2D(size, offset, mipmap, data);
        }
        return *this;
    }

    Texture2D& Texture2D::read_data(Buffer& data, MipMapLevel level)
    {
        throw EngineException("Unimplemented method");
        return *this;
    }


    Texture2D& Texture2D::read_image(Image& image, MipMapLevel level)
    {
        auto texture_mip_size = mip_size(level);
        image._M_width        = static_cast<int_t>(texture_mip_size.x);
        image._M_height       = static_cast<int_t>(texture_mip_size.y);
        read_data(image._M_data, level);
        image._M_channels = image._M_data.size() / static_cast<std::size_t>(image._M_width * image._M_height);
        return *this;
    }

    bool Texture2D::archive_process(Archive* archive)
    {
        if (!Texture::archive_process(archive))
        {
            return false;
        }

        (*archive) & image;
        return static_cast<bool>(*archive);
    }


    TextureType Texture2D::type() const
    {
        return TextureType::Texture2D;
    }

}// namespace Engine
