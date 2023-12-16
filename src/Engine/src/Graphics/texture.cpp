#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <Core/archive.hpp>

namespace Engine
{
    implement_class(Texture, "Engine", 0);
    implement_default_initialize_class(Texture);

    Texture::Texture() = default;

    const Texture& Texture::bind_combined(Sampler* sampler, BindLocation location) const
    {
        if (_M_rhi_object)
        {
            RHI_Sampler* rhi_sampler = reinterpret_cast<RenderResource*>(sampler)->rhi_object<RHI_Sampler>();
            if (rhi_sampler)
            {
                rhi_object<RHI_Texture>()->bind_combined(rhi_sampler, location);
            }
        }
        return *this;
    }

    Texture& Texture::generate_mipmap()
    {
        if (_M_rhi_object)
        {
            rhi_object<RHI_Texture>()->generate_mipmap();
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


    bool Texture::archive_process(Archive* archive)
    {
        if (!RenderResource::archive_process(archive))
            return false;


        (*archive) & size;
        (*archive) & base_mip_level;
        (*archive) & mipmap_count;
        (*archive) & format;
        (*archive) & swizzle;
        return static_cast<bool>(*archive);
    }

    Texture::~Texture()
    {}
}// namespace Engine
