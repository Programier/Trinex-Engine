#pragma once

#include <d3d11.h>
#include <none_api.hpp>


namespace Engine
{
#define DXAPI Engine::D3D11::m_instance
    template<typename T>
    FORCE_INLINE void d3d11_release(T*& address)
    {
        if (address)
        {
            address->Release();
            address = nullptr;
        }
    }

    struct State {
        ViewPort viewport;
        Size2D render_target_size = {-1.f, -1.f};
    };

    class D3D11 : public NoneApi
    {
    public:
        static D3D11* m_instance;
        State m_state;

        Window* m_main_window = nullptr;

        IDXGIFactory* m_dxgi_factory   = nullptr;
        IDXGIAdapter* m_dxgi_adapter   = nullptr;
        ID3D11Device* m_device         = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        D3D_FEATURE_LEVEL m_feature_level = D3D_FEATURE_LEVEL_11_0;

        D3D11();
        ID3D11RenderTargetView* create_render_target_view(ID3D11Texture2D* buffer);

        D3D11& initialize(Window* window) override;
        RHI_Viewport* create_viewport(RenderViewport* viewport) override;
        void viewport(const ViewPort& viewport) override;
        ViewPort viewport() override;


        D3D11& draw(size_t vertex_count, size_t vertices_offset) override;

        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;

        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;
        ~D3D11();
    };
}// namespace Engine
