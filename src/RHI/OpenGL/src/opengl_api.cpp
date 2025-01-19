#include <Core/definitions.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/struct.hpp>
#include <Window/config.hpp>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_render_target.hpp>
#include <opengl_shader.hpp>

namespace Engine
{
	namespace TRINEX_RHI
	{
		using OPENGL = OpenGL;
	}

	implement_struct_default_init(Engine::TRINEX_RHI::OPENGL, 0);

	OpenGL* OpenGL::static_constructor()
	{
		if (OpenGL::m_instance == nullptr)
		{
			OpenGL::m_instance                       = new OpenGL();
			OpenGL::m_instance->info.struct_instance = static_struct_instance();
		}

		return OpenGL::m_instance;
	}

	void OpenGL::static_destructor(OpenGL* opengl)
	{
		if (opengl == m_instance)
		{
			delete opengl;
			m_instance = nullptr;
		}
	}

	OpenGL* OpenGL::m_instance = nullptr;

	static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
	                           const void* userParam)
	{
		if (type == GL_DEBUG_TYPE_ERROR)
		{
			error_log("OpenGL", "%s", message);
		}
	}

	OpenGL::OpenGL()
	{
#if USING_OPENGL_ES
		info.name = "OpenGL ES";
#else
		info.name = "OpenGL Core";
#endif
	}

	OpenGL::~OpenGL()
	{
		OpenGL_RenderTarget::release_all();
		delete m_local_ubo;

		extern void destroy_opengl_context(void* context);
		destroy_opengl_context(m_context);
	}

	OpenGL& OpenGL::initialize(Window* window)
	{
		extern void* create_opengl_context(Window * window);
		m_context = create_opengl_context(window);

#if USING_OPENGL_CORE
		glewExperimental = GL_TRUE;

		if (glewInit() != GLEW_OK)
		{
			error_log("OpenGL", "Cannot init glew!");
		}
#endif

#if TRINEX_DEBUG_BUILD
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

		info.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		initialize_ubo();

		glEnable(GL_SCISSOR_TEST);

		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &m_uniform_alignment);
		return *this;
	}

	void* OpenGL::context()
	{
		return m_context;
	}

	OpenGL& OpenGL::prepare_render()
	{
		if (OPENGL_API->m_state.pipeline)
		{
			if (OPENGL_API->m_state.pipeline->m_local_parameters.has_parameters())
			{
				m_local_ubo->bind(OPENGL_API->m_state.pipeline->m_local_parameters.bind_index());
			}
		}
		return *this;
	}

	OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		prepare_render();
		indices_offset *= (m_state.index_buffer->m_format == GL_UNSIGNED_SHORT ? 2 : 4);
		glDrawElementsBaseVertex(OPENGL_API->m_state.pipeline->m_topology, indices_count, m_state.index_buffer->m_format,
		                         reinterpret_cast<void*>(indices_offset), vertices_offset);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw(size_t vertex_count, size_t vertices_offset)
	{
		prepare_render();
		glDrawArrays(OPENGL_API->m_state.pipeline->m_topology, vertices_offset, vertex_count);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		prepare_render();
		glDrawArraysInstanced(OPENGL_API->m_state.pipeline->m_pipeline, vertices_offset, vertex_count, instances);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances)
	{
		prepare_render();
		glDrawElementsInstancedBaseVertex(OPENGL_API->m_state.pipeline->m_topology, indices_count, m_state.index_buffer->m_format,
		                                  reinterpret_cast<void*>(indices_offset), instances, vertices_offset);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::reset_state()
	{
		new (&m_state) OpenGL_State();
		return *this;
	}

	OpenGL& OpenGL::submit()
	{
		m_local_ubo->submit();
		return *this;
	}

	OpenGL& OpenGL::push_debug_stage(const char* stage, const Color& color)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, stage);
		return *this;
	}

	OpenGL& OpenGL::pop_debug_stage()
	{
		glPopDebugGroup();
		return *this;
	}

	void OpenGL::reset_samplers()
	{
		for (BindingIndex unit : m_sampler_units)
		{
			glBindSampler(unit, 0);
		}

		m_sampler_units.clear();
	}
}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::RHI*)
{
	return new Engine::OpenGL();
}
