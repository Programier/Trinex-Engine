#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
    implement_class(Texture, "Engine", Class::IsAsset);
    implement_initialize_class(Texture)
    {
        Class* self = static_class_instance();
        self->create_prop<Vec2Property>("Size", "Size of texture", STRUCT_OFFSET(This, size));
        self->create_prop<ByteProperty>("Base mip level", "Base mip level", STRUCT_OFFSET(This, base_mip_level));
        self->create_prop<ByteProperty>("MipMap count", "MipMap Count", STRUCT_OFFSET(This, mipmap_count));

        //        ColorFormat format         = ColorFormat::R8G8B8A8Unorm;
        //        SwizzleRGBA swizzle;
    }

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
