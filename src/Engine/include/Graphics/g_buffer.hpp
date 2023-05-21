#pragma once
#include <Core/pointer.hpp>
#include <Graphics/framebuffer.hpp>

namespace Engine
{

    class Texture2D;

    struct ENGINE_EXPORT GBufferData {
        Pointer<Texture2D> albedo;
        Pointer<Texture2D> position;
        Pointer<Texture2D> normal;
        Pointer<Texture2D> specular;
        Pointer<Texture2D> depth;
    };

    class ENGINE_EXPORT GBuffer : public FrameBuffer
    {
    private:
        static ENGINE_EXPORT void init_g_buffer();
        GBufferData _M_buffer_data[3];
        byte index = 0;

        void swap_buffer();

        GBuffer();

    public:
        static ENGINE_EXPORT GBuffer* instance();
        const GBufferData& buffer_data();
        const GBufferData& previous_buffer_data();
        GBuffer& bind();

        friend class Window;
        friend class Object;
    };
}// namespace Engine
