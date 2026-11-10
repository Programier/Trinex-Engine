#include <Core/etl/allocator.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/lifecycle.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <Rendering/render_graph.hpp>

namespace Trinex
{
	static constexpr usize s_default_reserve_size = 64;

	// TODO(RenderGraph):
	// - Make imports and exports explicit in all passes; imported resources retain external ownership.
	// - Use RGLifetime to acquire transient resources at first use and release them after their final use.
	// - Alias compatible transient allocations whose lifetimes do not overlap.
	// - Add texture subresource ranges (mip/layer/aspect) once RHI supports subresource barriers.
	// - Add graphics/compute/copy queue assignment, synchronization, and ownership transfers.
	// - Support parallel command recording for independent execution-plan branches.
	// - Expose a graph debugger: passes, dependencies, resource versions, lifetimes, barriers, and allocations.

	class RGResource
	{
	public:
		enum Type
		{
			Texture    = 0,
			Buffer     = 1,
			RenderPass = 2,
		};

		struct Backing
		{
			Type type;
			RHITexture* texture = nullptr;
			RHIBuffer* buffer   = nullptr;
			RGTextureDesc texture_desc;
			RGBufferDesc buffer_desc;
			RGLifetime lifetime;
			bool imported = false;

			explicit Backing(RHITexture* value) : type(Texture), texture(value), imported(true) {}
			explicit Backing(RHIBuffer* value) : type(Buffer), buffer(value), imported(true) {}
			explicit Backing(const RGTextureDesc& value) : type(Texture), texture_desc(value) {}
			explicit Backing(const RGBufferDesc& value) : type(Buffer), buffer_desc(value) {}
		};

	private:
		RGResource* m_next;
		RGResource* m_prev;
		RGPass* m_writer;
		Backing* m_backing;
		RGPass* m_pass;

	public:
		inline RGResource(Backing* backing, RGPass* writer = nullptr, RGResource* prev = nullptr)
		    : m_next(nullptr), m_prev(prev), m_writer(writer), m_backing(backing), m_pass(nullptr)
		{
			if (prev)
				prev->m_next = this;
		}

		inline RGResource(RGPass* pass) : m_next(nullptr), m_prev(nullptr), m_writer(nullptr), m_backing(nullptr), m_pass(pass) {}

		RHITexture* texture()
		{
			if (m_backing == nullptr || m_backing->type != Texture)
				return nullptr;

			if (m_backing->texture == nullptr)
			{
				const RGTextureDesc& desc = m_backing->texture_desc;
				m_backing->texture = RHITexturePool::instance()->acquire_transient(desc.type, desc.format, desc.size, desc.flags);
			}
			return m_backing->texture;
		}

		RHIBuffer* buffer()
		{
			if (m_backing == nullptr || m_backing->type != Buffer)
				return nullptr;

			if (m_backing->buffer == nullptr)
			{
				const RGBufferDesc& desc = m_backing->buffer_desc;
				m_backing->buffer        = RHIBufferPool::instance()->acquire_transient(desc.size, desc.flags);
			}
			return m_backing->buffer;
		}

		inline RGPass* writer() const { return m_writer; }
		inline RGResource* previous() const { return m_prev; }
		inline RGResource* next() const { return m_next; }
		inline RGPass* pass() const { return m_pass; }
		inline Backing* backing() const { return m_backing; }
		inline Type resource_type() const { return m_backing ? m_backing->type : RenderPass; }
		inline bool is_imported() const { return m_backing && m_backing->imported; }
		inline RGLifetime lifetime() const { return m_backing ? m_backing->lifetime : RGLifetime{}; }
	};

	RHITexture* RGPass::ResourceRef::as_texture() const
	{
		if (resource->resource_type() == RGResource::Texture)
			return resource->texture();
		return nullptr;
	}

	RHIBuffer* RGPass::ResourceRef::as_buffer() const
	{
		if (resource->resource_type() == RGResource::Buffer)
			return resource->buffer();
		return nullptr;
	}

	RGPass* RGPass::ResourceRef::as_pass() const
	{
		if (resource->resource_type() == RGResource::RenderPass)
			return resource->pass();
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

	RGPass& RGPass::add_resource(RGTexture texture, RHIAccess access, RHIAccess initial)
	{
		if (!texture.is_valid())
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references an invalid RG texture", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(texture, this), access, initial);
		return add_resource(m_graph->find_resource(texture), access, initial);
	}

	RGPass& RGPass::add_resource(RGBuffer buffer, RHIAccess access, RHIAccess initial)
	{
		if (!buffer.is_valid())
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' references an invalid RG buffer", name());
			return *this;
		}

		if (access & RHIAccess::WritableMask)
			return add_resource(m_graph->find_resource(buffer, this), access, initial);
		return add_resource(m_graph->find_resource(buffer), access, initial);
	}

	RGPass& RGPass::read(RGTexture texture, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares an RG texture read with writable access", name());
		}
		return add_resource(texture, access);
	}

	RGPass& RGPass::read(RGBuffer buffer, RHIAccess access)
	{
		if (access & RHIAccess::WritableMask)
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares an RG buffer read with writable access", name());
		}
		return add_resource(buffer, access);
	}

	RGPass& RGPass::write(RGTexture texture, RHIAccess access)
	{
		if (!(access & RHIAccess::WritableMask))
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares an RG texture write without writable access", name());
		}
		return add_resource(texture, access);
	}

	RGPass& RGPass::write(RGBuffer buffer, RHIAccess access)
	{
		if (!(access & RHIAccess::WritableMask))
		{
			trinex_error(Log::Graphics, "RenderGraph pass '%s' declares an RG buffer write without writable access", name());
		}
		return add_resource(buffer, access);
	}

	RGPass& RGPass::side_effect()
	{
		m_never_cull = true;
		m_graph->invalidate();
		return *this;
	}

	RHITexture* RGContext::texture(RGTexture texture) const
	{
		return m_graph->texture(texture);
	}

	RHIBuffer* RGContext::buffer(RGBuffer buffer) const
	{
		return m_graph->buffer(buffer);
	}

	RGPass& RGPass::execute(RHIContext* ctx)
	{
		RGContext context(m_graph, ctx);
		for (const RGPass::ResourceRef* ref : m_resources)
		{
			auto resource    = ref->resource;
			RHIAccess access = ref->initial == RHIAccess::Undefined ? ref->access : ref->initial;

			switch (resource->resource_type())
			{
				case RGResource::Texture:
				{
					ctx->barrier(resource->texture(), access);
					break;
				}

				case RGResource::Buffer:
				{
					ctx->barrier(resource->buffer(), access);
					break;
				}

				default: break;
			}
		}

		for (Task* task : m_tasks)
		{
			task->execute(context);
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
				case RGResource::Texture: ctx->barrier(ref->resource->texture(), ref->access); break;
				case RGResource::Buffer: ctx->barrier(ref->resource->buffer(), ref->access); break;
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

	RGTexture RenderGraph::import(RHITexture* texture)
	{
		if (texture == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot import a null RenderGraph texture");
			return {};
		}

		find_resource(texture);
		invalidate();
		return RGTexture(texture);
	}

	RGBuffer RenderGraph::import(RHIBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot import a null RenderGraph buffer");
			return {};
		}

		find_resource(buffer);
		invalidate();
		return RGBuffer(buffer);
	}

	RGResource* RenderGraph::create_resource(RHITexture* texture, RGPass* writer)
	{
		return trx_frame_new RGResource(trx_frame_new RGResource::Backing(texture), writer, nullptr);
	}

	RGResource* RenderGraph::create_resource(RHIBuffer* buffer, RGPass* writer)
	{
		return trx_frame_new RGResource(trx_frame_new RGResource::Backing(buffer), writer, nullptr);
	}

	RGResource* RenderGraph::create_resource(const RGTextureDesc& desc, RGPass* writer)
	{
		return trx_frame_new RGResource(trx_frame_new RGResource::Backing(desc), writer, nullptr);
	}

	RGResource* RenderGraph::create_resource(const RGBufferDesc& desc, RGPass* writer)
	{
		return trx_frame_new RGResource(trx_frame_new RGResource::Backing(desc), writer, nullptr);
	}

	RGResource* RenderGraph::find_resource(void* identity)
	{
		auto it = m_resource_map.find(identity);
		if (it == m_resource_map.end())
			return nullptr;
		return it->second;
	}

	RGResource* RenderGraph::find_resource(RHITexture* texture)
	{
		RGResource*& resource = m_resource_map[texture];

		if (resource == nullptr)
			resource = create_resource(texture);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RHIBuffer* buffer)
	{
		RGResource*& resource = m_resource_map[buffer];

		if (resource == nullptr)
			resource = create_resource(buffer);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RGTexture texture)
	{
		return texture.is_valid() ? find_resource(texture.m_identity) : nullptr;
	}

	RGResource* RenderGraph::find_resource(RGBuffer buffer)
	{
		return buffer.is_valid() ? find_resource(buffer.m_identity) : nullptr;
	}

	RGResource* RenderGraph::find_resource(RGPass* pass)
	{
		RGResource*& resource = m_resource_map[pass];

		if (resource == nullptr)
			resource = trx_frame_new RGResource(pass);

		return resource;
	}

	RGResource* RenderGraph::find_resource(RGTexture texture, RGPass* writer)
	{
		RGResource* previous = find_resource(texture);
		if (previous == nullptr)
			return nullptr;

		RGResource*& resource = m_resource_map[texture.m_identity];
		resource              = trx_frame_new RGResource(previous->backing(), writer, previous);
		return resource;
	}

	RGResource* RenderGraph::find_resource(RGBuffer buffer, RGPass* writer)
	{
		RGResource* previous = find_resource(buffer);
		if (previous == nullptr)
			return nullptr;

		RGResource*& resource = m_resource_map[buffer.m_identity];
		resource              = trx_frame_new RGResource(previous->backing(), writer, previous);
		return resource;
	}

	RGTexture RenderGraph::create_texture(const RGTextureDesc& desc)
	{
		if (desc.size.x == 0 || desc.size.y == 0 || desc.size.z == 0 || desc.flags == RHITextureFlags::Undefined)
		{
			trinex_error(Log::Graphics, "Cannot create an invalid RenderGraph texture");
			return {};
		}

		RGResource* resource                = create_resource(desc);
		m_resource_map[resource->backing()] = resource;
		invalidate();
		return RGTexture(resource->backing());
	}

	RGBuffer RenderGraph::create_buffer(const RGBufferDesc& desc)
	{
		if (desc.size == 0 || desc.flags == RHIBufferFlags::Undefined)
		{
			trinex_error(Log::Graphics, "Cannot create an invalid RenderGraph buffer");
			return {};
		}

		RGResource* resource                = create_resource(desc);
		m_resource_map[resource->backing()] = resource;
		invalidate();
		return RGBuffer(resource->backing());
	}

	RenderGraph& RenderGraph::add_output(RGTexture texture)
	{
		if (!texture.is_valid() || find_resource(texture) == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot export an undefined RenderGraph texture");
			return *this;
		}

		m_outputs.insert(texture.m_identity);
		invalidate();
		return *this;
	}

	RenderGraph& RenderGraph::add_output(RGBuffer buffer)
	{
		if (!buffer.is_valid() || find_resource(buffer) == nullptr)
		{
			trinex_error(Log::Graphics, "Cannot export an undefined RenderGraph buffer");
			return *this;
		}

		m_outputs.insert(buffer.m_identity);
		invalidate();
		return *this;
	}

	RHITexture* RenderGraph::texture(RGTexture texture)
	{
		RGResource* resource = find_resource(texture);
		return resource ? resource->texture() : nullptr;
	}

	RHIBuffer* RenderGraph::buffer(RGBuffer buffer)
	{
		RGResource* resource = find_resource(buffer);
		return resource ? resource->buffer() : nullptr;
	}

	RGLifetime RenderGraph::lifetime(RGTexture texture) const
	{
		auto it = m_resource_map.find(texture.m_identity);
		return it == m_resource_map.end() ? RGLifetime{} : it->second->lifetime();
	}

	RGLifetime RenderGraph::lifetime(RGBuffer buffer) const
	{
		auto it = m_resource_map.find(buffer.m_identity);
		return it == m_resource_map.end() ? RGLifetime{} : it->second->lifetime();
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

			writer = resource->pass();
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

		for (void* output : m_outputs)
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

				const bool is_write = resource->access & RHIAccess::WritableMask;
				if (resource->resource->resource_type() != RGResource::RenderPass && !is_write
				    && !resource->resource->is_imported() && resource->resource->writer() == nullptr
				    && resource->resource->previous() == nullptr)
				{
					trinex_error(Log::Graphics, "RenderGraph pass '%s' reads a transient resource before it is written",
					             pass->name());
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

		for (auto& [identity, resource] : m_resource_map)
		{
			static_cast<void>(identity);
			if (resource->backing())
				resource->backing()->lifetime = {};
		}

		for (u32 pass_index = 0; pass_index < m_execution_plan.size(); ++pass_index)
		{
			for (const RGPass::ResourceRef* ref : m_execution_plan[pass_index]->resources())
			{
				if (RGResource::Backing* backing = ref->resource->backing())
				{
					backing->lifetime.first_pass = std::min(backing->lifetime.first_pass, pass_index);
					backing->lifetime.last_pass  = pass_index;
				}
			}
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
			if (ctx)
				trinex_rhi_push_stage(ctx, pass->name());

			for (Plugin* plugin : m_plugins)
				plugin->on_pass_begin(pass, ctx);
			pass->execute(ctx);
			for (Plugin* plugin : m_plugins)
				plugin->on_pass_end(pass, ctx);

			if (ctx)
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
			first.add_func([](RGContext& context) { trinex_verify(context.rhi() == nullptr); });
			second.add_func([](RHIContext*) {});

			trinex_verify(graph.compile());
			const auto& plan = graph.execution_plan();
			trinex_verify_msg(plan.size() == 2, "The execution plan must contain both side-effect passes");
			trinex_verify_msg(plan[0] == &first && plan[1] == &second, "Dependencies must execute before their consumers");
			trinex_verify(graph.execute(nullptr));
		}

		{
			RenderGraph graph;
			RGTexture texture = graph.create_texture({
			        .size  = {16, 16, 1},
			        .flags = RHITextureFlags::RWColorAttachment,
			});
			RGBuffer buffer   = graph.create_buffer({
			        .size  = 256,
			        .flags = RHIBufferFlags::ShaderResource | RHIBufferFlags::UnorderedAccess,
			});

			trinex_verify(texture.is_valid());
			trinex_verify(buffer.is_valid());

			RGPass& produce = graph.add_pass("Produce").side_effect();
			produce.write(texture, RHIAccess::RTV).write(buffer, RHIAccess::UAVCompute).add_func([](RHIContext*) {});

			RGPass& consume = graph.add_pass("Consume").side_effect();
			consume.read(texture).read(buffer, RHIAccess::SRVCompute).add_func([](RHIContext*) {});

			trinex_verify(graph.compile());
			const auto& plan = graph.execution_plan();
			trinex_verify_msg(plan.size() == 2, "Logical resources must retain producer-consumer dependencies");
			trinex_verify_msg(plan[0] == &produce && plan[1] == &consume,
			                  "Logical resources must schedule writers before readers");
			trinex_verify_msg(graph.lifetime(texture).first_pass == 0 && graph.lifetime(texture).last_pass == 1,
			                  "Texture lifetime must cover every pass that references it");
			trinex_verify_msg(graph.lifetime(buffer).first_pass == 0 && graph.lifetime(buffer).last_pass == 1,
			                  "Buffer lifetime must cover every pass that references it");
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
