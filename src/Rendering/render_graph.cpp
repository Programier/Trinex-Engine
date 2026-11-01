#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/lifecycle.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <Rendering/render_graph.hpp>

namespace Trinex
{
	static constexpr usize s_default_reserve_size = 64;

	class RGResource
	{
	public:
		enum Type
		{
			Texture    = 0,
			Buffer     = 1,
			RenderPass = 2,
		};

	private:
		static constexpr u64 s_resource_type_mask    = 0x7ull;
		static constexpr u64 s_resource_address_mask = ~s_resource_type_mask;

		RGResource* m_next;
		RGResource* m_prev;
		RGPass* m_writer;

		// Since RHIObject is 8-byte aligned, the address is guaranteed to be aligned to the same value.
		// So, the last 3 bits of the address will be zeros. We will actually use them to store the resource type.
		union
		{
			void* m_resource;
			u64 m_resource_address;
		};

	public:
		inline RGResource(RHITexture* resource, RGPass* writer = nullptr, RGResource* prev = nullptr)
		    : m_next(nullptr), m_prev(prev), m_writer(writer), m_resource(resource)
		{
			if (prev)
			{
				prev->m_next = this;
			}
		}

		inline RGResource(RHIBuffer* resource, RGPass* writer = nullptr, RGResource* prev = nullptr)
		    : m_next(nullptr), m_prev(prev), m_writer(writer), m_resource(resource)
		{
			m_resource_address |= Buffer;
			if (prev)
			{
				prev->m_next = this;
			}
		}

		inline RGResource(RGPass* pass) : m_next(nullptr), m_prev(nullptr), m_writer(nullptr), m_resource(pass)
		{
			m_resource_address |= RenderPass;
		}

		template<typename T>
		inline T* as() const
		{
			return reinterpret_cast<T*>(m_resource_address & s_resource_address_mask);
		}

		inline RGPass* writer() const { return m_writer; }
		inline RGResource* previous() const { return m_prev; }
		inline RGResource* next() const { return m_next; }
		inline Type resource_type() const { return static_cast<Type>(m_resource_address & s_resource_type_mask); }
	};

	RHITexture* RGPass::ResourceRef::as_texture() const
	{
		if (resource->resource_type() == RGResource::Texture)
		{
			return resource->as<RHITexture>();
		}
		return nullptr;
	}

	RHIBuffer* RGPass::ResourceRef::as_buffer() const
	{
		if (resource->resource_type() == RGResource::Buffer)
		{
			return resource->as<RHIBuffer>();
		}
		return nullptr;
	}

	RGPass* RGPass::ResourceRef::as_pass() const
	{
		if (resource->resource_type() == RGResource::RenderPass)
		{
			return resource->as<RGPass>();
		}
		return nullptr;
	}

	RGPass::RGPass(RenderGraph* graph, const char* name) : m_graph(graph), m_name(name), m_node(nullptr)
	{
		m_resources.reserve(s_default_reserve_size);
		m_tasks.reserve(s_default_reserve_size);
	}

	RGPass& RGPass::add_resource(RGResource* resource, RHIAccess access, RHIAccess initial)
	{
		if (resource == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references an undefined resource", name());
			return *this;
		}

		m_resources.emplace_back(new (rg_allocate<RGResource>()) ResourceRef(resource, access, initial));
		m_graph->invalidate();
		return *this;
	}

	RGPass& RGPass::add_dependency(RGPass* pass)
	{
		if (pass == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' depends on an undefined pass", name());
			return *this;
		}

		return add_resource(m_graph->find_resource(pass), RHIAccess::Undefined, RHIAccess::Undefined);
	}

	RGPass& RGPass::add_resource(RHITexture* texture, RHIAccess access, RHIAccess initial)
	{
		if (texture == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references a null texture", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(texture, this), access, initial);
		else
			return add_resource(m_graph->find_resource(texture), access, initial);
	}

	RGPass& RGPass::add_resource(RHIBuffer* buffer, RHIAccess access, RHIAccess initial)
	{
		if (buffer == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references a null buffer", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(buffer, this), access, initial);
		else
			return add_resource(m_graph->find_resource(buffer), access, initial);
	}

	RGPass& RGPass::read(RHITexture* texture, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares a texture read with writable access", name());
		}
		return add_resource(texture, access);
	}

	RGPass& RGPass::read(RHIBuffer* buffer, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares a buffer read with writable access", name());
		}
		return add_resource(buffer, access);
	}

	RGPass& RGPass::write(RHITexture* texture, RHIAccess access)
	{
		if (!(access & RHIAccess::WritableMask))
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares a texture write without writable access", name());
		}
		return add_resource(texture, access);
	}

	RGPass& RGPass::write(RHIBuffer* buffer, RHIAccess access)
	{
		if (!(access & RHIAccess::WritableMask))
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares a buffer write without writable access", name());
		}
		return add_resource(buffer, access);
	}

	RGPass& RGPass::side_effect()
	{
		m_never_cull = true;
		m_graph->invalidate();
		return *this;
	}

	RGPass& RGPass::add_untracked_resource(RHITexture* texture, RHIAccess access, RHIAccess initial)
	{
		if (texture == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references a null untracked texture", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->create_resource(texture, this), access, initial);
		else
			return add_resource(m_graph->create_resource(texture), access, initial);
	}

	RGPass& RGPass::add_untracked_resource(RHIBuffer* buffer, RHIAccess access, RHIAccess initial)
	{
		if (buffer == nullptr)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references a null untracked buffer", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->create_resource(buffer, this), access, initial);
		else
			return add_resource(m_graph->create_resource(buffer), access, initial);
	}

	RGPass& RGPass::execute(RHIContext* ctx)
	{
		for (const RGPass::ResourceRef* ref : m_resources)
		{
			auto resource    = ref->resource;
			RHIAccess access = ref->initial == RHIAccess::Undefined ? ref->access : ref->initial;

			switch (resource->resource_type())
			{
				case RGResource::Texture:
				{
					ctx->barrier(resource->as<RHITexture>(), access);
					break;
				}

				case RGResource::Buffer:
				{
					ctx->barrier(resource->as<RHIBuffer>(), access);
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

		// A resource with an explicit initial access is used in that state by the
		// task and leaves the pass in `access`. TODO: promote this to per-resource
		// subresource transitions when RHI exposes subresource barriers.
		for (const RGPass::ResourceRef* ref : m_resources)
		{
			if (ref->initial == RHIAccess::Undefined || ref->initial == ref->access)
				continue;

			switch (ref->resource->resource_type())
			{
				case RGResource::Texture: ctx->barrier(ref->resource->as<RHITexture>(), ref->access); break;
				case RGResource::Buffer: ctx->barrier(ref->resource->as<RHIBuffer>(), ref->access); break;
				default: break;
			}
		}
		return *this;
	}

	RenderGraph::Plugin& RenderGraph::Plugin::on_frame_begin(RenderGraph* graph, RHIContext* ctx)
	{
		return *this;
	}

	RenderGraph::Plugin& RenderGraph::Plugin::on_frame_end(RenderGraph* graph, RHIContext* ctx)
	{
		return *this;
	}

	RenderGraph::Plugin& RenderGraph::Plugin::on_pass_begin(RGPass* pass, RHIContext* ctx)
	{
		return *this;
	}

	RenderGraph::Plugin& RenderGraph::Plugin::on_pass_end(RGPass* pass, RHIContext* ctx)
	{
		return *this;
	}

	RenderGraph::RenderGraph()
	{
		m_resource_map.reserve(s_default_reserve_size);
		m_outputs.reserve(s_default_reserve_size);
		m_passes.reserve(s_default_reserve_size);
		m_execution_plan.reserve(s_default_reserve_size);
	}

	void RenderGraph::invalidate()
	{
		m_is_compiled = false;
		m_execution_plan.clear();

		for (RGPass* pass : m_passes)
		{
			pass->m_node = nullptr;
		}
	}

	RenderGraph& RenderGraph::import(RHITexture* texture)
	{
		if (texture == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot import a null RenderGraph texture");
			return *this;
		}

		find_resource(texture);
		invalidate();
		return *this;
	}

	RenderGraph& RenderGraph::import(RHIBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot import a null RenderGraph buffer");
			return *this;
		}

		find_resource(buffer);
		invalidate();
		return *this;
	}

	RGResource* RenderGraph::create_resource(RHITexture* texture, RGPass* writer)
	{
		return trx_frame_new RGResource(texture, writer, nullptr);
	}

	RGResource* RenderGraph::create_resource(RHIBuffer* buffer, RGPass* writer)
	{
		return trx_frame_new RGResource(buffer, writer, nullptr);
	}

	RGResource* RenderGraph::find_resource(RHIObject* object)
	{
		auto it = m_resource_map.find(object);
		if (it == m_resource_map.end())
			return nullptr;
		return it->second;
	}

	RGResource* RenderGraph::find_resource(RHITexture* texture)
	{
		RGResource*& resource = m_resource_map[texture];

		if (resource == nullptr)
			resource = trx_frame_new RGResource(texture, nullptr, nullptr);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RHIBuffer* buffer)
	{
		RGResource*& resource = m_resource_map[buffer];

		if (resource == nullptr)
			resource = trx_frame_new RGResource(buffer, nullptr, nullptr);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RGPass* pass)
	{
		RGResource*& resource = m_resource_map[pass];

		if (resource == nullptr)
			resource = trx_frame_new RGResource(pass);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RHITexture* texture, RGPass* writer)
	{
		RGResource*& resource = m_resource_map[texture];
		resource              = trx_frame_new RGResource(texture, writer, resource);
		return resource;
	}

	RGResource* RenderGraph::find_resource(RHIBuffer* buffer, RGPass* writer)
	{
		RGResource*& resource = m_resource_map[buffer];
		resource              = trx_frame_new RGResource(buffer, writer, resource);
		return resource;
	}

	RenderGraph& RenderGraph::add_output(RHITexture* texture)
	{
		import(texture);
		m_outputs.insert(texture);
		invalidate();
		return *this;
	}

	RenderGraph& RenderGraph::add_output(RHIBuffer* buffer)
	{
		import(buffer);
		m_outputs.insert(buffer);
		invalidate();
		return *this;
	}

	RGPass& RenderGraph::add_pass(const char* name)
	{
		RGPass* pass = new (rg_allocate<RGPass>()) RGPass(this, name);
		m_passes.push_back(pass);
		invalidate();
		return *pass;
	}

	RenderGraph::Node* RenderGraph::build_graph(RGPass* writer)
	{
		trinex_profile_cpu_n("RenderRenderGraph::Build writer node");

		if (writer->m_node)
			return writer->m_node;

		auto node      = Node::create();
		writer->m_node = node;
		node->pass     = writer;

		for (const RGPass::ResourceRef* ref : writer->resources())
		{
			RGResource* current = ref->resource;

			if (ref->access & RHIAccess::WritableMask)
			{
				current = current->previous();

				if (current == nullptr)
					continue;
			}

			if (Node* dep = build_graph(current))
			{
				node->dependencies.push_back(dep);
			}
		}
		return node;
	}

	RenderGraph::Node* RenderGraph::build_graph(RGResource* resource)
	{
		trinex_profile_cpu_n("RenderRenderGraph::Build resource node");

		auto writer = resource->writer();

		if (writer == nullptr)
		{
			if (resource->resource_type() != RGResource::RenderPass)
				return nullptr;

			writer = resource->as<RGPass>();
		}

		Node* node = build_graph(writer);

		if (RGResource* prev = resource->previous())
		{
			if (auto dep = build_graph(prev))
				node->dependencies.push_back(dep);
		}

		return node;
	}

	RenderGraph::Node* RenderGraph::build_graph()
	{
		trinex_profile_cpu_n("RenderRenderGraph::build");
		Node* root = Node::create();

		for (RHIObject* output : m_outputs)
		{
			if (RGResource* resource = find_resource(output))
			{
				if (Node* dependency = build_graph(resource))
					root->dependencies.push_back(dependency);
			}
		}

		for (RGPass* pass : m_passes)
		{
			if (pass->is_never_culled())
			{
				if (Node* dependency = build_graph(pass))
					root->dependencies.push_back(dependency);
			}
		}

		return root;
	}

	bool RenderGraph::validate_graph() const
	{
		for (const RGPass* pass : m_passes)
		{
			for (const RGPass::ResourceRef* resource : pass->resources())
			{
				if (resource == nullptr || resource->resource == nullptr)
				{
					trinex_error(Log::Graphics, "RenderGraph pass '%s' has an invalid resource declaration", pass->name());
					return false;
				}
			}
		}
		return true;
	}

	bool RenderGraph::schedule_node(Node* node)
	{
		if (node == nullptr || node->is_executed())
			return true;

		if (node->state == Node::State::Scheduled)
			return true;

		if (node->state == Node::State::Visiting)
		{
			trinex_error(Log::Graphics, "RenderGraph contains a dependency cycle at pass '%s'", node->name());
			return false;
		}

		node->state = Node::State::Visiting;
		for (Node* dependency : node->dependencies)
		{
			if (!schedule_node(dependency))
				return false;
		}

		node->state = Node::State::Scheduled;
		if (!node->is_empty())
			m_execution_plan.push_back(node->pass);
		return true;
	}

	bool RenderGraph::compile()
	{
		if (m_is_compiled)
			return true;

		if (!validate_graph())
			return false;

		m_execution_plan.clear();
		Node* root = build_graph();

		for (Node* dependency : root->dependencies)
		{
			if (!schedule_node(dependency))
				return false;
		}

		m_is_compiled = true;
		return true;
	}

	bool RenderGraph::execute(RHIContext* ctx)
	{
		trinex_profile_cpu_n("RenderRenderGraph::execute");
		StackByteAllocator::Mark mark;
		if (!compile())
			return false;

		for (Plugin* plugin : m_plugins)
			plugin->on_frame_begin(this, ctx);

		for (RGPass* pass : m_execution_plan)
		{
			trinex_profile_transient_cpu_n(pass->name());
			trinex_rhi_push_stage(ctx, pass->name());

			for (Plugin* plugin : m_plugins)
				plugin->on_pass_begin(pass, ctx);
			pass->execute(ctx);
			for (Plugin* plugin : m_plugins)
				plugin->on_pass_end(pass, ctx);

			trinex_rhi_pop_stage(ctx);
		}

		for (Plugin* plugin : m_plugins)
			plugin->on_frame_end(this, ctx);
		return true;
	}

	trinex_on_testing(.name = "RenderGraph")
	{
		{
			RenderGraph graph;
			RGPass& first  = graph.add_pass("First").side_effect();
			RGPass& second = graph.add_pass("Second").add_dependency(&first).side_effect();
			first.add_func([](RHIContext*) {});
			second.add_func([](RHIContext*) {});

			trinex_verify(graph.compile());
			const auto& plan = graph.execution_plan();
			trinex_verify_msg(plan.size() == 2, "The execution plan must contain both side-effect passes");
			trinex_verify_msg(plan[0] == &first && plan[1] == &second, "Dependencies must execute before their consumers");
		}

		{
			RenderGraph graph;
			graph.add_pass("Culled").add_func([](RHIContext*) {});
			RGPass& side_effect = graph.add_pass("Side Effect").side_effect();
			side_effect.add_func([](RHIContext*) {});

			trinex_verify(graph.compile());
			const auto& plan = graph.execution_plan();
			trinex_verify_msg(plan.size() == 1 && plan[0] == &side_effect, "Unreachable passes must be culled");
		}

		{
			RenderGraph graph;
			RGPass& first  = graph.add_pass("First");
			RGPass& second = graph.add_pass("Second");
			first.add_dependency(&second).side_effect();
			second.add_dependency(&first);

			trinex_verify_msg(!graph.compile(), "A cyclic graph must fail compilation");
		}
	}
} // namespace Trinex
