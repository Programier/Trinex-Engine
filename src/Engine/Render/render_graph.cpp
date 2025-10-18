#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/memory.hpp>
#include <Engine/Render/render_graph.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>

namespace Engine::RenderGraph
{
	static constexpr size_t s_default_reserve_size = 64;

	class Resource
	{
	public:
		enum Type
		{
			Texture = 0,
			Buffer  = 1,
			Scope   = 2,
		};

	private:
		static constexpr uint64_t s_resource_type_mask    = 0x7ull;
		static constexpr uint64_t s_resource_address_mask = ~s_resource_type_mask;


		Resource* m_next;
		Resource* m_prev;
		Pass* m_writer;

		// Since RHIObject is 8-byte aligned, the address is guaranteed to be aligned to the same value.
		// So, the last 3 bits of the address will be zeros. We will actually use them to store the resource type.
		union
		{
			void* m_resource;
			uint64_t m_resource_address;
		};

	public:
		inline Resource(RHITexture* resource, Pass* writer = nullptr, Resource* prev = nullptr)
		    : m_next(nullptr), m_prev(prev), m_writer(writer), m_resource(resource)
		{
			if (prev)
			{
				prev->m_next = this;
			}
		}

		inline Resource(RHIBuffer* resource, Pass* writer = nullptr, Resource* prev = nullptr)
		    : m_next(nullptr), m_prev(prev), m_writer(writer), m_resource(resource)
		{
			m_resource_address |= Buffer;
			if (prev)
			{
				prev->m_next = this;
			}
		}

		template<typename T>
		inline T* as() const
		{
			return reinterpret_cast<T*>(m_resource_address & s_resource_address_mask);
		}

		inline Pass* writer() const { return m_writer; }
		inline Resource* previous() const { return m_prev; }
		inline Resource* next() const { return m_next; }
		inline Type resource_type() const { return static_cast<Type>(m_resource_address & s_resource_type_mask); }
		friend class Graph;
	};

	Pass::Pass(Graph* graph, const char* name) : m_graph(graph), m_name(name), m_node(nullptr)
	{
		m_resources.reserve(s_default_reserve_size);
		m_tasks.reserve(s_default_reserve_size);
	}

	Pass& Pass::add_resource(RenderGraph::Resource* resource, RHIAccess access)
	{
		m_resources.emplace_back(new (rg_allocate<Resource>()) Resource(resource, access));
		return *this;
	}

	Pass& Pass::add_resource(RHITexture* texture, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(texture, this), access);
		else
			return add_resource(m_graph->find_resource(texture), access);
	}

	Pass& Pass::add_resource(RHIBuffer* buffer, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(buffer, this), access);
		else
			return add_resource(m_graph->find_resource(buffer), access);
	}

	Pass& Pass::execute(RHIContext* ctx)
	{
		for (Pass::Resource* ref : m_resources)
		{
			auto resource = ref->resource;

			switch (resource->resource_type())
			{
				case RenderGraph::Resource::Texture:
				{
					ctx->barrier(resource->as<RHITexture>(), ref->access);
					break;
				}

				case RenderGraph::Resource::Buffer:
				{
					ctx->barrier(resource->as<RHIBuffer>(), ref->access);
					break;
				}

				case RenderGraph::Resource::Scope:
				{
					break;
				}

				default: break;
			}
		}

		for (Task* task : m_tasks)
		{
			task->execute(ctx);
			task->~Task();
		}

		m_tasks.clear();
		return *this;
	}

	Graph::Plugin& Graph::Plugin::on_frame_begin(Graph* graph)
	{
		return *this;
	}

	Graph::Plugin& Graph::Plugin::on_frame_end(Graph* graph)
	{
		return *this;
	}

	Graph::Plugin& Graph::Plugin::on_pass_begin(Pass* pass)
	{
		return *this;
	}

	Graph::Plugin& Graph::Plugin::on_pass_end(Pass* pass)
	{
		return *this;
	}

	Graph::Graph()
	{
		m_resource_map.reserve(s_default_reserve_size);
		m_outputs.reserve(s_default_reserve_size);
	}

	Resource* Graph::find_resource(RHIObject* object)
	{
		auto it = m_resource_map.find(object);
		if (it == m_resource_map.end())
			return nullptr;
		return it->second;
	}

	Resource* Graph::find_resource(RHITexture* texture)
	{
		Resource*& resource = m_resource_map[texture];

		if (resource == nullptr)
			resource = trx_frame_new Resource(texture, nullptr, nullptr);

		return resource;
	}

	Resource* Graph::find_resource(RHIBuffer* buffer)
	{
		Resource*& resource = m_resource_map[buffer];

		if (resource == nullptr)
			resource = trx_frame_new Resource(buffer, nullptr, nullptr);

		return resource;
	}

	Resource* Graph::find_resource(RHITexture* texture, Pass* writer)
	{
		Resource*& resource = m_resource_map[texture];
		resource            = trx_frame_new Resource(texture, writer, resource);
		return resource;
	}

	Resource* Graph::find_resource(RHIBuffer* buffer, Pass* writer)
	{
		Resource*& resource = m_resource_map[buffer];
		resource            = trx_frame_new Resource(buffer, writer, resource);
		return resource;
	}

	Graph& Graph::add_output(RHITexture* texture)
	{
		m_outputs.insert(texture);
		return *this;
	}

	Graph& Graph::add_output(RHIBuffer* buffer)
	{
		m_outputs.insert(buffer);
		return *this;
	}

	Pass& Graph::add_pass(const char* name)
	{
		return *(new (rg_allocate<Pass>()) Pass(this, name));
	}

	Graph::Node* Graph::build_graph(Pass* writer)
	{
		if (writer->m_node)
			return writer->m_node;

		auto node      = Node::create();
		writer->m_node = node;
		node->pass     = writer;

		for (Pass::Resource* resource : writer->resources())
		{
			if (resource->access & RHIAccess::ReadableMask && resource->resource->writer() != writer)
			{
				if (Node* dep = build_graph(resource->resource))
					node->dependencies.push_back(dep);

				if (Resource* next = resource->resource->next())
				{
					Node* depends = build_graph(next);
					depends->dependencies.push_back(node);
				}
			}
		}
		return node;
	}

	Graph::Node* Graph::build_graph(Resource* resource)
	{
		auto writer = resource->writer();

		if (writer == nullptr)
			return nullptr;

		Node* node = build_graph(writer);

		if (Resource* prev = resource->previous())
		{
			if (auto dep = build_graph(prev))
				node->dependencies.push_back(dep);
		}

		return node;
	}

	Graph::Node* Graph::build_graph()
	{
		Node* root = Node::create();

		for (RHIObject* output : m_outputs)
		{
			if (Resource* resource = find_resource(output))
			{
				root->dependencies.push_back(build_graph(resource));
			}
		}

		return root;
	}

	void Graph::execute_node(Node* node, RHIContext* ctx)
	{
		if (!node->is_executed())
		{
			for (Node* dependency : node->dependencies)
			{
				execute_node(dependency, ctx);
			}

			if (!node->is_executed() && !node->is_empty())
			{
				trinex_rhi_push_stage(ctx, node->name());

				Pass* pass = node->pass;
				node->pass = nullptr;

				for (Plugin* plugin : m_plugins) plugin->on_pass_begin(pass);
				pass->execute(ctx);
				for (Plugin* plugin : m_plugins) plugin->on_pass_end(pass);

				trinex_rhi_pop_stage(ctx);
			}
		}
	}

	bool Graph::execute(RHIContext* ctx)
	{
		StackByteAllocator::Mark mark;
		Node* root = build_graph();

		for (Plugin* plugin : m_plugins) plugin->on_frame_begin(this);

		for (Node* dependency : root->dependencies)
		{
			execute_node(dependency, ctx);
		}

		for (Plugin* plugin : m_plugins) plugin->on_frame_end(this);
		return true;
	}
}// namespace Engine::RenderGraph
