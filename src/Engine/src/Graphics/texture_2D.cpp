#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Graphics/texture_2D.hpp>
#include <Image/image.hpp>
#include <api.hpp>

namespace Engine
{
    declare_instance_info_cpp(Texture2D);
    constructor_cpp(Texture2D)
    {}

    Texture2D::Texture2D(const Size2D& size, int mipmap, void* data)
    {
        gen(size, mipmap, data);
    }

    Texture2D& Texture2D::from_current_read_buffer(const Size2D& size, const Point2D& pos, int mipmap)
    {
        EngineInstance::get_instance()->api_interface()->copy_read_buffer_to_texture_2D(_M_ID, size, pos, mipmap);
        return *this;
    }

    Texture2D& Texture2D::update_from_current_read_buffer(const Size2D& size, const Offset2D& offset, const Size2D& pos,
                                                          int mipmap)
    {
        EngineInstance::get_instance()->api_interface()->texture_2D_update_from_current_read_buffer(_M_ID, size, offset,
                                                                                                    pos, mipmap);
        return *this;
    }


    Texture2D& Texture2D::gen(const Size2D& size, int mipmap, void* data)
    {
        EngineInstance::get_instance()->api_interface()->gen_texture_2D(_M_ID, size, mipmap, data);
        return *this;
    }

    Texture2D& Texture2D::update(const Size2D& size, const Offset2D& offset, int mipmap, void* data)
    {
        EngineInstance::get_instance()->api_interface()->update_texture_2D(_M_ID, size, offset, mipmap, data);
        return *this;
    }

    Texture2D& Texture2D::read_data(std::vector<byte>& data, int level)
    {
        EngineInstance::get_instance()->api_interface()->read_texture_2D_data(_M_ID, data, level);
        return *this;
    }

    Texture2D& Texture2D::load(const String& path)
    {
        destroy();
        logger->log("Loading Texture '%ls'\n", path.c_str());
        Image image(path);

        TextureParams params;

        static PixelFormat _M_formats[5] = {PixelFormat::RGBA, PixelFormat::DEPTH, PixelFormat::DEPTH, PixelFormat::RGB,
                                            PixelFormat::RGBA};
        params.format = _M_formats[static_cast<int>(image.channels())];

        params.pixel_type = BufferValueType::UNSIGNED_BYTE;
        params.type = TextureType::Texture_2D;
        params.border = false;


        create(params);
        if (image.empty())
        {
            std::vector<byte> tmp = {100, 100, 100, 255};
            gen({1, 1}, 0, (void*) tmp.data());
        }
        else
        {
            logger->log("Image data: %zu, {%f, %f}\n", image.vector().size(), image.size().x, image.size().y);
            gen(image.size(), 0, (void*) image.vector().data()).max_mipmap_level(2).generate_mipmap();
        }

        min_filter(TextureFilter::LINEAR).mag_filter(TextureFilter::LINEAR);

        return *this;
    }

}// namespace Engine
