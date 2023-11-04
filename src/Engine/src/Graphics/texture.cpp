#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
    implement_class(Texture, "Engine");
    implement_default_initialize_class(Texture);

    Texture::Texture() = default;

    const Texture& Texture::bind_combined(Sampler* sampler, BindLocation location) const
    {
        if (_M_rhi_texture)
        {
            RHI_Sampler* rhi_sampler = reinterpret_cast<ApiObject*>(sampler)->rhi_object<RHI_Sampler>();
            _M_rhi_texture->bind_combined(rhi_sampler, location);
        }
        return *this;
    }

    Texture& Texture::generate_mipmap()
    {
        if (_M_rhi_texture)
        {
            _M_rhi_texture->generate_mipmap();
        }
        return *this;
    }

    Texture& Texture::setup_render_target_texture()
    {
        _M_use_for_render_target = true;
        return *this;
    }

    bool Texture::is_render_target_texture() const
    {
        return _M_use_for_render_target;
    }

    Size2D Texture::mip_size(MipMapLevel level) const
    {
        Size2D current_size = size;
        for (MipMapLevel i = 0; i < level; i++)
        {
            current_size /= 2;
        }
        return current_size;
    }

    bool TextureResources::archive_process(Archive* archive_ptr)
    {
        if (!SerializableObject::archive_process(archive_ptr))
            return false;

        Archive& archive = *archive_ptr;

        int_t index = 0;
        int_t count = static_cast<int_t>(images.size());

        if (!(archive & count))
        {
            error_log("TextureResources", "Failed to process images count");
            return false;
        }

        if (archive.is_reading())
        {
            images.resize(count);
        }

        for (auto& image : images)
        {
            if (!image.archive_process(archive_ptr))
            {
                error_log("TextureResources", "Failed to serialize image[%d]", index);
                return false;
            }
            ++index;
        }

        return static_cast<bool>(archive);
    }

    bool Texture::archive_process(Archive* archive)
    {
        if (!ApiObject::archive_process(archive))
            return false;

        if (archive->is_reading())
        {
            resources(true);
        }

        if (_M_resources == nullptr)
        {
            info_log("Texture: Cannot process texture '%s'. Texture resources is nullptr", full_name().c_str());
            return false;
        }

        (*archive) & size;
        (*archive) & base_mip_level;
        (*archive) & mipmap_count;
        (*archive) & format;
        (*archive) & swizzle;
        return static_cast<bool>(*archive) && _M_resources->archive_process(archive);
    }

    Texture::~Texture()
    {
        delete_resources();
    }
}// namespace Engine
