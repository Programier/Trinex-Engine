#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <d3d11_api.hpp>
#include <d3d11_shader.hpp>

namespace Engine
{
    static const char* semantic_name(VertexBufferSemantic semantic)
    {
        switch (semantic)
        {
            case VertexBufferSemantic::Position:
                return "POSITION";
            case VertexBufferSemantic::TexCoord:
                return "TEXCOORD";
            case VertexBufferSemantic::Color:
                return "COLOR";
            case VertexBufferSemantic::Normal:
                return "NORMAL";
            case VertexBufferSemantic::Tangent:
                return "TANGENT";
            case VertexBufferSemantic::Binormal:
                return "BINORMAL";
            case VertexBufferSemantic::BlendWeight:
                return "BLENDWEIGHT";
            case VertexBufferSemantic::BlendIndices:
                return "BLENDINDICES";
            default:
                throw EngineException("Undefined semantic");
        }
    }

    static DXGI_FORMAT format_of(VertexBufferElementType type)
    {
        switch (type)
        {
            case VertexBufferElementType::Float1:
                return DXGI_FORMAT_R32_FLOAT;
            case VertexBufferElementType::Float2:
                return DXGI_FORMAT_R32G32_FLOAT;
            case VertexBufferElementType::Float3:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case VertexBufferElementType::Float4:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case VertexBufferElementType::UByte4:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case VertexBufferElementType::UByte4N:
            case VertexBufferElementType::Color:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            default:
                throw EngineException("Undefined vertex element type");
        }
    }

    bool D3D11_VertexShader::init(const class VertexShader* shader)
    {
        const byte* data = shader->source_code.data();
        size_t size      = shader->source_code.size();
        HRESULT hr       = DXAPI->m_device->CreateVertexShader(data, size, nullptr, &m_shader);

        if (hr != S_OK)
        {
            error_log("D3D11 VertexShader", "Failed to create vertex shader");
            return false;
        }


        Vector<D3D11_INPUT_ELEMENT_DESC> inputs;

        for (auto& attribute : shader->attributes)
        {
            D3D11_INPUT_ELEMENT_DESC desc{};
            desc.SemanticName         = semantic_name(attribute.semantic);
            desc.SemanticIndex        = attribute.semantic_index;
            desc.Format               = format_of(attribute.type);
            desc.InputSlot            = attribute.stream_index;
            desc.AlignedByteOffset    = attribute.offset;
            desc.InputSlotClass       = attribute.rate == VertexAttributeInputRate::Vertex ? D3D11_INPUT_PER_VERTEX_DATA
                                                                                           : D3D11_INPUT_PER_INSTANCE_DATA;
            desc.InstanceDataStepRate = attribute.rate == VertexAttributeInputRate::Vertex ? 0 : 1;
            inputs.emplace_back(desc);
        }

        hr = DXAPI->m_device->CreateInputLayout(inputs.data(), inputs.size(), data, size, &m_layout);

        if (hr != S_OK)
        {
            error_log("D3D11 VertexShader", "Failed to create vertex input layout");
            return false;
        }

        return true;
    }

    void D3D11_VertexShader::bind(D3D11_VertexShader* self)
    {
        auto context = DXAPI->m_context;

        if (self)
        {
            context->VSSetShader(self->m_shader, nullptr, 0);
            context->IASetInputLayout(self->m_layout);
        }
        else
        {
            context->VSSetShader(nullptr, nullptr, 0);
            context->IASetInputLayout(nullptr);

            ID3D11Buffer* null_constants_buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {nullptr};
            context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, null_constants_buffers);

            ID3D11ShaderResourceView* null_srv[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {nullptr};
            context->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, null_srv);

            ID3D11SamplerState* null_samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {nullptr};
            context->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, null_samplers);
        }
    }

    D3D11_VertexShader::~D3D11_VertexShader()
    {
        d3d11_release(m_layout);
        d3d11_release(m_shader);
    }

    bool D3D11_TesselationControlShader::init(const class TessellationControlShader* shader)
    {
        const byte* data = shader->source_code.data();
        size_t size      = shader->source_code.size();
        bool result      = DXAPI->m_device->CreateHullShader(data, size, nullptr, &m_shader) == S_OK;

        if (!result)
        {
            error_log("D3D11 TesselationControlShader", "Failed to create tesselation control shader");
        }
        return result;
    }

    void D3D11_TesselationControlShader::bind(D3D11_TesselationControlShader* self)
    {
        DXAPI->m_context->HSSetShader(self ? self->m_shader : nullptr, nullptr, 0);
    }

    D3D11_TesselationControlShader::~D3D11_TesselationControlShader()
    {
        d3d11_release(m_shader);
    }

    bool D3D11_TesselationShader::init(const class TessellationShader* shader)
    {
        const byte* data = shader->source_code.data();
        size_t size      = shader->source_code.size();
        bool result      = DXAPI->m_device->CreateDomainShader(data, size, nullptr, &m_shader) == S_OK;

        if (!result)
        {
            error_log("D3D11 TesselationControlShader", "Failed to create tesselation control shader");
        }
        return result;
    }

    void D3D11_TesselationShader::bind(D3D11_TesselationShader* self)
    {
        DXAPI->m_context->DSSetShader(self ? self->m_shader : nullptr, nullptr, 0);
    }

    D3D11_TesselationShader::~D3D11_TesselationShader()
    {
        d3d11_release(m_shader);
    }

    bool D3D11_GeometryShader::init(const class GeometryShader* shader)
    {
        const byte* data = shader->source_code.data();
        size_t size      = shader->source_code.size();
        bool result      = DXAPI->m_device->CreateGeometryShader(data, size, nullptr, &m_shader) == S_OK;

        if (!result)
        {
            error_log("D3D11 TesselationControlShader", "Failed to create tesselation control shader");
        }
        return result;
    }

    void D3D11_GeometryShader::bind(D3D11_GeometryShader* self)
    {
        DXAPI->m_context->GSSetShader(self ? self->m_shader : nullptr, nullptr, 0);
    }

    D3D11_GeometryShader::~D3D11_GeometryShader()
    {
        d3d11_release(m_shader);
    }

    bool D3D11_FragmentShader::init(const class FragmentShader* shader)
    {
        const byte* data = shader->source_code.data();
        size_t size      = shader->source_code.size();
        bool result      = DXAPI->m_device->CreatePixelShader(data, size, nullptr, &m_shader) == S_OK;

        if (!result)
        {
            error_log("D3D11 FragmentShader", "Failed to create fragment shader");
        }
        return result;
    }

    void D3D11_FragmentShader::bind(D3D11_FragmentShader* self)
    {
        auto context = DXAPI->m_context;
        if (self)
        {
            context->PSSetShader(self->m_shader, nullptr, 0);
        }
        else
        {
            context->PSSetShader(nullptr, nullptr, 0);

            ID3D11Buffer* null_constants_buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {nullptr};
            context->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, null_constants_buffers);

            ID3D11ShaderResourceView* null_srv[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {nullptr};
            context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, null_srv);

            ID3D11SamplerState* null_samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {nullptr};
            context->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, null_samplers);
        }
    }

    D3D11_FragmentShader::~D3D11_FragmentShader()
    {
        d3d11_release(m_shader);
    }


    template<typename DXShader, typename EngineShader>
    static FORCE_INLINE RHI_Shader* create_shader_internal(const EngineShader* shader)
    {
        DXShader* dx_shader = new DXShader();

        if (!dx_shader->init(shader))
        {
            delete dx_shader;
            dx_shader = nullptr;
        }

        return dx_shader;
    }

    RHI_Shader* D3D11::create_vertex_shader(const VertexShader* shader)
    {
        return create_shader_internal<D3D11_VertexShader>(shader);
    }

    RHI_Shader* D3D11::create_tesselation_control_shader(const TessellationControlShader* shader)
    {
        return create_shader_internal<D3D11_TesselationControlShader>(shader);
    }

    RHI_Shader* D3D11::create_tesselation_shader(const TessellationShader* shader)
    {
        return create_shader_internal<D3D11_TesselationShader>(shader);
    }

    RHI_Shader* D3D11::create_geometry_shader(const GeometryShader* shader)
    {
        return create_shader_internal<D3D11_GeometryShader>(shader);
    }

    RHI_Shader* D3D11::create_fragment_shader(const FragmentShader* shader)
    {
        return create_shader_internal<D3D11_FragmentShader>(shader);
    }


}// namespace Engine
