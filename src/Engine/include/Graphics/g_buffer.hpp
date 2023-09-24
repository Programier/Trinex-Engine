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

    class ENGINE_EXPORT GBuffer : public Object
    {
        declare_class(GBuffer, Object);

    public:

        static void static_init();
    };
}// namespace Engine
