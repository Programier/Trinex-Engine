#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/default_client.hpp>
#include <Engine/world.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

#include <Core/base_engine.hpp>
#include <Engine/Render/scene.hpp>
#include <Engine/Render/scene_view.hpp>
#include <Engine/camera_view.hpp>
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
		static void render(RHIContext* ctx, RHIBuffer* uniforms, u32 instances, u32 base = 0)
		{
			auto pipeline = instance();

			ctx->bind_uniform_buffer(uniforms, pipeline->m_scene_view->binding);
			ctx->bind_pipeline(pipeline->rhi_pipeline());
			ctx->draw(RHITopology::TriangleList, 36, 0, instances, base);
		}
	};

	trinex_implement_pipeline(GeometryView, "[shaders]:/TrinexEngine/trinex/debug/geometry_view.slang")
	{
		m_scene_view = find_parameter("scene_view");
	}

	static CameraView camera(Vector2u size)
	{
		float aspect = static_cast<float>(size.x) / static_cast<float>(size.y);

		const float offset = 5;

		Vector3f up = Math::normalize(Math::cross(Vector3f{0.f, -2.f, -2.f}, Vector3f{1.f, 0.f, 0.f}));
		return CameraView::static_perspective({0, offset, offset}, {0, -offset, -offset}, up, Math::radians(75.f), aspect, 1.f,
		                                      1000.f);
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
		RHIBufferFlags flags = RHIBufferFlags::ShaderResource | RHIBufferFlags::TransferDst | RHIBufferFlags::StructuredBuffer |
		                       RHIBufferFlags::UniformBuffer | RHIBufferFlags::UnorderedAccess;

		RHIBuffer* buffer = RHI::instance()->create_buffer(size, flags);
		m_resources.push_back(buffer);

		ctx->barrier(buffer, RHIAccess::TransferDst);
		ctx->update(buffer, data, {.size = size});
		ctx->barrier(buffer, RHIAccess::SRVGraphics);

		return buffer;
	}

	RHIBuffer* DefaultClient::create_buffer(RHIContext* ctx, usize size)
	{
		auto flags        = RHIBufferFlags::UnorderedAccess | RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::IndirectBuffer;
		RHIBuffer* buffer = RHI::instance()->create_buffer(size, flags);
		m_resources.push_back(buffer);
		return buffer;
	}

	RHIDescriptor DefaultClient::create_descriptor(RHIContext* ctx, const void* data, usize size)
	{
		return create_buffer(ctx, data, size)->as_uav(RHIBufferViewType::ByteAddress)->descriptor();
	}

	DefaultClient& DefaultClient::attach(class RenderViewport* viewport)
	{
		m_world       = new_instance<World>("World", this);
		Vector2u size = viewport->size();

		GlobalShaderParameters uniform;
		auto view = SceneView(m_world->scene(), camera(size), size);
		uniform.update(&view);

		auto ctx = RHIContextPool::global_instance()->begin();
		m_scene  = create_buffer(ctx, uniform);

		auto scene = m_world->scene();

		u32 geometry_id  = 0;
		u32 transfrom_id = 0;

		// Step 1: Create Geometry
		{
			RenderScene::Geometry geometry;

			const Vector3f positions[8] = {
			        Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, -1.f, -1.f), Vector3f(1.f, 1.f, -1.f), Vector3f(-1.f, 1.f, -1.f),
			        Vector3f(-1.f, -1.f, 1.f),  Vector3f(1.f, -1.f, 1.f),  Vector3f(1.f, 1.f, 1.f),  Vector3f(-1.f, 1.f, 1.f),
			};

			const u32 indices[36] = {
			        0, 1, 2, 2, 3, 0,// Front
			        1, 5, 6, 6, 2, 1,// Right
			        5, 4, 7, 7, 6, 5,// Back
			        4, 0, 3, 3, 7, 4,// Left
			        3, 2, 6, 6, 7, 3,// Top
			        4, 5, 1, 1, 0, 4 // Bottom
			};

			geometry.vertex_stream.buffer = create_descriptor(ctx, positions, sizeof(positions));
			geometry.vertex_stream.offset = 0;
			geometry.vertex_stream.stride = sizeof(Vector3f);

			geometry.index_stream.buffer = create_descriptor(ctx, indices, sizeof(indices));
			geometry.index_stream.offset = 0;
			geometry.index_stream.stride = sizeof(u32);

			geometry_id = scene->create_geometry(geometry);// id = 0
		}


		for (int i = 0; i < 5; ++i)
		{
			// Step 2: Create transforms
			{
				Matrix4f transform = Math::translate(Vector3f(i * 3, -i * 3, 0));
				transfrom_id       = scene->allocate(sizeof(Matrix4f), &transform);
			}

			// Step 3: Create instances
			{
				scene->create_primitive({
				        .transform = transfrom_id,
				        .geometry  = geometry_id,
				});
			}
		}


		RHIContextPool::global_instance()->end(ctx);
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		// Render
		auto swapchain      = viewport->swapchain();
		const Vector2u size = viewport->size();

		auto ctx = RHIContextPool::global_instance()->begin();
		auto depth =
		        RHITexturePool::global_instance()->acquire(RHISurfaceFormat::D32F, size, RHITextureFlags::DepthStencilAttachment);

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
			for (auto& chunk : m_world->scene()->chunks())
			{
				GeometryView::render(ctx, m_scene, chunk.count, chunk.address);
			}
		}
		ctx->end_rendering();


		ctx->barrier(swapchain->as_texture(), RHIAccess::PresentSrc);
		RHITexturePool::global_instance()->release(depth);
		RHIContextPool::global_instance()->end(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
		RHI::instance()->present(swapchain);
		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Trinex
