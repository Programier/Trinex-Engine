#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/memory.hpp>
#include <Engine/Render/render_graph.hpp>
#include <RHI/rhi.hpp>

namespace Engine::RenderGraph
{
	static constexpr size_t s_default_reserve_size = 64;

	class Resource
	{
	private:
		Resource* m_next_version = nullptr;
		RHIObject* m_resource;
		Pass* m_writer;
		uint32_t m_version;

	public:
		inline Resource(RHIObject* resource, size_t version, Pass* writer = nullptr)
		    : m_resource(resource), m_writer(writer), m_version(version)
		{}
		inline Pass* writer() const { return m_writer; }
		inline Resource* next_version() const { return m_next_version; }
		inline Resource* next_version(Resource* resource) { return (m_next_version = resource); }
		inline RHIObject* resource() const { return m_resource; }
		inline uint32_t version() const { return m_version; }
		friend class Graph;
	};

	Pass::Pass(Graph* graph, Type type, const char* name)
	    : m_graph(graph), m_name(name), m_node(nullptr), m_type(type), m_flags(Flags::Undefined)
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
			return add_resource(m_graph->writable_resource(texture, this), access);
		else
			return add_resource(m_graph->readable_resource(texture), access);
	}

	Pass& Pass::add_resource(RHIBuffer* buffer, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->writable_resource(buffer, this), access);
		else
			return add_resource(m_graph->readable_resource(buffer), access);
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
		m_passes.reserve(s_default_reserve_size);
		m_outputs.reserve(s_default_reserve_size);
	}

	Graph::ResourceEntry* Graph::find_resource(RHIObject* object)
	{
		return &m_resource_map[object];
	}

	Graph::ResourceEntry* Graph::find_resource(RHITexture* texture)
	{
		auto& entry = m_resource_map[texture];

		if (entry.first == nullptr)
			entry.first = entry.last = new (rg_allocate<Resource>()) Resource(texture, 0);

		return &entry;
	}

	Graph::ResourceEntry* Graph::find_resource(RHIBuffer* buffer)
	{
		auto& entry = m_resource_map[buffer];

		if (entry.first == nullptr)
			entry.first = entry.last = new (rg_allocate<Resource>()) Resource(buffer, 0);

		return &entry;
	}

	Resource* Graph::writable_resource(RHITexture* texture, Pass* writer)
	{
		ResourceEntry* entry = find_resource(texture);
		uint_t version       = entry->last->version() + 1;
		entry->last          = entry->last->next_version(new (rg_allocate<Resource>()) Resource(texture, version, writer));
		return entry->last;
	}

	Resource* Graph::writable_resource(RHIBuffer* buffer, Pass* writer)
	{
		ResourceEntry* entry = find_resource(buffer);
		uint_t version       = entry->last->version() + 1;
		entry->last          = entry->last->next_version(new (rg_allocate<Resource>()) Resource(buffer, version, writer));
		return entry->last;
	}

	Graph& Graph::add_output(RHITexture* texture)
	{
		m_outputs.insert(find_resource(texture)->first);
		return *this;
	}

	Graph& Graph::add_output(RHIBuffer* buffer)
	{
		m_outputs.insert(find_resource(buffer)->first);
		return *this;
	}

	Pass& Graph::add_pass(Pass::Type type, const char* name)
	{
		Pass* pass = new (rg_allocate<Pass>()) Pass(this, type, name);
		m_passes.emplace_back(pass);
		return *pass;
	}

	Graph& Graph::build_graph(Pass* writer, Node* owner)
	{
		if (owner->pass == writer)
			return *this;

		if (!writer->is_visited())
		{
			writer->add_flags(Pass::Flags::IsVisited);

			Node* writer_node = Node::create();
			writer_node->pass = writer;
			writer->m_node    = writer_node;
			owner->dependencies.push_back(writer_node);

			for (Pass* dependency : writer->dependencies())
			{
				build_graph(dependency, writer_node);
			}

			for (const Pass::Resource* pr : writer->resources())
			{
				if (pr->access & RHIAccess::ReadableMask)
				{
					build_graph(pr->resource, writer_node);
				}
			}
		}
		return *this;
	}

	Graph& Graph::build_graph(Resource* resource, Node* owner)
	{
		uint32_t requested_version = resource->version();
		Resource*& resource_state  = find_resource(resource->resource())->first;

		while (resource_state && resource_state->version() <= requested_version)
		{
			Resource* current_state = resource_state;
			resource_state          = current_state->next_version();

			if (Pass* writer = current_state->writer())
			{
				build_graph(writer, owner);
			}
		}
		return *this;
	}

	Graph& Graph::build_output(Resource* resource, Node* owner)
	{
		while (resource)
		{
			if (Pass* writer = resource->writer())
			{
				build_graph(writer, owner);
			}

			resource = resource->next_version();
		}
		return *this;
	}

	Graph::Node* Graph::build_graph()
	{
		Node* root = Node::create();
		root->pass = &add_pass(Pass::Type::Graphics, "Root");

		for (Resource* output : m_outputs)
		{
			build_output(output, root);
		}

		return root;
	}

	void Graph::execute_node(Node* node)
	{
		if (!node->is_executed())
		{
			for (Node* dependency : node->dependencies)
			{
				execute_node(dependency);
			}

			if (!node->is_executed() && !node->is_empty())
			{
				trinex_rhi_push_stage(node->name());

				for (Plugin* plugin : m_plugins) plugin->on_pass_begin(node->pass);

				node->execute();

				for (Plugin* plugin : m_plugins) plugin->on_pass_end(node->pass);
				trinex_rhi_pop_stage();
			}
		}
	}

	bool Graph::execute()
	{
		Node* root = build_graph();

		// TODO: Implement graph optimizations

		for (Plugin* plugin : m_plugins) plugin->on_frame_begin(this);
		execute_node(root);
		for (Plugin* plugin : m_plugins) plugin->on_frame_end(this);
		return true;
	}
}// namespace Engine::RenderGraph
