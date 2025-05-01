#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_Fence : public RHI_DefaultDestroyable<RHI_Fence> {
		GLsync m_sync   = nullptr;
		bool m_signaled = false;

		OpenGL_Fence();
		~OpenGL_Fence();

		bool is_signaled() override;
		void reset() override;
	};
}// namespace Engine
