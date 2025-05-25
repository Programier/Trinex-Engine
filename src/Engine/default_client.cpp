#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/types/color.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	class HelloTriangle : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(HelloTriangle, GlobalGraphicsPipeline);
	};

	trinex_implement_pipeline(HelloTriangle, "[shaders_dir]:/TrinexEngine/trinex/graphics/hello_triangle.slang",
	                          ShaderType::BasicGraphics)
	{}

	DefaultClient::DefaultClient()
	{
		render_thread()->call([this]() {
			Vector2f positions[3] = {
			        {0.f, 0.5f},
			        {0.5f, -0.5f},
			        {-0.5f, -0.5f},
			};

			m_vertex_buffer =
			        rhi->create_buffer(sizeof(positions), reinterpret_cast<byte*>(positions), BufferCreateFlags::VertexBuffer);
			rhi->barrier(m_vertex_buffer, RHIAccess::VertexBuffer);
		});
	}

	DefaultClient::~DefaultClient()
	{
		render_thread()->call([buffer = m_vertex_buffer]() { buffer->release(); });
	}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		render_thread()->call([viewport, this]() {
			rhi->viewport(ViewPort({}, viewport->size()));
			rhi->scissor(Scissor({}, viewport->size()));

			viewport->rhi_clear_color(LinearColor(0, 0, 0, 1));
			viewport->rhi_bind();
			HelloTriangle::instance()->rhi_bind();
			rhi->bind_vertex_buffer(m_vertex_buffer, 0, sizeof(Vector2f), 0);
			rhi->draw(3, 0);
			viewport->rhi_present();
		});
		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
