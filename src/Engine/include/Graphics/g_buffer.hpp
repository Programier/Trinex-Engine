#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_target.hpp>

namespace Engine
{

    class Texture2D;

    class ENGINE_EXPORT GBuffer : public Singletone<GBuffer, RenderTarget>
    {
        declare_class(GBuffer, Object);

    public:
        struct Frame : public RenderTarget::Frame {
            Texture2D* albedo() const;
            Texture2D* position() const;
            Texture2D* normal() const;
            Texture2D* specular() const;
            Texture2D* depth() const;
        };

    private:
        GBuffer();
        ~GBuffer();

        void init(bool is_reinit = false);

    public:
        GBuffer& resize();
        friend class Singletone<GBuffer, RenderTarget>;
        friend class Object;
    };
}// namespace Engine
