#include <Core/struct.hpp>
#include <none_api.hpp>

namespace Engine
{
    NoneApi* NoneApi::m_instance = nullptr;

    implement_struct(Engine::RHI, NONE, ).push([]() {
        Struct::static_find("Engine::RHI::NONE", true)->struct_constructor([]() -> void* {
            if (NoneApi::m_instance == nullptr)
            {
                NoneApi::m_instance                       = new NoneApi();
                NoneApi::m_instance->info.name            = "None";
                NoneApi::m_instance->info.renderer        = "None";
                NoneApi::m_instance->info.struct_instance = Struct::static_find("Engine::RHI::NONE", true);
            }
            return NoneApi::m_instance;
        });
    });

    struct NoneSampler : public RHI_DefaultDestroyable<RHI_Sampler> {
        void bind(BindLocation location) override
        {}
    };

    struct NoneTexture : public RHI_DefaultDestroyable<RHI_Texture> {
        void bind(BindLocation location) override
        {}

        void bind_combined(RHI_Sampler* sampler, BindLocation location) override
        {}

        void clear_color(const Color& color) override
        {}

        void clear_depth_stencil(float depth, byte stencil) override
        {}
    };

    struct NoneShader : public RHI_DefaultDestroyable<RHI_Shader> {
    };

    struct NonePipeline : public RHI_DefaultDestroyable<RHI_Pipeline> {
        void bind() override
        {}
    };

    template<typename BufferType>
    struct NoneBuffer : public RHI_DefaultDestroyable<BufferType> {
        void update(size_t offset, size_t size, const byte* data) override
        {}
    };

    struct NoneIndexBuffer : public NoneBuffer<RHI_IndexBuffer> {
        void bind(size_t offset) override
        {}
    };

    struct NoneVertexBuffer : public NoneBuffer<RHI_VertexBuffer> {
        void bind(byte stream_index, size_t stride, size_t offset) override
        {}
    };

    struct NoneSSBOBuffer : public NoneBuffer<RHI_SSBO> {
        void bind(BindLocation location) override
        {}
    };

    struct NoneViewport : public RHI_DefaultDestroyable<RHI_Viewport> {
        void begin_render() override
        {}
        void end_render() override
        {}

        void vsync(bool flag) override
        {}

        void on_resize(const Size2D& new_size) override
        {}

        void bind() override
        {}

        void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override
        {}

        void clear_color(const Color& color) override
        {}
    };


    NoneApi& NoneApi::initialize(Window* window)
    {
        return *this;
    }

    void* NoneApi::context()
    {
        return nullptr;
    }

    NoneApi& NoneApi::draw(size_t vertex_count, size_t vertices_offset)
    {
        return *this;
    }

    NoneApi& NoneApi::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
    {
        return *this;
    }

    NoneApi& NoneApi::draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)
    {
        return *this;
    }

    NoneApi& NoneApi::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                             size_t instances)
    {
        return *this;
    }

    NoneApi& NoneApi::begin_render()
    {
        return *this;
    }

    NoneApi& NoneApi::end_render()
    {
        return *this;
    }

    NoneApi& NoneApi::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        return *this;
    }

    NoneApi& NoneApi::viewport(const ViewPort& viewport)
    {
        return *this;
    }

    ViewPort NoneApi::viewport()
    {
        return {};
    }

    NoneApi& NoneApi::scissor(const Scissor& scissor)
    {
        return *this;
    }

    Scissor NoneApi::scissor()
    {
        return {};
    }

    RHI_Sampler* NoneApi::create_sampler(const Sampler*)
    {
        return new NoneSampler();
    }

    RHI_Texture* NoneApi::create_texture_2d(const Texture2D*)
    {
        return new NoneTexture();
    }

    RHI_Texture* NoneApi::create_render_surface(const RenderSurface* surface)
    {
        return create_texture_2d(nullptr);
    }

    RHI_Shader* NoneApi::create_vertex_shader(const VertexShader* shader)
    {
        return new NoneShader();
    }

    RHI_Shader* NoneApi::create_tesselation_control_shader(const TessellationControlShader* shader)
    {
        return new NoneShader();
    }

    RHI_Shader* NoneApi::create_tesselation_shader(const TessellationShader* shader)
    {
        return new NoneShader();
    }

    RHI_Shader* NoneApi::create_geometry_shader(const GeometryShader* shader)
    {
        return new NoneShader();
    }

    RHI_Shader* NoneApi::create_fragment_shader(const FragmentShader* shader)
    {
        return new NoneShader();
    }

    RHI_Pipeline* NoneApi::create_pipeline(const Pipeline* pipeline)
    {
        return new NonePipeline();
    }

    RHI_VertexBuffer* NoneApi::create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)
    {
        return new NoneVertexBuffer();
    }

    RHI_IndexBuffer* NoneApi::create_index_buffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type)
    {
        return new NoneIndexBuffer();
    }

    RHI_SSBO* NoneApi::create_ssbo(size_t size, const byte* data, RHIBufferType type)
    {
        return new NoneSSBOBuffer();
    }

    RHI_Viewport* NoneApi::create_viewport(RenderViewport* viewport)
    {
        return new NoneViewport();
    }

    NoneApi& NoneApi::push_global_params(const GlobalShaderParameters& params)
    {
        return *this;
    }

    NoneApi& NoneApi::pop_global_params()
    {
        return *this;
    }

    NoneApi& NoneApi::update_local_parameter(const void* data, size_t size, size_t offset)
    {
        return *this;
    }

    NoneApi& NoneApi::push_debug_stage(const char* stage, const Color& color)
    {
        return *this;
    }

    NoneApi& NoneApi::pop_debug_stage()
    {
        return *this;
    }
}// namespace Engine