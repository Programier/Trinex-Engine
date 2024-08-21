#pragma once
#include <Core/executable_object.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>


namespace Engine
{
	struct OpenGL_Shader : public RHI_DefaultDestroyable<RHI_Shader> {
		GLuint m_id = 0;

		~OpenGL_Shader();
		virtual GLuint type() = 0;
	};

	struct OpenGL_VertexShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};

	struct OpenGL_TesselationControlShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};

	struct OpenGL_TesselationShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};

	struct OpenGL_GeometryShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};

	struct OpenGL_FragmentShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};


	template<typename Func>
	struct OpenGL_StateCommand : public ExecutableObject {
		Func m_func;

		OpenGL_StateCommand(Func func) : m_func(func)
		{}

		int_t execute() override
		{
			m_func();
			return 0;
		}
	};

	struct OpenGL_Pipeline : public RHI_DefaultDestroyable<RHI_Pipeline> {
		Vector<ExecutableObject*> m_apply_state;
		MaterialScalarParametersInfo m_global_parameters;
		MaterialScalarParametersInfo m_local_parameters;

		GLuint m_pipeline = 0;
		GLuint m_topology = 0;
		GLuint m_vao      = 0;

		void init_pipeline_shader(Shader* shader, GLbitfield stage);
		void init(const Pipeline* pipeline);
		void bind() override;
		~OpenGL_Pipeline();
	};
}// namespace Engine
