#pragma once
#include <Core/etl/vector.hpp>
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

		OpenGL_IndexBuffer(size_t, const byte* data, RHIIndexFormat format, RHIBufferType type);
		void bind(size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;

		~OpenGL_IndexBuffer();
	};

	struct OpenGL_UniformBuffer : public RHI_DefaultDestroyable<RHI_UniformBuffer> {
		GLuint m_id;

		OpenGL_UniformBuffer(size_t size, const byte* data, RHIBufferType type);
		void bind(BindingIndex location) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};

	struct OpenGL_SSBO : public RHI_DefaultDestroyable<RHI_SSBO> {
		GLuint m_id;

		OpenGL_SSBO(size_t size, const byte* data);

		void bind(BindLocation location) override;
		void update(size_t offset, size_t size, const byte* data) override;

		~OpenGL_SSBO();
	};

	struct OpenGL_LocalUniformBufferManager {
	private:
		Vector<struct OpenGL_LocalUniformBufferPool*> m_buffers;

	public:
		void bind();
		void update(const void* data, size_t size, size_t offset, BindingIndex index);
		void submit();
		~OpenGL_LocalUniformBufferManager();
	};

}// namespace Engine
