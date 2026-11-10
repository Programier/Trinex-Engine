#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/math/vector.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class RHIObject;
	class RHIBuffer;
	class RHITexture;
	class RHIContext;
} // namespace Trinex

namespace Trinex
{
	template<typename T>
	using RGArray = Array<T, FrameAllocator<T>>;

	template<typename Key>
	using RGSet = Set<Key, Hash<Key>, std::equal_to<Key>, FrameAllocator<Key>>;

	template<typename Key, typename Value>
	using RGMap = Map<Key, Value, Hash<Key>, std::equal_to<Key>, FrameAllocator<Pair<const Key, Value>>>;

	template<typename T>
	static inline T* rg_allocate()
	{
		return FrameAllocator<T>::allocate(1);
	}

	class RenderGraph;
	class RGResource;
	class RGPass;

	class trinex_api RGTexture
	{
	private:
		void* m_identity = nullptr;

		explicit RGTexture(void* identity) : m_identity(identity) {}
		friend class RenderGraph;

	public:
		RGTexture() = default;
		inline bool is_valid() const { return m_identity != nullptr; }
	};

	class trinex_api RGBuffer
	{
	private:
		void* m_identity = nullptr;

		explicit RGBuffer(void* identity) : m_identity(identity) {}
		friend class RenderGraph;

	public:
		RGBuffer() = default;
		inline bool is_valid() const { return m_identity != nullptr; }
	};

	struct trinex_api RGTextureDesc
	{
		RHITextureType type     = RHITextureType::Texture2D;
		RHISurfaceFormat format = RHISurfaceFormat::RGBA8;
		Vector3u size           = {0, 0, 1};
		RHITextureFlags flags   = RHITextureFlags::ColorAttachment;
	};

	struct trinex_api RGBufferDesc
	{
		u32 size             = 0;
		RHIBufferFlags flags = RHIBufferFlags::Undefined;
	};

	struct trinex_api RGLifetime
	{
		static constexpr u32 undefined = ~0u;
		u32 first_pass                 = undefined;
		u32 last_pass                  = undefined;

		inline bool is_valid() const { return first_pass != undefined; }
	};

	class trinex_api RGContext
	{
	private:
		RenderGraph* m_graph;
		RHIContext* m_context;

		RGContext(RenderGraph* graph, RHIContext* context) : m_graph(graph), m_context(context) {}
		friend class RGPass;

	public:
		RHITexture* texture(RGTexture texture) const;
		RHIBuffer* buffer(RGBuffer buffer) const;
		inline RHIContext* rhi() const { return m_context; }
		inline operator RHIContext*() const { return m_context; }
	};

	class trinex_api RenderGraph
	{
	public:
		class Plugin
		{
		public:
			virtual Plugin& on_frame_begin(RenderGraph* graph, RHIContext* ctx);
			virtual Plugin& on_frame_end(RenderGraph* graph, RHIContext* ctx);

			virtual Plugin& on_pass_begin(RGPass* pass, RHIContext* ctx);
			virtual Plugin& on_pass_end(RGPass* pass, RHIContext* ctx);
		};

	private:
		struct Node
		{
			enum class State : u8
			{
				Unvisited,
				Visiting,
				Scheduled,
			};

			RGPass* pass;
			RGArray<Node*> dependencies;
			State state = State::Unvisited;

			inline Node() : pass(nullptr) { dependencies.reserve(8); }

			inline bool is_executed() const;
			inline bool is_empty() const;
			inline const char* name() const;

			static inline Node* create() { return trx_stack_new Node(); }
		};

		RGMap<void*, RGResource*> m_resource_map;
		RGSet<void*> m_outputs;
		RGArray<Plugin*> m_plugins;
		RGArray<RGPass*> m_passes;
		RGArray<RGPass*> m_execution_plan;
		bool m_is_compiled = false;

	private:
		Node* build_graph(RGPass* writer);
		Node* build_graph(RGResource* resource);
		Node* build_graph();
		bool schedule_node(Node* node);
		bool validate_graph() const;
		void invalidate();

		RGResource* create_resource(RHITexture* texture, RGPass* writer = nullptr);
		RGResource* create_resource(RHIBuffer* buffer, RGPass* writer = nullptr);
		RGResource* create_resource(const RGTextureDesc& desc, RGPass* writer = nullptr);
		RGResource* create_resource(const RGBufferDesc& desc, RGPass* writer = nullptr);

		RGResource* find_resource(void* identity);
		RGResource* find_resource(RHITexture* texture);
		RGResource* find_resource(RHIBuffer* buffer);
		RGResource* find_resource(RGTexture texture);
		RGResource* find_resource(RGBuffer buffer);
		RGResource* find_resource(RGPass* pass);
		RGResource* find_resource(RGTexture texture, RGPass* writer);
		RGResource* find_resource(RGBuffer buffer, RGPass* writer);

	public:
		RenderGraph();
		// Imported resources are owned outside of the graph. Their initial state remains
		// tracked by RHI until RHI exposes explicit state ownership to RenderGraph.
		RGTexture import(RHITexture* texture);
		RGBuffer import(RHIBuffer* buffer);
		RGTexture create_texture(const RGTextureDesc& desc);
		RGBuffer create_buffer(const RGBufferDesc& desc);
		RenderGraph& add_output(RGTexture texture);
		RenderGraph& add_output(RGBuffer buffer);
		RHITexture* texture(RGTexture texture);
		RHIBuffer* buffer(RGBuffer buffer);
		RGLifetime lifetime(RGTexture texture) const;
		RGLifetime lifetime(RGBuffer buffer) const;
		RGPass& add_pass(const char* name = "Unnamed pass");
		bool compile();
		bool execute(RHIContext* context);

		inline const RGArray<Plugin*> plugins() const { return m_plugins; }
		inline const RGArray<RGPass*>& execution_plan() const { return m_execution_plan; }
		inline RenderGraph& add_plugin(Plugin* plugin)
		{
			m_plugins.push_back(plugin);
			return *this;
		}

		template<typename PluginType, typename... Args>
		PluginType* create_plugin(Args&&... args)
		{
			PluginType* plugin = FrameAllocator<PluginType>::allocate(1);
			new (plugin) PluginType(std::forward<Args>(args)...);
			m_plugins.push_back(plugin);
			return plugin;
		}

		friend RGPass;
	};

	class trinex_api RGPass
	{
	public:
		struct ResourceRef
		{
			RGResource* resource;
			RHIAccess access;
			RHIAccess initial;

			inline ResourceRef(RGResource* resource, RHIAccess access, RHIAccess initial = RHIAccess::Undefined)
			    : resource(resource), access(access), initial(initial)
			{}

			RHITexture* as_texture() const;
			RHIBuffer* as_buffer() const;
			RGPass* as_pass() const;
		};

		class Task
		{
		public:
			virtual void execute(RGContext& ctx) = 0;
			virtual ~Task() {}
			friend RGPass;
		};

	private:
		template<typename Callable>
		class LambdaTask final : public Task
		{
		private:
			std::decay_t<Callable> m_callable;

		public:
			explicit LambdaTask(Callable&& fn) : m_callable(std::forward<Callable>(fn)) {}
			void execute(RGContext& ctx) override { m_callable(ctx); }
		};

		class RenderGraph* m_graph;
		const char* m_name;
		RenderGraph::Node* m_node;

		RGArray<const ResourceRef*> m_resources;
		RGArray<Task*> m_tasks;
		bool m_never_cull = false;

		RGPass(class RenderGraph* graph, const char* name = "Unnamed pass");
		RGPass& add_resource(RGResource* resource, RHIAccess access, RHIAccess initial);
		RGPass& execute(RHIContext* ctx);

	public:
		RGPass& add_dependency(RGPass* pass);
		// `access` is the state required after this pass. If `initial` is provided,
		// it is the state required while the task executes. This permits a pass to
		// read a resource and leave it ready for its next writer.
		RGPass& add_resource(RGTexture texture, RHIAccess access, RHIAccess initial = RHIAccess::Undefined);
		RGPass& add_resource(RGBuffer buffer, RHIAccess access, RHIAccess initial = RHIAccess::Undefined);
		RGPass& read(RGTexture texture, RHIAccess access = RHIAccess::SRVGraphics);
		RGPass& read(RGBuffer buffer, RHIAccess access = RHIAccess::SRVGraphics);
		RGPass& write(RGTexture texture, RHIAccess access);
		RGPass& write(RGBuffer buffer, RHIAccess access);
		RGPass& side_effect();

		template<typename TaskType, typename... Args>
		RGPass& add_task(Args&&... args)
		{
			static_assert(std::is_base_of_v<Task, TaskType>, "TaskType must be derived from Pass::Task!");
			TaskType* task = FrameAllocator<TaskType>::allocate(1);
			new (task) TaskType(std::forward<Args>(args)...);
			m_tasks.emplace_back(task);
			m_graph->invalidate();
			return *this;
		}

		template<typename Callable, typename... Args>
		RGPass& add_func(Callable&& callable, Args&&... args)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				add_task<LambdaTask<Callable>>(std::forward<Callable>(callable));
			}
			else
			{
				auto wrapper = [func = std::forward<Callable>(callable), ... args = std::forward<Args>(args)](
				                       RHIContext* ctx) mutable { func(ctx, std::forward<Args>(args)...); };

				return add_task<LambdaTask<decltype(wrapper)>>(std::move(wrapper));
			}

			return *this;
		}

		inline class RenderGraph* graph() const { return m_graph; }
		inline const char* name() const { return m_name; }
		inline const RGArray<const ResourceRef*>& resources() const { return m_resources; }
		inline const RGArray<Task*>& tasks() const { return m_tasks; }
		inline bool is_empty() const { return m_tasks.empty(); }
		inline bool is_never_culled() const { return m_never_cull; }

		friend class RenderGraph;
	};

	inline bool RenderGraph::Node::is_executed() const
	{
		return pass == nullptr;
	}

	inline bool RenderGraph::Node::is_empty() const
	{
		return pass->is_empty();
	}

	inline const char* RenderGraph::Node::name() const
	{
		return pass->name();
	}
} // namespace Trinex
