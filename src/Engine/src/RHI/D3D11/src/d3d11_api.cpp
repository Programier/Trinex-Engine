#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <d3d11_api.hpp>
#include <d3d9.h>
#include <tchar.h>

namespace Engine
{
    extern HWND extract_d3dx11_hwnd(class Window* main_window);
    D3D11* D3D11::m_instance = nullptr;

    implement_struct(Engine::RHI, D3D11, ).push([]() {
        Struct::static_find("Engine::RHI::D3D11", true)->struct_constructor([]() -> void* {
            if (D3D11::m_instance == nullptr)
            {
                D3D11::m_instance                       = new D3D11();
                D3D11::m_instance->info.name            = "D3D11";
                D3D11::m_instance->info.struct_instance = Struct::static_find("Engine::RHI::D3D11", true);
            }
            return D3D11::m_instance;
        });
    });

    D3D11::D3D11()
    {
        info_log("D3D11", "Creating RHI");
    }


    D3D11& D3D11::initialize(Window* window)
    {
        if (m_device != nullptr)
            return *this;
        m_main_window = window;

        uint32_t device_flags       = 0;
        D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN;

#if TRINEX_DEBUG_BUILD
        device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &m_dxgi_factory);
        trinex_always_check(result == S_OK, "Failed to create DXGI Factory");

        uint32_t current_adapter = 0;
        while (m_dxgi_factory->EnumAdapters(current_adapter, &m_dxgi_adapter) == DXGI_ERROR_NOT_FOUND)
        {
            ++current_adapter;
        }

        trinex_always_check(m_dxgi_adapter, "GPU adapter not found");

        D3D_FEATURE_LEVEL max_feature_level = D3D_FEATURE_LEVEL_11_0;
        result = D3D11CreateDevice(m_dxgi_adapter, driver_type, nullptr, device_flags, &max_feature_level, 1, D3D11_SDK_VERSION,
                                   &m_device, &m_feature_level, &m_context);
        trinex_always_check(result == S_OK, "Failed to create D3D11 Device");
        return *this;
    }

    ID3D11RenderTargetView* D3D11::create_render_target_view(ID3D11Texture2D* buffer)
    {
        ID3D11RenderTargetView* back_buffer_render_target_view = nullptr;
        D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;

        rtv_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtv_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtv_desc.Texture2D.MipSlice = 0;

        HRESULT result = m_device->CreateRenderTargetView(buffer, &rtv_desc, &back_buffer_render_target_view);
        trinex_always_check(result == S_OK, "Failed to create render target view");
        return back_buffer_render_target_view;
    }

    void D3D11::viewport(const ViewPort& viewport)
    {
        m_state.viewport = viewport;
        if (m_state.render_target_size.y > 0.f)
        {
            D3D11_VIEWPORT vp = {};
            vp.Width          = viewport.size.x;
            vp.Height         = viewport.size.y;
            vp.TopLeftX       = viewport.pos.x;
            vp.TopLeftY       = m_state.render_target_size.y - viewport.pos.y - viewport.size.y;
            vp.Height         = viewport.size.y;
            vp.MinDepth       = viewport.min_depth;
            vp.MaxDepth       = viewport.max_depth;
            m_context->RSSetViewports(1, &vp);
        }
    }

    ViewPort D3D11::viewport()
    {
        return m_state.viewport;
    }

    D3D11& D3D11::draw(size_t vertex_count, size_t vertices_offset)
    {
        m_context->Draw(vertex_count, vertices_offset);
        return *this;
    }

    void D3D11::push_debug_stage(const char* stage, const Color& color)
    {
        byte r                = static_cast<byte>(color.r * 255.f);
        byte g                = static_cast<byte>(color.g * 255.f);
        byte b                = static_cast<byte>(color.b * 255.f);
        byte a                = static_cast<byte>(color.a * 255.f);
        D3DCOLOR marker_color = D3DCOLOR_RGBA(r, g, b, a);

        static thread_local WCHAR buffer[256]{};
        MultiByteToWideChar(CP_UTF8, 0, stage, -1, buffer, 256);
        D3DPERF_BeginEvent(marker_color, buffer);
    }

    void D3D11::pop_debug_stage()
    {
        D3DPERF_EndEvent();
    }

    D3D11::~D3D11()
    {
        d3d11_release(m_dxgi_adapter);
        d3d11_release(m_dxgi_adapter);
        d3d11_release(m_device);
        d3d11_release(m_context);
    }
}// namespace Engine