#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/types/color.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	class HelloTriangle : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(HelloTriangle, GlobalGraphicsPipeline);
	};

	trinex_implement_pipeline(HelloTriangle, "[shaders]:/TrinexEngine/trinex/graphics/hello_triangle.slang") {}

	DefaultClient::DefaultClient()
	{
		info_log("Trinex Engine", "Creating default client!");
	}

	DefaultClient::~DefaultClient() {}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		viewport->rhi_present();

		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
