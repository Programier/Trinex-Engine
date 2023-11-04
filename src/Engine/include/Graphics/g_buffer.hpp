#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_target.hpp>
#include <Core/executable_object.hpp>

namespace Engine
{

    class Texture2D;

    class ENGINE_EXPORT GBuffer : public Singletone<GBuffer, RenderTarget>
    {
        declare_class(GBuffer, Object);

    public:
        Pointer<Texture2D> albedo;
        Pointer<Texture2D> position;
        Pointer<Texture2D> normal;
        Pointer<Texture2D> specular;
        Pointer<Texture2D> depth;


    private:
        static GBuffer* _M_instance;
        Vector<ColorFormat> _M_color_formats;

        GBuffer();
        ~GBuffer();

        void init();

    public:
        GBuffer& resize();
        friend class Singletone<GBuffer, RenderTarget>;
        friend class Object;
    };
}// namespace Engine
