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


    Texture& Texture::create(const byte* ptr)
    {

        ApiObject::destroy();
        _M_rhi_binding_object = EngineInstance::instance()->api_interface()->create_texture(info, _M_type, ptr);

        return *this;
    }

    const Texture& Texture::bind_combined(Sampler* sampler, BindingIndex binding, BindingIndex set) const
    {
        if (_M_rhi_texture)
        {
            RHI::RHI_Sampler* rhi_sampler = reinterpret_cast<ApiObject*>(sampler)->get_rhi_object<RHI::RHI_Sampler>();
            _M_rhi_texture->bind_combined(rhi_sampler, binding, set);
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

    Size2D Texture::size(MipMapLevel level) const
    {
        Size2D current_size = info.size;
        for (MipMapLevel i = 0; i < level; i++)
        {
            current_size /= 2;
        }
        return current_size;
    }

    Identifier Texture::internal_id() const
    {
        return EngineInstance::instance()->api_interface()->imgui_texture_id(_M_ID);
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

        (*archive) & info;
        return static_cast<bool>(*archive) && _M_resources->archive_process(archive);
    }

    Texture::~Texture()
    {
        delete_resources();
    }
}// namespace Engine
