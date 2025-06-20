#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	struct RHI_Object;
	struct RHI_Buffer;
	struct RHI_Texture;
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

	class ENGINE_EXPORT Resource
	{
	private:
		RGVector<class Pass*> m_readers;
		RGVector<class Pass*> m_writers;
		RGVector<class Pass*> m_read_writers;

		inline Resource() {}

	public:
		inline Resource& add_writer(Pass* pass)
		{
			m_writers.emplace_back(pass);
			return *this;
		}

		inline Resource& add_reader(Pass* pass)
		{
			m_readers.emplace_back(pass);
			return *this;
		}

		inline Resource& add_read_writer(Pass* pass)
		{
			m_read_writers.push_back(pass);
			return *this;
		}

		inline const RGVector<Pass*>& writers() const { return m_writers; }
		inline const RGVector<Pass*>& readers() const { return m_readers; }
		inline const RGVector<Pass*>& read_writers() const { return m_read_writers; }

		friend class Graph;
	};

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
				IsLive     = BIT(1),
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

		Pass(class Graph* graph, Type type, const StringView& name = "Unnamed pass");

		Pass& add_resource(RenderGraph::Resource* resource, RHIAccess access);

	public:
		Pass& add_resource(RHI_Texture* texture, RHIAccess access);
		Pass& add_resource(RHI_Buffer* buffer, RHIAccess access);

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
		inline const RGVector<Resource*>& resources() const { return m_resources; }
		inline const RGVector<Pass*>& dependencies() const { return m_dependencies; }
		inline const RGVector<Task*>& tasks() const { return m_tasks; }

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
	private:
		struct Node {
			Pass* pass;
			RGVector<Node*> dependencies;

			uint32_t depth       = 0;
			uint32_t phase_score = 0;

			inline Node() { dependencies.reserve(8); }
			inline void execute() { pass->execute(); }
			inline bool is_executed() const { return pass->m_flags & Pass::Flags::IsExecuted; }
			inline bool is_live() const { return pass->m_flags & Pass::Flags::IsLive; }
			inline Pass::Type type() const { return pass->type(); }
			inline const char* name() const { return pass->name(); }

			static inline Node* create() { return new (rg_allocate<Node>()) Node(); }
		};

		RGMap<RHI_Object*, Resource*> m_resource_map;
		RGVector<Pass*> m_passes;
		RGSet<Resource*> m_outputs;

		Graph& build_graph(Pass* writer, Node* owner);
		Graph& build_graph(Resource* resource, Node* owner);
		Node* build_graph();
		void execute_node(Node* node);

	public:
		Graph();
		Resource& find_resource(RHI_Texture* texture);
		Resource& find_resource(RHI_Buffer* buffer);
		Graph& add_output(RHI_Texture* texture);
		Graph& add_output(RHI_Buffer* buffer);
		Pass& add_pass(Pass::Type type, const StringView& name = "Unnamed pass");
		bool execute();

		class Test;
		friend Pass;
	};
}// namespace Engine::RenderGraph
