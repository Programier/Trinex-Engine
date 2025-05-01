#include <opengl_api.hpp>
#include <opengl_fence.hpp>

namespace Engine
{
	OpenGL_Fence::OpenGL_Fence()
	{
		m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	OpenGL_Fence::~OpenGL_Fence()
	{
		glDeleteSync(m_sync);
	}

	bool OpenGL_Fence::is_signaled()
	{
		if (m_signaled)
			return true;

		GLint status = 0;
		glGetSynciv(m_sync, GL_SYNC_STATUS, sizeof(status), nullptr, &status);

		m_signaled = status == GL_SIGNALED;
		return m_signaled;
	}

	void OpenGL_Fence::reset()
	{
		if (is_signaled())
		{
			glDeleteSync(m_sync);
			m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}
	}

	RHI_Fence* OpenGL::create_fence()
	{
		return new OpenGL_Fence();
	}

	OpenGL& OpenGL::signal_fence(RHI_Fence* fence)
	{
		static_cast<OpenGL_Fence*>(fence)->reset();
		return *this;
	}
}// namespace Engine
