#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class RHIObject;
	class RHIBuffer;
	class RHITexture;
	class RHIContext;
}// namespace Engine

namespace Engine::RenderGraph
{
	template<typename T>
	using RGVector = Vector<T, FrameAllocator<T>>;

	template<typename Key>
	using RGSet = Set<Key, Hash<Key>, std::equal_to<Key>, FrameAllocator<Key>>;

	template<typename Key, typename Value>
	using RGMap = Map<Key, Value, Hash<Key>, std::equal_to<Key>, FrameAllocator<Pair<const Key, Value>>>;

	template<typename T>
	static FORCE_INLINE T* rg_allocate()
	{
		return FrameAllocator<T>::allocate(1);
	}

	class Graph;
	class Resource;
	class Pass;

	class ENGINE_EXPORT Graph
	{
	public:
		class Plugin
		{
		public:
			virtual Plugin& on_frame_begin(Graph* graph);
			virtual Plugin& on_frame_end(Graph* graph);

			virtual Plugin& on_pass_begin(Pass* pass);
			virtual Plugin& on_pass_end(Pass* pass);
		};

	private:
		struct Node {
			Pass* pass;
			RGVector<Node*> dependencies;

			inline Node() : pass(nullptr) { dependencies.reserve(8); }

			inline bool is_executed() const;
			inline bool is_empty() const;
			inline const char* name() const;

			static inline Node* create() { return trx_stack_new Node(); }
		};

		RGMap<RHIObject*, Resource*> m_resource_map;
		RGSet<RHIObject*> m_outputs;
		RGVector<Pass*> m_passes;
		RGVector<Plugin*> m_plugins;

	private:
		Node* build_graph(Pass* writer);
		Node* build_graph(Resource* resource);
		Node* build_graph();
		void execute_node(Node* node, RHIContext* ctx);

		Resource* find_resource(RHIObject* texture);
		Resource* find_resource(RHITexture* texture);
		Resource* find_resource(RHIBuffer* buffer);
		Resource* find_resource(RHITexture* texture, Pass* writer);
		Resource* find_resource(RHIBuffer* buffer, Pass* writer);

	public:
		Graph();
		Graph& add_output(RHITexture* texture);
		Graph& add_output(RHIBuffer* buffer);
		Pass& add_pass(const char* name = "Unnamed pass");
		bool execute();

		inline const RGVector<Pass*> passes() const { return m_passes; }
		inline const RGVector<Plugin*> plugins() const { return m_plugins; }
		inline Graph& add_plugin(Plugin* plugin)
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
		friend Pass;
	};


	class ENGINE_EXPORT Pass
	{
	public:
		struct Resource {
			RenderGraph::Resource* resource;
			RHIAccess access;

			inline Resource(RenderGraph::Resource* resource, RHIAccess access) : resource(resource), access(access) {}
		};

		class Task
		{
		public:
			virtual void execute() = 0;
			virtual ~Task() {}
			friend Pass;
		};

	private:
		template<typename Callable>
		class LambdaTask final : public Task
		{
		private:
			std::decay_t<Callable> m_callable;

		public:
			explicit LambdaTask(Callable&& fn) : m_callable(std::forward<Callable>(fn)) {}
			void execute() override { m_callable(); }
		};

		class Graph* m_graph;
		const char* m_name;
		Graph::Node* m_node;

		RGVector<Resource*> m_resources;
		RGVector<Pass*> m_dependencies;
		RGVector<Task*> m_tasks;

		Pass(class Graph* graph, const char* name = "Unnamed pass");
		Pass& add_resource(RenderGraph::Resource* resource, RHIAccess access);
		Pass& execute(RHIContext* ctx);

	public:
		Pass& add_resource(RHITexture* texture, RHIAccess access);
		Pass& add_resource(RHIBuffer* buffer, RHIAccess access);

		inline Pass& add_dependency(Pass* dependency)
		{
			m_dependencies.push_back(dependency);
			return *this;
		}

		inline Pass& add_pass(const char* name = "Unnamed pass")
		{
			Pass& pass = m_graph->add_pass(name);
			add_dependency(&pass);
			return pass;
		}

		template<typename TaskType, typename... Args>
		Pass& add_task(Args&&... args)
		{
			static_assert(std::is_base_of_v<Task, TaskType>, "TaskType must be derived from Pass::Task!");
			TaskType* task = FrameAllocator<TaskType>::allocate(1);
			new (task) TaskType(std::forward<Args>(args)...);
			m_tasks.emplace_back(task);
			return *this;
		}

		template<typename Callable, typename... Args>
		Pass& add_func(Callable&& callable, Args&&... args)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				add_task<LambdaTask<Callable>>(std::forward<Callable>(callable));
			}
			else
			{
				auto wrapper = [func = std::forward<Callable>(callable), ... args = std::forward<Args>(args)]() mutable {
					func(std::forward<Args>(args)...);
				};

				return add_task<LambdaTask<decltype(wrapper)>>(std::move(wrapper));
			}

			return *this;
		}

		inline class Graph* graph() const { return m_graph; }
		inline const char* name() const { return m_name; }
		inline const RGVector<Resource*>& resources() const { return m_resources; }
		inline const RGVector<Pass*>& dependencies() const { return m_dependencies; }
		inline const RGVector<Task*>& tasks() const { return m_tasks; }
		inline bool is_empty() const { return m_tasks.empty(); }

		friend class Graph;
	};

	inline bool Graph::Node::is_executed() const
	{
		return pass == nullptr;
	}

	inline bool Graph::Node::is_empty() const
	{
		return pass->is_empty();
	}

	inline const char* Graph::Node::name() const
	{
		return pass->name();
	}
}// namespace Engine::RenderGraph
