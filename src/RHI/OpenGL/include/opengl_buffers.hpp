#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_VertexBuffer : public RHI_DefaultDestroyable<RHI_VertexBuffer> {
		GLuint m_id;


		OpenGL_VertexBuffer(size_t size, const byte* data, RHIBufferType type);
		void bind(byte stream_index, size_t stride, size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;

		~OpenGL_VertexBuffer();
	};

	struct OpenGL_IndexBuffer : public RHI_DefaultDestroyable<RHI_IndexBuffer> {
		GLuint m_id;
		GLuint m_format;

		OpenGL_IndexBuffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type);
		void bind(size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;

		~OpenGL_IndexBuffer();
	};

	struct OpenGL_SSBO : public RHI_DefaultDestroyable<RHI_SSBO> {
		GLuint m_id;

		OpenGL_SSBO(size_t size, const byte* data);

		void bind(BindLocation location) override;
		void update(size_t offset, size_t size, const byte* data) override;

		~OpenGL_SSBO();
	};
	
	struct OpenGL_GlobalUniformBufferManager {
	private:
		Vector<struct OpenGL_GlobalUniformBuffer*> m_buffers;
		int_t m_index = -1;
		
		struct OpenGL_GlobalUniformBuffer* buffer();
		
	public:
		void bind(BindingIndex index);
		void push(const GlobalShaderParameters& params);
		void pop();
		void submit();
		~OpenGL_GlobalUniformBufferManager();
	};
	
	struct OpenGL_LocalUniformBufferManager {
	private:
		Vector<struct OpenGL_LocalUniformBuffer*> m_buffers;
		int_t m_index = -1;

		struct OpenGL_LocalUniformBuffer* buffer();

	public:
		void bind(BindingIndex index);
		void update(const void* data, size_t size, size_t offset);
		void submit();
		~OpenGL_LocalUniformBufferManager();
	};

}// namespace Engine
