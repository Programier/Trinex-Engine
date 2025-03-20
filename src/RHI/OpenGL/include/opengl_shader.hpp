#pragma once
#include <Core/etl/vector.hpp>
#include <Core/task.hpp>
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

	struct OpenGL_ComputeShader : public OpenGL_Shader {
		virtual GLuint type() override;
	};


	template<typename Func>
	struct OpenGL_StateCommand : public Task<OpenGL_StateCommand<Func>> {
		Func m_func;

		OpenGL_StateCommand(Func func) : m_func(func) {}

		void execute() override { m_func(); }
	};

	struct OpenGL_Pipeline : public RHI_DefaultDestroyable<RHI_Pipeline> {
		GLuint m_pipeline = 0;

		void init_pipeline_shader(Shader* shader, GLbitfield stage);
		void bind() override;
		~OpenGL_Pipeline();
	};

	struct OpenGL_GraphicsPipeline : public OpenGL_Pipeline {
		Vector<TaskInterface*> m_apply_state;
		GLuint m_topology = 0;
		GLuint m_vao      = 0;

		void init(const GraphicsPipeline* pipeline);
		void bind() override;
		~OpenGL_GraphicsPipeline();
	};

	struct OpenGL_ComputePipeline : public OpenGL_Pipeline {
		void init(const ComputePipeline* pipeline);
	};
}// namespace Engine
