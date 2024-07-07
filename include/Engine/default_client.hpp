#pragma once
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>

namespace Engine
{
    // This viewport can be used for testing something
    class ENGINE_EXPORT DefaultClient : public ViewportClient
    {
        declare_class(DefaultClient, ViewportClient);

        Vector4D color = {1.f, 0, 0, 0.f};
        float factor   = 0.5;

    public:
        Pointer<TexCoordVertexBuffer> m_vertex_buffer;
        Pointer<Material> m_material;

        DefaultClient();
        DefaultClient& on_bind_viewport(class RenderViewport* viewport) override;
        DefaultClient& render(class RenderViewport* viewport) override;
        DefaultClient& update(class RenderViewport* viewport, float dt) override;
    };
}// namespace Engine
