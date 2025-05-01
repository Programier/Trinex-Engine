#include <Core/definitions.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/struct.hpp>
#include <Window/config.hpp>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_render_target.hpp>
#include <opengl_resource.hpp>
#include <opengl_sampler.hpp>
#include <opengl_shader.hpp>

namespace Engine
{
	namespace TRINEX_RHI
	{
		using OPENGL = OpenGL;
	}

	trinex_implement_struct_default_init(Engine::TRINEX_RHI::OPENGL, 0);

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

	OpenGL& OpenGL::on_draw()
	{
		m_local_ubo->bind();
		return *this;
	}

	OpenGL& OpenGL::on_dispatch()
	{
		m_local_ubo->bind();
		return *this;
	}

	OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		on_draw();
		indices_offset *= (m_state.index_type == GL_UNSIGNED_SHORT ? 2 : 4);
		glDrawElementsBaseVertex(static_cast<OpenGL_GraphicsPipeline*>(m_state.pipeline)->m_topology, indices_count,
		                         m_state.index_type, reinterpret_cast<void*>(indices_offset), vertices_offset);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw(size_t vertex_count, size_t vertices_offset)
	{
		on_draw();
		glDrawArrays(static_cast<OpenGL_GraphicsPipeline*>(m_state.pipeline)->m_topology, vertices_offset, vertex_count);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		on_draw();
		glDrawArraysInstanced(static_cast<OpenGL_GraphicsPipeline*>(m_state.pipeline)->m_topology, vertices_offset, vertex_count,
		                      instances);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances)
	{
		on_draw();
		glDrawElementsInstancedBaseVertex(static_cast<OpenGL_GraphicsPipeline*>(m_state.pipeline)->m_topology, indices_count,
		                                  m_state.index_type, reinterpret_cast<void*>(indices_offset), instances,
		                                  vertices_offset);
		reset_samplers();
		return *this;
	}

	OpenGL& OpenGL::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		on_dispatch();
		glDispatchCompute(group_x, group_y, group_z);
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
		glFlush();
		return *this;
	}

	OpenGL& OpenGL::push_debug_stage(const char* stage, const LinearColor& color)
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

	OpenGL& OpenGL::bind_srv(RHI_ShaderResourceView* view, byte slot, RHI_Sampler* sampler)
	{
		static_cast<OpenGL_SRV*>(view)->bind(slot, static_cast<OpenGL_Sampler*>(sampler));
		return *this;
	}

	OpenGL& OpenGL::bind_uav(RHI_UnorderedAccessView* view, byte slot)
	{
		static_cast<OpenGL_UAV*>(view)->bind(slot);
		return *this;
	}

}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::RHI*)
{
	return new Engine::OpenGL();
}
