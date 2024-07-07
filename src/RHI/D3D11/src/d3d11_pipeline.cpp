#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>

#include <d3d11_1.h>
#include <d3d11_api.hpp>
#include <d3d11_enums.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_shader.hpp>

namespace Engine
{

    template<typename ShaderType>
    static ShaderType* extract_shader(Shader* shader)
    {
        if (shader == nullptr)
            return nullptr;
        return shader->rhi_object<ShaderType>();
    }

    template<typename ShaderType>
    static void bind_shader(ShaderType* shader)
    {
        ShaderType::bind(shader);
    }


    static D3D11_DEPTH_STENCIL_DESC create_depth_stencil_description(const Pipeline* pipeline)
    {
        D3D11_DEPTH_STENCIL_DESC desc{};

        auto& depth   = pipeline->depth_test;
        auto& stencil = pipeline->stencil_test;

        desc.DepthEnable                  = depth.enable ? TRUE : FALSE;
        desc.DepthWriteMask               = depth.enable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc                    = comparison_func_of(depth.func);
        desc.StencilEnable                = stencil.enable ? TRUE : FALSE;
        desc.StencilReadMask              = static_cast<UINT8>(stencil.compare_mask);
        desc.StencilWriteMask             = static_cast<UINT8>(stencil.write_mask);
        desc.FrontFace.StencilFailOp      = stencil_op_of(stencil.fail);
        desc.FrontFace.StencilDepthFailOp = stencil_op_of(stencil.depth_fail);
        desc.FrontFace.StencilPassOp      = stencil_op_of(stencil.depth_pass);
        desc.FrontFace.StencilFunc        = comparison_func_of(stencil.compare);
        desc.BackFace                     = desc.FrontFace;

        return desc;
    }

    static D3D11_BLEND_DESC create_blend_description(const Pipeline* pipeline)
    {
        D3D11_BLEND_DESC desc{};
        auto& blend = pipeline->color_blending;

        desc.AlphaToCoverageEnable  = FALSE;
        desc.IndependentBlendEnable = FALSE;

        desc.RenderTarget[0].BlendEnable    = blend.enable ? TRUE : FALSE;
        desc.RenderTarget[0].SrcBlend       = blend_func_of(blend.src_color_func);
        desc.RenderTarget[0].DestBlend      = blend_func_of(blend.dst_color_func);
        desc.RenderTarget[0].SrcBlendAlpha  = blend_func_of(blend.src_alpha_func);
        desc.RenderTarget[0].DestBlendAlpha = blend_func_of(blend.dst_alpha_func);
        desc.RenderTarget[0].BlendOp        = blend_op_of(blend.color_op);
        desc.RenderTarget[0].BlendOpAlpha   = blend_op_of(blend.alpha_op);

        desc.RenderTarget[0].RenderTargetWriteMask = component_mask_of(blend.color_mask);

        constexpr size_t elements = ARRAY_SIZE(desc.RenderTarget);

        for (size_t i = 1; i < elements; ++i)
        {
            desc.RenderTarget[i] = desc.RenderTarget[0];
        }

        return desc;
    }

    static D3D11_RASTERIZER_DESC create_rasterizer_description(const Pipeline* pipeline)
    {
        D3D11_RASTERIZER_DESC desc{};
        auto& rasterizer           = pipeline->rasterizer;
        desc.FillMode              = fill_mode_of(rasterizer.polygon_mode);
        desc.CullMode              = cull_mode_of(rasterizer.cull_mode);
        desc.FrontCounterClockwise = rasterizer.front_face == FrontFace::CounterClockWise;
        desc.DepthBias             = 0.f;
        desc.DepthBiasClamp        = 0.f;
        desc.SlopeScaledDepthBias  = 0.f;
        desc.DepthClipEnable       = TRUE;
        desc.ScissorEnable         = FALSE;
        desc.MultisampleEnable     = FALSE;
        desc.AntialiasedLineEnable = FALSE;
        return desc;
    }

    bool D3D11_Pipeline::init(const class Pipeline* pipeline)
    {
        m_engine_pipeline = pipeline;
        m_vertex_shader   = extract_shader<D3D11_VertexShader>(pipeline->vertex_shader());
        m_tsc_shader      = extract_shader<D3D11_TesselationControlShader>(pipeline->tessellation_control_shader());
        m_ts_shader       = extract_shader<D3D11_TesselationShader>(pipeline->tessellation_shader());
        m_geometry_shader = extract_shader<D3D11_GeometryShader>(pipeline->geometry_shader());
        m_fragment_shader = extract_shader<D3D11_FragmentShader>(pipeline->fragment_shader());

        if (m_vertex_shader == nullptr || m_fragment_shader == nullptr)
            return false;

        m_primitive_topology = primitive_topology_of(pipeline->input_assembly.primitive_topology);

        {
            D3D11_DEPTH_STENCIL_DESC ds_desc = create_depth_stencil_description(pipeline);
            if (DXAPI->m_device->CreateDepthStencilState(&ds_desc, &m_depth_stencil_state) != S_OK)
            {
                error_log("D3D11 Pipeline", "Failed to create depth stencil state");
                return false;
            }
        }
        {
            D3D11_BLEND_DESC bl_desc = create_blend_description(pipeline);
            if (DXAPI->m_device->CreateBlendState(&bl_desc, &m_blend_state) != S_OK)
            {
                error_log("D3D11 Pipeline", "Failed to create blend state");
                return false;
            }
        }
        {
            D3D11_RASTERIZER_DESC rs_desc = create_rasterizer_description(pipeline);
            if (DXAPI->m_device->CreateRasterizerState(&rs_desc, &m_rasterizer_state) != S_OK)
            {
                error_log("D3D11 Pipeline", "Failed to create rasterizer state");
                return false;
            }
        }
        return true;
    }

    void D3D11_Pipeline::bind()
    {
        bind_shader(m_vertex_shader);
        bind_shader(m_tsc_shader);
        bind_shader(m_ts_shader);
        bind_shader(m_geometry_shader);
        bind_shader(m_fragment_shader);

        DXAPI->m_context->IASetPrimitiveTopology(m_primitive_topology);
        DXAPI->m_context->OMSetDepthStencilState(m_depth_stencil_state, 0);
        DXAPI->m_context->OMSetBlendState(m_blend_state, nullptr, 0xFFFFFFFF);
        DXAPI->m_context->RSSetState(m_rasterizer_state);

        DXAPI->m_state.pipeline = this;
    }

    D3D11_Pipeline::~D3D11_Pipeline()
    {
        d3d11_release(m_depth_stencil_state);
        d3d11_release(m_blend_state);
        d3d11_release(m_rasterizer_state);
    }

    RHI_Pipeline* D3D11::create_pipeline(const Pipeline* pipeline)
    {
        D3D11_Pipeline* d3d11_pipeline = new D3D11_Pipeline();
        if (!d3d11_pipeline->init(pipeline))
        {
            delete d3d11_pipeline;
            d3d11_pipeline = nullptr;
        }
        return d3d11_pipeline;
    }
}// namespace Engine
