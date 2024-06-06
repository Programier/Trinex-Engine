#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Image/image.hpp>

namespace Engine
{
    implement_engine_class(Texture2D, 0)
    {
        Class* self_class = static_class_instance();
        self_class->add_properties(new PathProperty("Path", "Path to texture", &Texture2D::path));
    }

    Texture2D::Texture2D() = default;

    Texture2D& Texture2D::rhi_create()
    {
        const byte* data = image.empty() ? nullptr : image.data();
        return rhi_create(data, image.buffer().size());
    }

    Texture2D& Texture2D::rhi_create(const byte* data, size_t data_size)
    {
        if (size.x >= 1.0f && size.y >= 1.f)
            m_rhi_object.reset(rhi->create_texture(this, data, data_size));
        return *this;
    }

    Texture2D& Texture2D::update(const Size2D& size, const Offset2D& offset, const byte* data, size_t data_size,
                                 MipMapLevel mipmap)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_Texture>()->update_texture_2D(size, offset, mipmap, data, data_size);
        }
        return *this;
    }

    bool Texture2D::archive_process(Archive& archive)
    {
        if (!Texture::archive_process(archive))
        {
            return false;
        }

        archive & image;
        return static_cast<bool>(archive);
    }

    TextureType Texture2D::type() const
    {
        return TextureType::Texture2D;
    }

    Texture2D& Texture2D::apply_changes()
    {
        Super::apply_changes();
        image.load(path);
        //image.add_alpha_channel();
        format = image.format();
        size   = image.size();
        postload();
        return *this;
    }
}// namespace Engine
