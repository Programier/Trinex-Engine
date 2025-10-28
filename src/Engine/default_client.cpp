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

	trinex_implement_pipeline(HelloTriangle, "[shaders_dir]:/TrinexEngine/trinex/graphics/hello_triangle.slang") {}

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
		auto rtv = viewport->rhi_rtv();

		auto ctx = rhi->context();

		auto target = static_cast<WindowRenderViewport*>(viewport)->rhi_swapchain()->as_texture();
		ctx->barrier(target, RHIAccess::TransferDst);
		ctx->clear_rtv(rtv, 1.f, 0.f, 0.f, 1.f);

		ctx->barrier(target, RHIAccess::RTV);
		ctx->viewport(RHIViewport(viewport->size()));
		ctx->scissor(RHIScissor(viewport->size()));
		ctx->bind_render_target1(rtv, nullptr);
		ctx->bind_pipeline(HelloTriangle::instance()->rhi_pipeline());
		ctx->draw(3, 0);

		auto handle = ctx->end();

		rhi->submit(handle);
		handle->release();
		ctx->begin();
		viewport->rhi_present();

		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
