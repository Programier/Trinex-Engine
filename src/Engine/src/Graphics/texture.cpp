#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <api.hpp>

namespace Engine
{
    implement_class(Texture, "Engine");
    implement_default_initialize_class(Texture);

    Texture::Texture()
    {}


    Texture& Texture::create()
    {
        if (_M_resources)
        {
            ApiObject::destroy();

            EngineInstance::instance()->api_interface()->create_texture(_M_ID, _M_resources->info, _M_type);
        }
        return *this;
    }

    const Texture& Texture::bind(TextureBindIndex num) const
    {
        EngineInstance::instance()->api_interface()->bind_texture(_M_ID, num);
        return *this;
    }

    MipMapLevel Texture::base_level() const
    {
        return EngineInstance::instance()->api_interface()->base_level_texture(_M_ID);
    }

    Texture& Texture::base_level(MipMapLevel level)
    {
        EngineInstance::instance()->api_interface()->base_level_texture(_M_ID, level);
        return *this;
    }

    CompareFunc Texture::compare_func() const
    {
        return EngineInstance::instance()->api_interface()->compare_func_texture(_M_ID);
    }

    Texture& Texture::compare_func(CompareFunc func)
    {
        EngineInstance::instance()->api_interface()->compare_func_texture(_M_ID, func);
        return *this;
    }

    Texture& Texture::compare_mode(CompareMode mode)
    {
        EngineInstance::instance()->api_interface()->compare_mode_texture(_M_ID, mode);
        return *this;
    }

    CompareMode Texture::compare_mode() const
    {
        return EngineInstance::instance()->api_interface()->compare_mode_texture(_M_ID);
    }

    const TextureFilter Texture::min_filter() const
    {
        return EngineInstance::instance()->api_interface()->min_filter_texture(_M_ID);
    }

    const TextureFilter Texture::mag_filter() const
    {
        return EngineInstance::instance()->api_interface()->mag_filter_texture(_M_ID);
    }

    Texture& Texture::min_filter(TextureFilter filter)
    {
        EngineInstance::instance()->api_interface()->min_filter_texture(_M_ID, filter);
        return *this;
    }

    Texture& Texture::mag_filter(TextureFilter filter)
    {
        EngineInstance::instance()->api_interface()->mag_filter_texture(_M_ID, filter);
        return *this;
    }

    Texture& Texture::min_lod_level(LodLevel value)
    {
        EngineInstance::instance()->api_interface()->min_lod_level_texture(_M_ID, value);
        return *this;
    }

    Texture& Texture::max_lod_level(LodLevel value)
    {
        EngineInstance::instance()->api_interface()->max_lod_level_texture(_M_ID, value);
        return *this;
    }

    LodLevel Texture::min_lod_level() const
    {
        return EngineInstance::instance()->api_interface()->min_lod_level_texture(_M_ID);
    }

    LodLevel Texture::max_lod_level() const
    {
        return EngineInstance::instance()->api_interface()->max_lod_level_texture(_M_ID);
    }

    MipMapLevel Texture::max_mipmap_level() const
    {
        return EngineInstance::instance()->api_interface()->max_mipmap_level_texture(_M_ID);
    }

    SwizzleRGBA Texture::swizzle() const
    {
        return EngineInstance::instance()->api_interface()->swizzle_texture(_M_ID);
    }

    Texture& Texture::swizzle(const SwizzleRGBA& value)
    {
        EngineInstance::instance()->api_interface()->swizzle_texture(_M_ID, value);
        return *this;
    }

    Texture& Texture::wrap_s(const WrapValue& wrap)
    {
        EngineInstance::instance()->api_interface()->wrap_s_texture(_M_ID, wrap);
        return *this;
    }

    Texture& Texture::wrap_t(const WrapValue& wrap)
    {
        EngineInstance::instance()->api_interface()->wrap_t_texture(_M_ID, wrap);
        return *this;
    }

    Texture& Texture::wrap_r(const WrapValue& wrap)
    {
        EngineInstance::instance()->api_interface()->wrap_r_texture(_M_ID, wrap);
        return *this;
    }

    WrapValue Texture::wrap_s() const
    {
        return EngineInstance::instance()->api_interface()->wrap_s_texture(_M_ID);
    }

    WrapValue Texture::wrap_t() const
    {
        return EngineInstance::instance()->api_interface()->wrap_t_texture(_M_ID);
    }

    WrapValue Texture::wrap_r() const
    {
        return EngineInstance::instance()->api_interface()->wrap_r_texture(_M_ID);
    }

    Texture& Texture::generate_mipmap()
    {
        EngineInstance::instance()->api_interface()->generate_texture_mipmap(_M_ID);
        return *this;
    }

    Size2D Texture::size(MipMapLevel level) const
    {
        Size2D _M_size;
        EngineInstance::instance()->api_interface()->texture_size(_M_ID, _M_size, level);
        return _M_size;
    }

    Identifier Texture::internal_id() const
    {
        return EngineInstance::instance()->api_interface()->imgui_texture_id(_M_ID);
    }

    Texture& Texture::anisotropic_filtering(float value)
    {
        EngineInstance::instance()->api_interface()->anisotropic_filtering_texture(_M_ID, value);
        return *this;
    }

    float Texture::anisotropic_filtering()
    {
        return EngineInstance::instance()->api_interface()->anisotropic_filtering_texture(_M_ID);
    }

    float Texture::max_anisotropic_filtering()
    {
        return EngineInstance::instance()->api_interface()->max_anisotropic_filtering();
    }

    SamplerMipmapMode Texture::sample_mipmap_mode()
    {
        return EngineInstance::instance()->api_interface()->sample_mipmap_mode_texture(_M_ID);
    }

    Texture& Texture::sample_mipmap_mode(SamplerMipmapMode mode)
    {
        EngineInstance::instance()->api_interface()->sample_mipmap_mode_texture(_M_ID, mode);
        return *this;
    }

    LodBias Texture::lod_bias()
    {
        return EngineInstance::instance()->api_interface()->lod_bias_texture(_M_ID);
    }

    Texture& Texture::lod_bias(LodBias bias)
    {
        EngineInstance::instance()->api_interface()->lod_bias_texture(_M_ID, bias);
        return *this;
    }

    bool TextureResources::archive_process(Archive* archive_ptr)
    {
        if (!SerializableObject::archive_process(archive_ptr))
            return false;

        Archive& archive = *archive_ptr;

        if (!(archive & info))
        {
            error_log("TextureResources", "Failed to process texture resources!");
            return false;
        }

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

        return _M_resources->archive_process(archive);
    }

    TextureCreateInfo& Texture::info(bool create)
    {
        if (_M_resources == nullptr)
        {
            if (create)
            {
                return resources(true)->info;
            }
            throw EngineException("Texture: Resources is nullptr!");
        }
        return _M_resources->info;
    }

    const TextureCreateInfo& Texture::info() const
    {
        if (_M_resources == nullptr)
        {
            throw EngineException("Texture: Resources is nullptr!");
        }

        return _M_resources->info;
    }

    Texture::~Texture()
    {
        delete_resources();
    }
}// namespace Engine
