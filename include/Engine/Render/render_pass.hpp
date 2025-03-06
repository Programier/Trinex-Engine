#pragma once
#include <Core/memory.hpp>
#include <Core/name.hpp>
#include <Core/task.hpp>
#include <Engine/Render/batched_primitives.hpp>

namespace Engine
{
	class SceneRenderer;
	class RenderViewport;
	class SceneComponent;
	class PrimitiveComponent;
	class MaterialInterface;
	class Pipeline;

	class ENGINE_EXPORT RenderPass
	{
		trinex_declare_struct(RenderPass, void);

	public:
		static constexpr size_t command_alignment = 16;

	private:
		SceneRenderer* m_renderer = nullptr;
		RenderPass* m_owner       = nullptr;
		RenderPass* m_childs      = nullptr;
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

		void initialize_subpass(RenderPass* pass);

	protected:
		RenderPass();
		virtual ~RenderPass();

	public:
		delete_copy_constructors(RenderPass);

		Refl::RenderPassInfo* info() const;
		bool destroy_subpass(RenderPass* pass);

		inline SceneRenderer* scene_renderer() const { return m_renderer; }

		inline RenderPass* next() const { return m_next; }

		inline RenderPass* owner() const { return m_owner; }

		virtual RenderPass& initialize();
		virtual Refl::Struct* struct_instance() const;
		virtual bool is_empty() const;
		virtual RenderPass& clear();
		virtual RenderPass& render(RenderViewport*);
		virtual RenderPass& predraw(PrimitiveComponent* primitive, MaterialInterface* material, Pipeline* pipeline);

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
			using SrcT = std::decay_t<Src>;
			return add_callabble([&dst, src_copy = SrcT(std::forward<Src>(src))]() { dst = src_copy; });
		}

		template<typename Task, typename... Args>
		Task* add_task(Args&&... args)
		{
			static_assert(alignof(Task) <= command_alignment);
			static_assert(std::is_base_of_v<TaskInterface, Task>);

			void* place = allocate(sizeof(Task));
			return new (place) Task(std::forward<Args>(args)...);
		}

		template<typename Callable>
		TaskInterface* add_callabble(Callable&& callable)
		{
			struct CallableTask : public Task<CallableTask> {
				Callable func;

				CallableTask(Callable&& callable) : func(std::forward<Callable>(callable)) {}

				void execute() override { func(); }
			};

			return add_task<CallableTask>(std::forward<Callable>(callable));
		}


		template<typename T = RenderPass, typename... Args, typename = std::enable_if<std::is_base_of_v<RenderPass, T>>>
		T* create_subpass(Args&&... args)
		{
			T* pass = new T(std::forward<Args>(args)...);
			initialize_subpass(pass);
			return pass;
		}


		friend class SceneRenderer;
	};
#define trinex_render_pass(name, parent)                                                                                         \
    trinex_declare_struct(name, parent);                                                                                         \
                                                                                                                                 \
public:                                                                                                                          \
	virtual Engine::Refl::Struct* struct_instance() const override;                                                              \
	friend class Engine::RenderPass;                                                                                             \
	friend class Engine::SceneRenderer;                                                                                          \
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

	class ENGINE_EXPORT GeometryPass : public RenderPass
	{
		trinex_render_pass(DepthPass, RenderPass);

	public:
		GeometryPass& render(RenderViewport*) override;
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
