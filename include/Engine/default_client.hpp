#pragma once
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>

namespace Engine
{

    struct Vertex {
        Color4 color;
        Vector2D uv;
    };

    // This viewport can be used for testing something
    class ENGINE_EXPORT DefaultClient : public ViewportClient
    {
        declare_class(DefaultClient, ViewportClient);

    public:
        Pointer<TypedVertexBuffer<Vertex>> m_vertex_buffer;
        Pointer<UInt32IndexBuffer> m_index_buffer;
        Pointer<Material> m_material;

        DefaultClient();
        DefaultClient& on_bind_viewport(class RenderViewport* viewport) override;
        DefaultClient& render(class RenderViewport* viewport) override;
        DefaultClient& update(class RenderViewport* viewport, float dt) override;
    };
}// namespace Engine
