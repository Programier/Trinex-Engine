#include <Core/exception.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <d3d11_api.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_viewport.hpp>

namespace Engine
{
    extern HWND extract_d3dx11_hwnd(class Window* window);

    bool D3D11_Viewport::is_window_viewport() const
    {
        return false;
    }

    ID3D11RenderTargetView* D3D11_Viewport::render_target()
    {
        return nullptr;
    }

    Size2D D3D11_Viewport::render_target_size() const
    {
        return {0.f, 0.f};
    }

    bool D3D11_WindowViewport::is_window_viewport() const
    {
        return true;
    }

    ID3D11RenderTargetView* D3D11_WindowViewport::render_target()
    {
        return m_view;
    }

    Size2D D3D11_WindowViewport::render_target_size() const
    {
        return m_size;
    }

    void D3D11_WindowViewport::init(class Window* window)
    {
        m_window = window;
        create_swapchain(window->size());
    }

    void D3D11_WindowViewport::create_swapchain(const Size2D& size)
    {
        DXGI_SWAP_CHAIN_DESC sd               = {};
        sd.BufferCount                        = 1;
        sd.BufferDesc.Width                   = static_cast<uint_t>(size.x);
        sd.BufferDesc.Height                  = static_cast<uint_t>(size.y);
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 75;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = extract_d3dx11_hwnd(m_window);
        sd.SampleDesc.Count                   = 1;
        sd.SampleDesc.Quality                 = 0;
        sd.Windowed                           = true;

        auto hr = DXAPI->m_dxgi_factory->CreateSwapChain(DXAPI->m_device, &sd, &m_swap_chain);
        trinex_always_check(hr == S_OK, "Failed to create swapchain!");

        hr = DXAPI->m_dxgi_factory->MakeWindowAssociation(sd.OutputWindow, DXGI_MWA_NO_WINDOW_CHANGES);
        trinex_always_check(hr == S_OK, "Failed to make window association!");

        hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &m_back_buffer);
        trinex_always_check(hr == S_OK, "Failed to create backbuffer");

        m_view = DXAPI->create_render_target_view(m_back_buffer, DXGI_FORMAT_R8G8B8A8_UNORM);
        m_size = size;
    }

    void D3D11_WindowViewport::begin_render()
    {
        ViewPort viewport;
        viewport.pos       = {0, 0};
        viewport.size      = m_size;
        viewport.min_depth = 0.f;
        viewport.max_depth = 1.f;

        DXAPI->m_state.render_viewport = this;
        DXAPI->m_state.viewport_mode   = D3D11_ViewportMode::Undefined;
        DXAPI->viewport(viewport);
    }

    void D3D11_WindowViewport::end_render()
    {
        m_swap_chain->Present(m_with_vsync ? 1 : 0, 0);
    }

    void D3D11_WindowViewport::vsync(bool flag)
    {
        m_with_vsync = flag;
    }

    void D3D11_WindowViewport::on_resize(const Size2D& new_size)
    {
        m_size = new_size;

        if (m_swap_chain)
        {
            auto context = DXAPI->m_context;
            context->ClearState();
            context->Flush();

            d3d11_release(m_back_buffer);
            d3d11_release(m_view);

            HRESULT result =
                    m_swap_chain->ResizeBuffers(1, static_cast<uint_t>(new_size.x), static_cast<uint_t>(new_size.y),
                                                DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
            trinex_always_check(result == S_OK, "Failed to resize swapchain");

            result = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &m_back_buffer);
            trinex_always_check(result == S_OK, "Failed to create backbuffer");

            m_view = DXAPI->create_render_target_view(m_back_buffer, DXGI_FORMAT_R8G8B8A8_UNORM);
        }
    }

    void D3D11_WindowViewport::bind()
    {
        D3D11_Pipeline::unbind();
        DXAPI->m_state.render_viewport = this;
        DXAPI->m_context->OMSetRenderTargets(1, &m_view, nullptr);

        if (DXAPI->current_viewport_mode() != DXAPI->m_state.viewport_mode)
        {
            DXAPI->viewport(DXAPI->m_state.viewport);
        }
    }

    void D3D11_WindowViewport::blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
                                           SamplerFilter filter)
    {}

    void D3D11_WindowViewport::clear_color(const Color& color)
    {
        DXAPI->m_context->ClearRenderTargetView(m_view, &color.x);
    }

    D3D11_WindowViewport::~D3D11_WindowViewport()
    {
        d3d11_release(m_swap_chain);
        d3d11_release(m_back_buffer);
        d3d11_release(m_view);
    }

    RHI_Viewport* D3D11::create_viewport(RenderViewport* viewport)
    {
        D3D11_WindowViewport* result = new D3D11_WindowViewport();
        result->init(viewport->window());
        return result;
    }
}// namespace Engine
