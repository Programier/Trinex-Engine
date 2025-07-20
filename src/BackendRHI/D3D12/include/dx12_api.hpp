#pragma once
#include <dx12_headers.hpp>
#include <dxgi1_4.h>
#include <none_api.hpp>

namespace Engine
{
	class D3D12DescritorManager;
	class D3D12CommandAlloctor;
	class D3D12_State;
	class D3D12UploadBufferManager;

	class D3D12 : public NoneApi
	{
		trinex_declare_struct(D3D12, void);

	private:
		static D3D12* s_d3d12;

		ComPtr<IDXGIFactory4> m_factory;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12Debug> m_debug;
		ComPtr<ID3D12CommandQueue> m_command_queue;
		ComPtr<ID3D12GraphicsCommandList> m_cmd_list;
		ComPtr<ID3D12RootSignature> m_root_signature;
		D3D12DescritorManager* m_descriptor_manager;
		D3D12CommandAlloctor* m_command_allocator;
		D3D12UploadBufferManager* m_upload_buffer_manager;
		D3D12_State* m_state;

	public:
		static D3D12* static_constructor();
		static void static_destructor(D3D12* d3d12);
		static D3D12* api() { return s_d3d12; }

		inline IDXGIFactory4* factory() const { return m_factory.Get(); }
		inline ID3D12Device* device() const { return m_device.Get(); }
		inline ID3D12CommandQueue* command_queue() const { return m_command_queue.Get(); }
		inline ID3D12GraphicsCommandList* cmd_list() const { return m_cmd_list.Get(); }
		inline ID3D12RootSignature* root_signature() const { return m_root_signature.Get(); }
		inline D3D12DescritorManager* descriptor_manager() const { return m_descriptor_manager; }
		inline D3D12UploadBufferManager* upload_buffer_manager() const { return m_upload_buffer_manager; }
		inline D3D12_State* state() const { return m_state; }

		D3D12();
		~D3D12();

		D3D12& compile_root_signature();
		D3D12& rebind_descriptors();
		D3D12& deferred_destroy(RHIObject* object);

		RHITexture* create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags) override;
		RHIBuffer* create_buffer(size_t size, const byte* data, BufferCreateFlags flags) override;
		RHIShader* create_vertex_shader(const byte* shader, size_t size, const VertexAttribute* attributes,
		                                 size_t attributes_count) override;
		RHIShader* create_tesselation_control_shader(const byte* shader, size_t size) override;
		RHIShader* create_tesselation_shader(const byte* shader, size_t size) override;
		RHIShader* create_geometry_shader(const byte* shader, size_t size) override;
		RHIShader* create_fragment_shader(const byte* shader, size_t size) override;
		RHIShader* create_compute_shader(const byte* shader, size_t size) override;

		RHIPipeline* create_graphics_pipeline(const GraphicsPipeline* pipeline) override;
		RHIPipeline* create_compute_pipeline(const ComputePipeline* pipeline) override;
		RHIViewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;

		D3D12& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) override;

		D3D12& barrier(RHITexture* texture, RHIAccess access) override;
		D3D12& barrier(RHIBuffer* buffer, RHIAccess access) override;

		D3D12& submit() override;

		D3D12& viewport(const ViewPort& viewport) override;
		D3D12& scissor(const Scissor& scissor) override;

		D3D12& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream) override;
		D3D12& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format) override;
		D3D12& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                          RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) override;

		D3D12& draw(size_t vertex_count, size_t vertices_offset) override;
	};
}// namespace Engine
