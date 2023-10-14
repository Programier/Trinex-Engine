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

    Texture2D::Texture2D()
    {
        _M_type = TextureType::Texture2D;
    }

    Texture2D& Texture2D::update(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap, const byte* data)
    {
        if (_M_rhi_texture)
        {
            _M_rhi_texture->update_texture_2D(size, offset, mipmap, data);
        }
        return *this;
    }

    Texture2D& Texture2D::read_data(Buffer& data, MipMapLevel level)
    {
        throw EngineException("Unimplemented method");
        return *this;
    }

    Texture2D& Texture2D::load(const Image& image)
    {
        destroy();
        resources(true);

        static ColorFormat _M_formats[5] = {ColorFormat::R8G8B8A8Sint, ColorFormat::R8Sint, ColorFormat::R8Sint,
                                            ColorFormat::R8G8B8Sint, ColorFormat::R8G8B8A8Sint};

        format       = _M_formats[static_cast<int>(image.channels())];
        mipmap_count = 1;

        _M_resources->images.clear();
        _M_resources->images.resize(1);

        if (image.empty())
        {
            Buffer tmp   = {100, 100, 100, 255};
            mipmap_count = 1;
            size         = {1, 1};
            _M_resources->images[0].create(size, 4, tmp);

            create();
            update(size, {0, 0}, 0, tmp.data());
        }
        else
        {
            info_log("Image data", "%zu, {%f, %f}\n", image.vector().size(), image.size().x, image.size().y);
            mipmap_count =
                    static_cast<MipMapLevel>(std::floor(std::log2(std::max(
                            static_cast<MipMapLevel>(image.width()), static_cast<MipMapLevel>(image.height()))))) +
                    1;

            mipmap_count = 8;
            size         = image.size();
            create();
            update(image.size(), {0, 0}, 0, image.vector().data());
            generate_mipmap();
        }


        if (engine_config.delete_resources_after_load)
            delete_resources();
        return *this;
    }

    Texture2D& Texture2D::load(const String& path)
    {
        destroy();
        info_log("Texture2D", "Loading Texture '%s'\n", path.c_str());
        Image image(path, true);
        return load(image);
    }

    bool Texture2D::load()
    {
        if (!_M_resources || _M_resources->images.size() == 0)
        {
            info_log("Texture2D", "Cannot find resources for texture '%s'", full_name().c_str());
            return false;
        }

        create();
        update(resource_image().size(), {0, 0}, 0, resource_image().vector().data());
        generate_mipmap();

        if (engine_config.delete_resources_after_load)
            delete_resources();
        return true;
    }

    Image& Texture2D::resource_image(bool create)
    {
        if (_M_resources == nullptr || _M_resources->images.size() == 0)
        {
            if (create)
            {
                resources(true);
                _M_resources->images.resize(1);
            }
            else
            {
                throw EngineException("Resources is null!");
            }
        }

        return _M_resources->images[0];
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

        if (archive->is_reading())
        {
            if (engine_instance->api() == EngineAPI::NoAPI || engine_config.load_textures_to_gpu == false)
                return true;
            return load();
        }

        return static_cast<bool>(*archive);
    }

}// namespace Engine
