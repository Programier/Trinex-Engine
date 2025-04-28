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

	void OpenGL_Fence::wait()
	{
		glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(-1));
	}

	bool OpenGL_Fence::is_signaled()
	{
		GLint status = 0;
		glGetSynciv(m_sync, GL_SYNC_STATUS, sizeof(status), nullptr, &status);
		return status == GL_SIGNALED;
	}

	void OpenGL_Fence::reset()
	{
		glDeleteSync(m_sync);
		m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	RHI_Fence* OpenGL::create_fence()
	{
		return new OpenGL_Fence();
	}
}// namespace Engine
