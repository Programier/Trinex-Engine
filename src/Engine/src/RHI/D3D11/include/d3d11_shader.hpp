#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{

    class D3D11_VertexShader : public RHI_Shader
    {
    public:
        ID3D11VertexShader* m_shader = nullptr;
        ID3D11InputLayout* m_layout  = nullptr;

        bool init(const class VertexShader* shader);
        static void bind(D3D11_VertexShader* self);
        ~D3D11_VertexShader();
    };

    class D3D11_TesselationControlShader : public RHI_Shader
    {
    public:
        ID3D11HullShader* m_shader = nullptr;

        bool init(const class TessellationControlShader* shader);
        static void bind(D3D11_TesselationControlShader* self);
        ~D3D11_TesselationControlShader();
    };

    class D3D11_TesselationShader : public RHI_Shader
    {
    public:
        ID3D11DomainShader* m_shader = nullptr;

        bool init(const class TessellationShader* shader);
        static void bind(D3D11_TesselationShader* self);
        ~D3D11_TesselationShader();
    };

    class D3D11_GeometryShader : public RHI_Shader
    {
    public:
        ID3D11GeometryShader* m_shader = nullptr;

        bool init(const class GeometryShader* shader);
        static void bind(D3D11_GeometryShader* self);
        ~D3D11_GeometryShader();
    };

    class D3D11_FragmentShader : public RHI_Shader
    {
    public:
        ID3D11PixelShader* m_shader = nullptr;

        bool init(const class FragmentShader* shader);
        static void bind(D3D11_FragmentShader* self);
        ~D3D11_FragmentShader();
    };
}// namespace Engine
