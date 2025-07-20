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
	struct RHIObject;
	struct RHIBuffer;
	struct RHITexture;
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

	class ENGINE_EXPORT Pass
	{
	public:
		enum Type
		{
			Graphics = 0,
			Compute  = 1,
			Transfer = 2,
		};

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

		struct Flags {
			enum Enum : byte
			{
				Undefined  = 0,
				IsExecuted = BIT(0),
				IsVisited  = BIT(1),
			};
			trinex_bitfield_enum_struct(Flags, byte);
		};

		class Graph* m_graph;
		const char* m_name;
		void* m_node;

		RGVector<Resource*> m_resources;
		RGVector<Pass*> m_dependencies;
		RGVector<Task*> m_tasks;
		Type m_type;
		Flags m_flags;

		Pass(class Graph* graph, Type type, const char* name = "Unnamed pass");
		Pass& add_resource(RenderGraph::Resource* resource, RHIAccess access);

	public:
		Pass& add_resource(RHITexture* texture, RHIAccess access);
		Pass& add_resource(RHIBuffer* buffer, RHIAccess access);

		inline Pass& add_dependency(Pass* dependency)
		{
			m_dependencies.push_back(dependency);
			return *this;
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
		inline Type type() const { return m_type; }
		inline const char* name() const { return m_name; }
		inline bool is_visited() const { return (m_flags & Flags::IsVisited) == Flags::IsVisited; }
		inline const RGVector<Resource*>& resources() const { return m_resources; }
		inline const RGVector<Pass*>& dependencies() const { return m_dependencies; }
		inline const RGVector<Task*>& tasks() const { return m_tasks; }
		inline bool is_empty() const { return m_tasks.empty(); }

		inline Pass& add_flags(Flags flags)
		{
			m_flags |= flags;
			return *this;
		}

		inline Pass& execute()
		{
			m_flags |= Flags::IsExecuted;
			for (Task* task : m_tasks)
			{
				task->execute();
				task->~Task();
			}
			m_tasks.clear();
			return *this;
		}

		friend class Graph;
	};

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

			uint32_t depth       = 0;
			uint32_t phase_score = 0;

			inline Node() { dependencies.reserve(8); }
			inline void execute() { pass->execute(); }
			inline bool is_executed() const { return pass->m_flags & Pass::Flags::IsExecuted; }
			inline bool is_visited() const { return pass->m_flags & Pass::Flags::IsVisited; }
			inline bool is_empty() const { return pass->is_empty(); }
			inline Pass::Type type() const { return pass->type(); }
			inline const char* name() const { return pass->name(); }

			static inline Node* create() { return new (rg_allocate<Node>()) Node(); }
		};

		struct ResourceEntry {
			Resource* first = nullptr;
			Resource* last  = nullptr;
		};

		RGMap<RHIObject*, ResourceEntry> m_resource_map;
		RGSet<Resource*> m_outputs;
		RGVector<Pass*> m_passes;
		RGVector<Plugin*> m_plugins;

	private:
		Graph& build_graph(Pass* writer, Node* owner);
		Graph& build_graph(Resource* resource, Node* owner);
		Graph& build_output(Resource* resource, Node* owner);
		Node* build_graph();
		void execute_node(Node* node);

		ResourceEntry* find_resource(RHIObject* object);
		ResourceEntry* find_resource(RHITexture* texture);
		ResourceEntry* find_resource(RHIBuffer* buffer);

		inline Resource* readable_resource(RHITexture* texture) { return find_resource(texture)->last; }
		inline Resource* readable_resource(RHIBuffer* buffer) { return find_resource(buffer)->last; }

		Resource* writable_resource(RHITexture* texture, Pass* writer);
		Resource* writable_resource(RHIBuffer* buffer, Pass* writer);

	public:
		Graph();
		Graph& add_output(RHITexture* texture);
		Graph& add_output(RHIBuffer* buffer);
		Pass& add_pass(Pass::Type type, const char* name = "Unnamed pass");
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
}// namespace Engine::RenderGraph
