#include <Core/math/math.hpp>
#include <Graphics/pipeline_library.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/RenderInterface.h>
#include <imgui.h>
#include <imgui_internal.h>

#define rml_stub_log() 

namespace Trinex::UI::RML::Backend
{
	namespace
	{
		template<typename Instance, typename Handle>
		class RMLHandle
		{
		public:
			static Instance* from(Handle handle) { return reinterpret_cast<Instance*>(handle); }
			Handle handle() const { return reinterpret_cast<Handle>(this); }

			virtual ~RMLHandle() {}
		};

		class RMLGeometry : public RMLHandle<RMLGeometry, Rml::CompiledGeometryHandle>
		{
		private:
			RHIBuffer* m_vertex;
			RHIBuffer* m_index;
			usize m_count;

		private:
			static RHIBuffer* create_buffer(const void* data, usize size, RHIBufferFlags flags)
			{
				RHIBuffer* buffer = RHI::instance()->create_buffer(size, flags);
				return buffer;
			}

			template<typename T>
			static RHIBuffer* create_buffer(const Rml::Span<T>& buffer, RHIBufferFlags flags)
			{
				return create_buffer(buffer.data(), buffer.size() * sizeof(T), flags);
			}

		public:
			RMLGeometry(RHIContext* ctx, Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
			{
				m_vertex = create_buffer(vertices, RHIBufferFlags::VertexBuffer);
				m_index  = create_buffer(indices, RHIBufferFlags::IndexBuffer);
				m_count  = indices.size();

				ctx->barrier(m_vertex, RHIAccess::TransferDst);
				ctx->barrier(m_index, RHIAccess::TransferDst);

				ctx->update(m_vertex, vertices.data(), {.size = m_vertex->size()});
				ctx->update(m_index, indices.data(), {.size = m_index->size()});

				ctx->barrier(m_vertex, RHIAccess::VertexBuffer);
				ctx->barrier(m_index, RHIAccess::IndexBuffer);
			}

			~RMLGeometry()
			{
				m_vertex->release();
				m_index->release();
			}

			inline RHIBuffer* vertex() const { return m_vertex; }
			inline RHIBuffer* index() const { return m_index; }
			inline usize count() const { return m_count; }

			inline RMLGeometry& bind(RHIContext* ctx)
			{
				ctx->bind_vertex_buffer(m_vertex, 0, sizeof(Rml::Vertex), 0);
				ctx->bind_index_buffer(m_index, RHIIndexFormat::UInt32);
				return *this;
			}
		};

		class RMLTexture : public RMLHandle<RMLTexture, Rml::TextureHandle>
		{
		public:
			virtual RHISampler* sampler() = 0;
			virtual RHITexture* texture() = 0;
		};

		class RMLTextureCopy final : public RMLTexture
		{
		private:
			RHITexture* m_texture;
			RHISampler* m_sampler;

		public:
			RMLTextureCopy(Vector2u size, RHITextureFlags flags, RHISampler* sampler = nullptr)
			{
				auto pool = RHITexturePool::global_instance();
				m_texture = pool->acquire(RHISurfaceFormat::RGBA8, {size.x, size.y}, flags);
				m_sampler = sampler ? sampler : RHIBilinearWrapSampler::static_sampler();
			}

			RHISampler* sampler() override { return m_sampler; }
			RHITexture* texture() override { return m_texture; }

			~RMLTextureCopy()
			{
				auto pool = RHITexturePool::global_instance();
				pool->release(reinterpret_cast<RHITexture*>(m_texture));
			}
		};

		class RMLTextureRef final : public RMLTexture
		{
		private:
			RHITexture* m_texture;
			RHISampler* m_sampler;

		public:
			RMLTextureRef(RHITexture* texture, RHISampler* sampler = nullptr) : m_texture(texture)
			{
				texture->add_reference();
				m_sampler = sampler ? sampler : RHIBilinearWrapSampler::static_sampler();
			}

			RHISampler* sampler() override { return m_sampler; }
			RHITexture* texture() override { return m_texture; }
			~RMLTextureRef() { m_texture->release(); }
		};

		class RMLPipeline : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(RMLPipeline, GlobalPipelineLibrary);

		private:
			struct TexturePipeline {
				GraphicsPipeline* pipeline;
				const RHIShaderParameterInfo* translate;
				const RHIShaderParameterInfo* transform;
				const RHIShaderParameterInfo* texture;
			} m_texture;

			struct ColorPipeline {
				GraphicsPipeline* pipeline;
				const RHIShaderParameterInfo* translate;
				const RHIShaderParameterInfo* transform;
			} m_color;

		public:
			static void bind(RHIContext* ctx, TexturePipeline* pipeline, const Matrix4f& transform, Rml::Vector2f translation,
			                 RMLTexture* texture)
			{
				ctx->bind_pipeline(pipeline->pipeline->handle());
				ctx->update_scalar(&translation, pipeline->translate);
				ctx->update_scalar(&transform, pipeline->transform);
				ctx->bind_srv(texture->texture()->as_srv(), pipeline->texture->binding);
				ctx->bind_sampler(texture->sampler(), pipeline->texture->binding);
			}

			static void bind(RHIContext* ctx, ColorPipeline* pipeline, const Matrix4f& transform, Rml::Vector2f translation)
			{
				ctx->bind_pipeline(pipeline->pipeline->handle());
				ctx->update_scalar(&translation, pipeline->translate);
				ctx->update_scalar(&transform, pipeline->transform);
			}

			static inline TexturePipeline* texture() { return &instance()->m_texture; }
			static inline ColorPipeline* color() { return &instance()->m_color; }
		};

		trinex_implement_pipeline(RMLPipeline, "[shaders]:/TrinexEditor/rml.slang")
		{
			m_texture.pipeline  = find_graphics_pipeline("Texture");
			m_texture.translate = m_texture.pipeline->find_parameter("translate");
			m_texture.transform = m_texture.pipeline->find_parameter("transform");
			m_texture.texture   = m_texture.pipeline->find_parameter("texture");

			m_color.pipeline  = find_graphics_pipeline("Color");
			m_color.translate = m_color.pipeline->find_parameter("translate");
			m_color.transform = m_color.pipeline->find_parameter("transform");
		}

		class RMLRenderInterface final : public Rml::RenderInterface
		{
		private:
			RHIContext* m_context = nullptr;
			Matrix4f m_projection;
			Matrix4f m_transform;

			struct Viewport {
				Vector2u size;
				Vector2u pos;
			} m_viewport;

		public:
			RHIContext* context()
			{
				if (m_context == nullptr)
				{
					m_context = RHIContextPool::global_instance()->begin();
				}

				return m_context;
			}

		public:
			void Begin(Rml::Vector2i size) override
			{
				m_viewport.pos.x = ImGui::GetCursorScreenPos().x;
				m_viewport.pos.y = ImGui::GetCursorScreenPos().y;
				m_viewport.size  = {size.x, size.y};
				m_projection     = Math::ortho(0, m_viewport.size.x, m_viewport.size.y, 0, 0.f, 1.f);
				m_transform      = m_projection;

				auto callback = ImDrawCallbackFunc
				{
					RHIContext* ctx    = args.ctx;
					Viewport* viewport = static_cast<Viewport*>(args.cmd->UserCallbackData);

					ctx->push_debug_stage("RML");

					ctx->bind_vertex_attribute(RHISemantic::Position, RHIVertexFormat::RG32F, 0, offsetof(Rml::Vertex, position));
					ctx->bind_vertex_attribute(RHISemantic::Color, RHIVertexFormat::RGBA8, 0, offsetof(Rml::Vertex, colour));
					ctx->bind_vertex_attribute(RHISemantic::TexCoord0, RHIVertexFormat::RG32F, 0,
					                           offsetof(Rml::Vertex, tex_coord));

					RHIRenderingInfo info;
					info.colors[0].view = args.color->as_rtv();

					ctx->begin_rendering(info);

					Vector2f window  = {args.data->DisplaySize.x, args.data->DisplaySize.y};
					RHIRegion region = RHIRegion(Vector2f(viewport->size) / window, Vector2f(viewport->pos) / window);

					ctx->viewport(region);
					ctx->scissor(region);
					ctx->blending_state(RHIBlendingState::alpha_composite());
				};

				ImGui::GetWindowDrawList()->AddCallback(callback, &m_viewport, sizeof(m_viewport));
			}

			void End() override
			{
				if (m_context)
				{
					RHIContextPool::global_instance()->end(m_context);
					m_context = nullptr;
				}

				ImGui::GetWindowDrawList()->AddCallback(ImDrawCallbackFunc {
					args.ctx->end_rendering();
					args.ctx->pop_debug_stage();
				});

				ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState);
			}

			Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
			                                            Rml::Span<const int> indices) override
			{
				return (trx_new RMLGeometry(context(), vertices, indices))->handle();
			}

			void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
			                    Rml::TextureHandle texture) override
			{
				struct Instance {
					RMLGeometry* geometry;
					RMLTexture* texture;
					Matrix4f transform;
					Rml::Vector2f translation;
				} instance;

				instance.geometry    = RMLGeometry::from(geometry);
				instance.texture     = RMLTexture::from(texture);
				instance.transform   = m_transform;
				instance.translation = translation;

				auto list = ImGui::GetWindowDrawList();

				auto callback = ImDrawCallbackFunc
				{
					Instance* instance = static_cast<Instance*>(args.cmd->UserCallbackData);
					RHIContext* ctx    = args.ctx;

					instance->geometry->bind(ctx);

					if (instance->texture)
					{
						RMLPipeline::bind(ctx, RMLPipeline::texture(), instance->transform, instance->translation,
						                  instance->texture);
					}
					else
					{
						RMLPipeline::bind(ctx, RMLPipeline::color(), instance->transform, instance->translation);
					}

					ctx->draw_indexed(RHITopology::TriangleList, instance->geometry->count(), 0, 0);
				};

				list->AddCallback(callback, &instance, sizeof(instance));
			}

			void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override { trx_delete RMLGeometry::from(geometry); }

			Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
			{
				rml_stub_log();
				(void) source;
				texture_dimensions = {};
				return 0;
			}

			Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i size) override
			{
				auto texture = trx_new RMLTextureCopy({size.x, size.y}, RHITextureFlags::ShaderResource);

				RHITextureRegion dst = RHITextureRegion({size.x, size.y, 1});

				auto ctx = context();

				ctx->barrier(texture->texture(), RHIAccess::TransferDst);
				ctx->update(texture->texture(), dst, source.data(), {.size = source.size()});
				ctx->barrier(texture->texture(), RHIAccess::SRVGraphics);

				return texture->handle();
			}

			void ReleaseTexture(Rml::TextureHandle texture) override
			{
				rml_stub_log();
				(void) texture;
			}

			void EnableScissorRegion(bool enable) override
			{
				rml_stub_log();
				(void) enable;
			}

			void SetScissorRegion(Rml::Rectanglei region) override
			{
				rml_stub_log();
				(void) region;
			}
		};
	}// namespace

	trinex_on_init()
	{
		static RMLRenderInterface interface;
		Rml::SetRenderInterface(&interface);
	}

	trinex_on_shutdown()
	{
		Rml::SetRenderInterface(nullptr);
	}
}// namespace Trinex::UI::RML::Backend

#undef rml_stub_log
