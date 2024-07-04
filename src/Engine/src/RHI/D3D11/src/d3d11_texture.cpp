#include <Core/logger.hpp>
#include <Graphics/texture_2D.hpp>
#include <d3d11.h>
#include <d3d11_api.hpp>
#include <d3d11_enums.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_sampler.hpp>
#include <d3d11_texture.hpp>

namespace Engine
{
    void D3D11_Texture::clear_color(const Color& color)
    {}

    void D3D11_Texture::clear_depth_stencil(float depth, byte stencil)
    {}

    static UINT format_block_size(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_R32_FLOAT:
                return sizeof(float);

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                return sizeof(float) * 2;

            case DXGI_FORMAT_R8_UNORM:
                return sizeof(uint8_t);

            case DXGI_FORMAT_R8G8B8A8_UNORM:
                return sizeof(uint8_t) * 4;

            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return sizeof(float) + sizeof(uint32_t);

            case DXGI_FORMAT_R16_TYPELESS:
                return sizeof(uint16_t);

            case DXGI_FORMAT_D32_FLOAT:
                return sizeof(float);

            case DXGI_FORMAT_R32_TYPELESS:
                return sizeof(float);

            case DXGI_FORMAT_BC1_UNORM:
                return 2;

            case DXGI_FORMAT_BC2_UNORM:
                return 4;

            case DXGI_FORMAT_BC3_UNORM:
                return 4;

            default:
                throw EngineException("Undefined format");
        }
    }

    bool D3D11_Texture2D::init(const Texture2D* texture)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width              = static_cast<UINT>(texture->width());
        desc.Height             = static_cast<UINT>(texture->height());
        desc.MipLevels          = static_cast<UINT>(texture->mipmap_count());
        desc.ArraySize          = 1;
        desc.Format             = format_of(texture->format());
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        Vector<D3D11_SUBRESOURCE_DATA> sub_resource_data;
        sub_resource_data.resize(texture->mipmap_count());

        for (size_t i = 0, count = texture->mipmap_count(); i < count; ++i)
        {
            D3D11_SUBRESOURCE_DATA& data = sub_resource_data[i];
            const auto& mip              = texture->mip(i);
            data.pSysMem                 = mip->data.data();
            data.SysMemSlicePitch        = 0;
            data.SysMemPitch             = static_cast<UINT>(mip->size.x) * format_block_size(desc.Format);
        }

        HRESULT hr = DXAPI->m_device->CreateTexture2D(&desc, sub_resource_data.empty() ? nullptr : sub_resource_data.data(),
                                                      &m_texture);

        if (hr != S_OK)
        {
            error_log("D3D11", "Failed to create texture!");
            return false;
        }


        D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
        view_desc.Format                    = desc.Format;
        view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MostDetailedMip = 0;
        view_desc.Texture2D.MipLevels       = desc.MipLevels;

        if (view_desc.Format == DXGI_FORMAT_R32_TYPELESS)
        {
            view_desc.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (view_desc.Format == DXGI_FORMAT_R16_TYPELESS)
        {
            view_desc.Format = DXGI_FORMAT_R16_UNORM;
        }
        else if (view_desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
        {
            view_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        }

        hr = DXAPI->m_device->CreateShaderResourceView(m_texture, &view_desc, &m_view);
        if (hr != S_OK)
        {
            error_log("D3D11", "Failed to texture view!");
            return false;
        }

        return true;
    }

    void D3D11_Texture2D::bind(BindLocation location)
    {
        auto& pipeline = DXAPI->m_state.pipeline;

        if (pipeline == nullptr)
            return;

        if (pipeline->m_vertex_shader)
            DXAPI->m_context->VSSetShaderResources(location.binding, 1, &m_view);
        if (pipeline->m_tsc_shader)
            DXAPI->m_context->HSSetShaderResources(location.binding, 1, &m_view);
        if (pipeline->m_ts_shader)
            DXAPI->m_context->DSSetShaderResources(location.binding, 1, &m_view);
        if (pipeline->m_geometry_shader)
            DXAPI->m_context->GSSetShaderResources(location.binding, 1, &m_view);
        if (pipeline->m_fragment_shader)
            DXAPI->m_context->PSSetShaderResources(location.binding, 1, &m_view);
    }

    void D3D11_Texture2D::bind_combined(RHI_Sampler* sampler, BindLocation location)
    {
        bind(location);
        //reinterpret_cast<D3D11_Sampler*>(sampler)->bind(location);
    }

    D3D11_Texture2D::~D3D11_Texture2D()
    {
        d3d11_release(m_view);
        d3d11_release(m_texture);
    }

    RHI_Texture* D3D11::create_texture_2d(const Texture2D* texture)
    {
        D3D11_Texture2D* d3d11_texture = new D3D11_Texture2D();

        if (!d3d11_texture->init(texture))
        {
            delete d3d11_texture;
            d3d11_texture = nullptr;
        }

        return d3d11_texture;
    }

}// namespace Engine
