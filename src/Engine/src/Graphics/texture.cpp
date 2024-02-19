#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/enum.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
    implement_class(Texture, Engine, Class::IsAsset);
    implement_initialize_class(Texture)
    {
        Class* self        = static_class_instance();
        Enum* swizzle_enum = Enum::find("Engine::Swizzle");
        self->add_properties(new Vec2Property("Size", "Size of texture", &This::size, Name::none, Property::IsConst),
                             new EnumProperty("Format", "Color format of texture", &This::format,
                                              Enum::find("Engine::ColorFormat"), Name::none, Property::IsConst),

                             new ByteProperty("Base mip level", "Base mip level of texture", &This::base_mip_level),
                             new ByteProperty("MipMap count", "MipMap Count of texture", &This::mipmap_count),
                             new EnumProperty("Swizze R", "Swizze R of texture", &This::swizzle_r, swizzle_enum),
                             new EnumProperty("Swizze G", "Swizze G of texture", &This::swizzle_g, swizzle_enum),
                             new EnumProperty("Swizze B", "Swizze B of texture", &This::swizzle_b, swizzle_enum),
                             new EnumProperty("Swizze A", "Swizze A of texture", &This::swizzle_a, swizzle_enum));
    }

    Texture::Texture() = default;

    const Texture& Texture::rhi_bind_combined(Sampler* sampler, BindLocation location) const
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


    bool Texture::archive_process(Archive& archive)
    {
        if (!RenderResource::archive_process(archive))
            return false;
        return static_cast<bool>(archive);
    }

    Texture::~Texture()
    {}
}// namespace Engine
