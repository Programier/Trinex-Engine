#include <Core/implement.hpp>
#include <Graphics/texture_2D.hpp>
#include <api_funcs.hpp>


namespace Engine
{
    implement_class_cpp(Texture2D);

    Texture2D::Texture2D(const Size2D& size, int mipmap, void* data)
    {
        gen(size, mipmap, data);
    }

    Texture2D& Texture2D::from_current_read_buffer(const Size2D& size, const Point2D& pos, int mipmap)
    {
        copy_read_buffer_to_texture_2D(_M_ID, size, pos, mipmap);
        return *this;
    }

    Texture2D& Texture2D::update_from_current_read_buffer(const Size2D& size, const Offset2D& offset, const Size2D& pos,
                                                          int mipmap)
    {
        texture_2D_update_from_current_read_buffer(_M_ID, size, offset, pos, mipmap);
        return *this;
    }


    Texture2D& Texture2D::gen(const Size2D& size, int mipmap, void* data)
    {
        gen_texture_2D(_M_ID, size, mipmap, data);
        return *this;
    }

    Texture2D& Texture2D::update(const Size2D& size, const Offset2D& offset, int mipmap, void* data)
    {
        update_texture_2D(_M_ID, size, offset, mipmap, data);
        return *this;
    }

    Texture2D& Texture2D::read_data(std::vector<byte>& data, int level)
    {
        read_texture_2D_data(_M_ID, data, level);
        return *this;
    }

}// namespace Engine
