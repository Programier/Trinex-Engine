#pragma once
#include <Core/callback.hpp>
#include <Core/memory.hpp>
#include <Core/name.hpp>
#include <Core/task.hpp>
#include <Engine/Render/batched_primitives.hpp>

namespace Engine
{
	class SceneRenderer;
	class RenderViewport;
	class SceneComponent;

	class ENGINE_EXPORT RenderPass
	{
		declare_struct(RenderPass, void);

	public:
		static constexpr size_t command_alignment = 16;
		using FunctionCallback                    = void(RenderViewport*, RenderPass*);
		CallBacks<FunctionCallback> on_render;

	private:
		SceneRenderer* m_renderer = nullptr;
		RenderPass* m_next        = nullptr;

		Buffer m_commands;
		size_t m_allocated = 0;

		inline byte* allocate(size_t size)
		{
			size_t start = m_allocated;
			m_allocated += align_up(size, command_alignment);

			if (m_allocated > m_commands.size())
			{
				m_commands.resize(m_allocated);
			}

			return m_commands.data() + start;
		}

	protected:
		RenderPass();
		virtual ~RenderPass();

	public:
		delete_copy_constructors(RenderPass);
		SceneRenderer* scene_renderer() const;
		RenderPass* next() const;
		Refl::RenderPassInfo* info() const;

		virtual Refl::Struct* struct_instance() const;

		virtual bool is_empty() const;
		virtual RenderPass& clear();
		virtual RenderPass& render(RenderViewport*);

		RenderPass& draw(size_t vertices_count, size_t vertices_offset);
		RenderPass& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset);
		RenderPass& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances);
		RenderPass& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances);
		RenderPass& bind_material(class MaterialInterface* material, SceneComponent* component = nullptr);
		RenderPass& bind_vertex_buffer(class VertexBuffer* buffer, byte stream, size_t offset = 0);
		RenderPass& bind_index_buffer(class IndexBuffer* buffer, size_t offset = 0);

		template<typename Dst, typename Src>
		auto assign(Dst& dst, Src&& src)
		{
			return add_callabble([&]() { dst = std::forward<Src>(src); });
		}

		template<typename Callable>
		Callable* add_callabble(Callable&& callable)
		{
			static_assert(alignof(Callable) <= command_alignment);
			return new (allocate(sizeof(Callable))) Callable(std::forward<Callable>(callable));
		}

		friend class SceneRenderer;
	};
#define trinex_render_pass(name, parent)                                                                                         \
    declare_struct(name, parent);                                                                                                \
                                                                                                                                 \
public:                                                                                                                          \
    virtual Engine::Refl::Struct* struct_instance() const override;                                                              \
                                                                                                                                 \
private:                                                                                                                         \
    void static_initialize_render_pass()

	class ENGINE_EXPORT ClearPass : public RenderPass
	{
		trinex_render_pass(ClearPass, RenderPass);

	public:
		bool is_empty() const override;
		ClearPass& render(RenderViewport*) override;
	};

	class ENGINE_EXPORT DepthPass : public RenderPass
	{
		trinex_render_pass(DepthPass, RenderPass);
	};

	class ENGINE_EXPORT ShadowPass : public RenderPass
	{
		trinex_render_pass(DepthPass, RenderPass);
	};

	class ENGINE_EXPORT GeometryPass : public RenderPass
	{
		trinex_render_pass(DepthPass, RenderPass);

	public:
		GeometryPass& render(RenderViewport*) override;
	};

	class ENGINE_EXPORT ForwardPass : public RenderPass
	{
		trinex_render_pass(ForwardPass, RenderPass);
	};

	class ENGINE_EXPORT DeferredLightingPass : public RenderPass
	{
		trinex_render_pass(DeferredLightingPass, RenderPass);

	public:
		bool is_empty() const override;
		DeferredLightingPass& render(RenderViewport*) override;
	};

	class ENGINE_EXPORT TransparencyPass : public RenderPass
	{
		trinex_render_pass(TransparencyPass, RenderPass);
	};

	class ENGINE_EXPORT PostProcessPass : public RenderPass
	{
		trinex_render_pass(PostProcessPass, RenderPass);

	public:
		bool is_empty() const override;
		PostProcessPass& render(RenderViewport*) override;
	};

	class ENGINE_EXPORT OverlayPass : public RenderPass
	{
		trinex_render_pass(OverlayPass, RenderPass);

		OverlayPass& copy_view_texture(ViewMode mode);

	public:
		BatchedLines lines;
		BatchedTriangles triangles;

		bool is_empty() const override;
		OverlayPass& clear() override;
		OverlayPass& render(RenderViewport*) override;
	};
}// namespace Engine
