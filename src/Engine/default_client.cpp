#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/types/color.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/pipeline.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	class HelloTriangle : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(HelloTriangle, GlobalGraphicsPipeline);
	};

	trinex_implement_pipeline(HelloTriangle, "[shaders_dir]:/TrinexEngine/trinex/graphics/hello_triangle.slang",
	                          ShaderType::BasicGraphics)
	{}

	DefaultClient::DefaultClient() {}

	DefaultClient::~DefaultClient() {}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		render_thread()->call([viewport]() {
			rhi->viewport(RHIViewport(viewport->size()));
			rhi->scissor(RHIScissors(viewport->size()));

			viewport->rhi_clear_color(LinearColor(0, 0, 0, 1));
			viewport->rhi_bind();
			HelloTriangle::instance()->rhi_bind();
			rhi->draw(6, 0);
			viewport->rhi_present();
		});
		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
