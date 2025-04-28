#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_Fence : public RHI_DefaultDestroyable<RHI_Fence> {
		GLsync m_sync;

		OpenGL_Fence();
		~OpenGL_Fence();
		
		void wait() override;
		bool is_signaled() override;
		void reset() override;
	};
}// namespace Engine
