#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/memory.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Graphics/rhi.hpp>
#include <cstring>

namespace Engine::RenderGraph
{
	static constexpr size_t s_default_reserve_size = 64;

	static FORCE_INLINE const char* copy_string(StringView str)
	{
#if TRINEX_DEBUG_BUILD
		byte* memory = FrameByteAllocator::allocate(str.size() + 1);
		std::memcpy(memory, str.data(), str.size());
		memory[str.size()] = 0;
		return reinterpret_cast<const char*>(memory);
#else
		return "";
#endif
	}

	Pass::Pass(Graph* graph, Type type, const StringView& name)
	    : m_graph(graph), m_name(copy_string(name)), m_node(nullptr), m_type(type), m_flags(Flags::Undefined)
	{
		m_resources.reserve(s_default_reserve_size);
		m_tasks.reserve(s_default_reserve_size);
	}

	Pass& Pass::add_resource(RenderGraph::Resource* resource, RHIAccess access)
	{
		Resource* pass_resource = new (rg_allocate<Resource>()) Resource(resource, access);
		m_resources.emplace_back(pass_resource);

		const bool is_reading = access & RHIAccess::ReadableMask;
		const bool is_writing = access & RHIAccess::WritableMask;

		if (is_reading && is_writing)
			resource->add_read_writer(this);
		else if (is_writing)
			resource->add_writer(this);
		else if (is_reading)
			resource->add_reader(this);
		return *this;
	}

	Pass& Pass::add_resource(RHI_Texture* texture, RHIAccess access)
	{
		return add_resource(&m_graph->find_resource(texture), access);
	}

	Pass& Pass::add_resource(RHI_Buffer* buffer, RHIAccess access)
	{
		return add_resource(&m_graph->find_resource(buffer), access);
	}

	Graph::Graph()
	{
		m_resource_map.reserve(s_default_reserve_size);
		m_passes.reserve(s_default_reserve_size);
		m_outputs.reserve(s_default_reserve_size);
	}

	Resource& Graph::find_resource(RHI_Texture* texture)
	{
		auto it = m_resource_map.find(texture);

		if (it == m_resource_map.end())
		{
			Resource* resource      = new (rg_allocate<Resource>()) Resource();
			m_resource_map[texture] = resource;
			return *resource;
		}
		else
		{
			return *(it->second);
		}
	}

	Resource& Graph::find_resource(RHI_Buffer* buffer)
	{
		auto it = m_resource_map.find(buffer);

		if (it == m_resource_map.end())
		{
			Resource* resource     = new (rg_allocate<Resource>()) Resource();
			m_resource_map[buffer] = resource;
			return *resource;
		}
		else
		{
			return *(it->second);
		}
	}

	Graph& Graph::add_output(RHI_Texture* texture)
	{
		m_outputs.insert(&find_resource(texture));
		return *this;
	}

	Graph& Graph::add_output(RHI_Buffer* buffer)
	{
		m_outputs.insert(&find_resource(buffer));
		return *this;
	}

	Pass& Graph::add_pass(Pass::Type type, const StringView& name)
	{
		Pass* pass = new (rg_allocate<Pass>()) Pass(this, type, name);
		m_passes.emplace_back(pass);
		return *pass;
	}

	Graph& Graph::build_graph(Pass* writer, Node* owner)
	{
		if (!(writer->m_flags & Pass::Flags::IsLive))
		{
			writer->m_flags |= Pass::Flags::IsLive;

			Node* writer_node = Node::create();
			writer_node->pass = writer;
			writer->m_node    = writer_node;
			writer_node->dependents.push_back(owner);
			owner->dependencies.push_back(writer_node);

			for (const Pass::Resource* pr : writer->resources())
			{
				if (pr->access & RHIAccess::ReadableMask)
				{
					build_graph(pr->resource, writer_node);
				}
			}
		}
		else if (writer->m_node != owner)
		{
			Node* writer_node = static_cast<Node*>(writer->m_node);
			writer_node->dependents.push_back(owner);
			owner->dependencies.push_back(writer_node);
		}
		return *this;
	}

	Graph& Graph::build_graph(Resource* resource, Node* owner)
	{
		for (Pass* writer : resource->writers())
		{
			build_graph(writer, owner);
		}

		for (Pass* writer : resource->read_writers())
		{
			build_graph(writer, owner);
		}
		return *this;
	}

	Graph::Node* Graph::build_graph()
	{
		Node* root = Node::create();
		root->pass = &add_pass(Pass::Type::Graphics);

		for (Resource* output : m_outputs)
		{
			build_graph(output, root);
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

			if (!node->is_executed())
			{
				node->execute();
			}
		}
	}

	bool Graph::execute()
	{
		Node* root = build_graph();

		// TODO: Implement graph optimizations

		execute_node(root);
		return true;
	}
}// namespace Engine::RenderGraph
