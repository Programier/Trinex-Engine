#pragma once
#include <Graphics/texture.hpp>

namespace Engine
{
    CLASS Texture2D : public Texture
    {
    public:
        Texture2D();
        Texture2D(const Texture2D&);
        Texture2D(Texture2D&&);
        Texture2D& operator=(const Texture2D&);
        Texture2D& operator=(Texture2D&&);
        Texture2D(const Size2D& size, int mipmap = 0, void* data = nullptr);

        Texture2D& from_current_read_buffer(const Size2D& size, const Point2D& pos = {0, 0}, int mipmap = 0);
        Texture2D& gen(const Size2D& size, int mipmap = 0, void* data = nullptr);
        Texture2D& update(const Size2D& size, const Offset2D& offset, int mipmap = 0, void* data = nullptr);
        Texture2D& update_from_current_read_buffer(const Size2D& size, const Offset2D& offset, const Size2D& pos = {0, 0},
                                                   int mipmap = 0);

        Texture2D& read_data(std::vector<byte>& data, int level = 0);
    };
}// namespace Engine
