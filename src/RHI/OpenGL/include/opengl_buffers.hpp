#pragma once
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>
#include <opengl_resource.hpp>

namespace Engine
{
	template<GLenum target>
	struct OpenGL_BufferSRV : public OpenGL_SRV {
		GLuint m_id;

		OpenGL_BufferSRV(GLuint id) : m_id(id) {}
		void bind(byte location, OpenGL_Sampler*) override { glBindBufferBase(target, location, m_id); }
	};

	template<GLenum target>
	struct OpenGL_BufferUAV : public OpenGL_UAV {
		GLuint m_id;

		OpenGL_BufferUAV(GLuint id) : m_id(id) {}
		void bind(byte location) override { glBindBufferBase(target, location, m_id); }
	};

	struct OpenGL_Buffer : public RHI_DefaultDestroyable<RHI_Buffer> {
		OpenGL_SRV* m_srv = nullptr;
		OpenGL_UAV* m_uav = nullptr;

		byte* m_mapped = nullptr;
		GLuint m_target;
		GLuint m_id;

		OpenGL_Buffer(size_t size, const byte* data, BufferCreateFlags flags);
		~OpenGL_Buffer();

		byte* map() override;
		void unmap() override;
		void update(size_t offset, size_t size, const byte* data) override;

		inline RHI_ShaderResourceView* as_srv() override { return m_srv; }
		inline RHI_UnorderedAccessView* as_uav() override { return m_uav; }
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
