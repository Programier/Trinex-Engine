#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/default_client.hpp>
#include <Engine/world.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

#include <Core/base_engine.hpp>
#include <Engine/camera_view.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader_parameters.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class ENGINE_EXPORT GeometryView : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(GeometryView, GlobalGraphicsPipeline);

	private:
		const RHIShaderParameterInfo* m_scene_view;

	public:
		static void render(RHIContext* ctx, RHIBuffer* uniforms)
		{
			auto pipeline = instance();

			ctx->bind_uniform_buffer(uniforms, pipeline->m_scene_view->binding);
			ctx->bind_pipeline(pipeline->rhi_pipeline());
			ctx->draw(RHITopology::TriangleList, 36, 0);
		}
	};

	trinex_implement_pipeline(GeometryView, "[shaders]:/TrinexEngine/trinex/debug/geometry_view.slang")
	{
		m_scene_view = find_parameter("scene_view");
	}

	static CameraView camera(Vector2u size)
	{
		float aspect = static_cast<float>(size.x) / static_cast<float>(size.y);

		Vector3f up = Math::normalize(Math::cross(Vector3f{0.f, -2.f, -2.f}, Vector3f{1.f, 0.f, 0.f}));
		return CameraView::static_perspective({0, 2, 2}, {0, -2, -2}, up, Math::radians(75.f), aspect, 1.f, 20.f);
	}

	DefaultClient::DefaultClient() {}

	DefaultClient::~DefaultClient()
	{
		m_world->owner(nullptr);
		for (RHIObject* object : m_resources)
		{
			object->release();
		}
	}

	RHIBuffer* DefaultClient::create_buffer(RHIContext* ctx, const void* data, usize size)
	{
		RHIBufferFlags flags = RHIBufferFlags::ShaderResource | RHIBufferFlags::TransferDst | RHIBufferFlags::DeviceAddress |
		                       RHIBufferFlags::StructuredBuffer | RHIBufferFlags::UniformBuffer;

		RHIBuffer* buffer = RHI::instance()->create_buffer(size, flags);
		m_resources.push_back(buffer);

		ctx->barrier(buffer, RHIAccess::TransferDst);
		ctx->update(buffer, data, {.size = size});
		ctx->barrier(buffer, RHIAccess::SRVGraphics);

		return buffer;
	}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		m_world       = new_instance<World>("World", this);
		Vector2u size = viewport->size();

		GlobalShaderParameters uniform;
		auto view = SceneView(m_world->scene(), camera(size), size);
		uniform.update(&view);

		auto ctx = RHIContextPool::global_instance()->begin_context();
		{
			m_scene = create_buffer(ctx, uniform);
		}
		RHIContextPool::global_instance()->end_context(ctx);

		auto scene = m_world->scene();
		scene->create_transform();// id = 0, local_to_world

		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		// Update scene
		{
			auto scene    = m_world->scene();
			float seconds = engine_instance->time_seconds();

			float speed = 45.0f;

			Matrix4f rotation = Math::rotate(Math::radians(seconds * speed * 1.0f), Vector3f(1.f, 0.f, 0.f)) *
			                    Math::rotate(Math::radians(seconds * speed * 0.7f), Vector3f(0.f, 1.f, 0.f)) *
			                    Math::rotate(Math::radians(seconds * speed * 1.3f), Vector3f(0.f, 0.f, 1.f));

			scene->update_transform(0, &rotation);
		}

		// Render
		auto swapchain      = viewport->swapchain();
		const Vector2u size = viewport->size();

		auto ctx   = RHIContextPool::global_instance()->begin_context();
		auto depth = RHITexturePool::global_instance()->request_surface(RHISurfaceFormat::D32F, size,
		                                                                RHITextureFlags::DepthStencilAttachment);

		m_world->scene()->flush(ctx);

		ctx->barrier(depth, RHIAccess::TransferDst);
		ctx->barrier(swapchain->as_texture(), RHIAccess::TransferDst);

		ctx->clear_dsv(depth->as_dsv());
		ctx->clear_rtv(swapchain->as_rtv());

		ctx->barrier(depth, RHIAccess::DSV);
		ctx->barrier(swapchain->as_texture(), RHIAccess::RTV);
		{
			RHIDepthStencilState state;
			state.depth.func  = RHICompareFunc::Gequal;
			state.depth.write = true;
			ctx->depth_stencil_state(state);
		}

		ctx->begin_rendering({swapchain->as_rtv(), depth->as_dsv()});
		{
			GeometryView::render(ctx, m_scene);
		}
		ctx->end_rendering();


		ctx->barrier(swapchain->as_texture(), RHIAccess::PresentSrc);
		RHITexturePool::global_instance()->return_surface(depth);
		RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
		RHI::instance()->present(swapchain);
		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Trinex
