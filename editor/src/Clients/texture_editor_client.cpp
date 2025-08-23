#include <Clients/texture_editor_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	class TextureView : public GlobalGraphicsPipeline
	{
	private:
		const RHIShaderParameterInfo* m_texture   = nullptr;
		const RHIShaderParameterInfo* m_transform = nullptr;
		const RHIShaderParameterInfo* m_mask      = nullptr;
		const RHIShaderParameterInfo* m_range     = nullptr;

	public:
		using GlobalGraphicsPipeline::GlobalGraphicsPipeline;

		void initialize() override
		{
			m_texture   = find_parameter("texture");
			m_transform = find_parameter("transform");
			m_mask      = find_parameter("mask");
			m_range     = find_parameter("range");
		}

		inline TextureView& update_texture(RHITexture* texture)
		{
			rhi->bind_srv(texture->as_srv(), m_texture->binding);
			rhi->bind_sampler(RHIPointWrapSampler::static_sampler(), m_texture->binding);
			return *this;
		}

		inline TextureView& update_transform(const Matrix4f& transform)
		{
			rhi->update_scalar(transform, m_transform);
			return *this;
		}

		inline TextureView& update_mask(const Vector4f& mask)
		{
			rhi->update_scalar(mask, m_mask);
			return *this;
		}

		inline TextureView& update_range(Vector2f range)
		{
			range = {range.x, 1.f / (range.y - range.x)};
			rhi->update_scalar(range, m_range);
			return *this;
		}
	};

	class TextureView2D : public TextureView
	{
		trinex_declare_pipeline(TextureView2D, TextureView);

	private:
		const RHIShaderParameterInfo* m_mip = nullptr;

	public:
		TextureView2D& update_mip(uint_t mip)
		{
			rhi->update_scalar(static_cast<float>(mip), m_mip);
			return *this;
		}

		TextureView2D& modify_compilation_env(ShaderCompilationEnvironment* env) override
		{
			Super::modify_compilation_env(env);
			env->add_module("editor/texture_view/2d.slang");
			return *this;
		}
	};

	trinex_implement_pipeline(TextureView2D, "[shaders_dir]:/TrinexEditor/editor/texture_view/view.slang")
	{
		Super::initialize();
		m_mip = find_parameter("mip");
	}

	class TextureViewCube : public TextureView
	{
		trinex_declare_pipeline(TextureViewCube, TextureView);

	private:
		const RHIShaderParameterInfo* m_mip  = nullptr;
		const RHIShaderParameterInfo* m_face = nullptr;

	public:
		TextureViewCube& update_mip(uint_t mip)
		{
			rhi->update_scalar(static_cast<float>(mip), m_mip);
			return *this;
		}

		TextureViewCube& update_face(uint_t face)
		{
			rhi->update_scalar(face, m_face);
			return *this;
		}

		TextureViewCube& modify_compilation_env(ShaderCompilationEnvironment* env) override
		{
			Super::modify_compilation_env(env);
			env->add_module("editor/texture_view/cube.slang");
			return *this;
		}
	};

	trinex_implement_pipeline(TextureViewCube, "[shaders_dir]:/TrinexEditor/editor/texture_view/view.slang")
	{
		Super::initialize();
		m_mip  = find_parameter("mip");
		m_face = find_parameter("face");
	}


	static inline void render_texture_2d(RHITexture* src, const Matrix4f& transform, Vector2f range, uint_t level,
	                                     const Vector4f& mask)
	{
		auto pipeline = TextureView2D::instance();

		pipeline->rhi_bind();
		pipeline->update_mip(level).update_texture(src).update_transform(transform).update_mask(mask).update_range(range);
		rhi->draw(6, 0);
	}

	static inline void render_texture_cube(RHITexture* src, const Matrix4f& transform, Vector2f range, uint_t level,
	                                       const Vector4f& mask)
	{
		auto pipeline = TextureViewCube::instance();

		static const Matrix4f face_scale = Math::scale(Matrix4f(1.f), Vector3f(1.f / 4.f, 1.f / 3.f, 1.f));

		const Vector2f face_offsets[6] = {
		        {0.25f, 0.0f},        // +X  (col=2, row=1)
		        {-0.75f, 0.0f},       // -X  (col=0, row=1)
		        {-0.25f, 0.6666667f}, // +Y  (col=1, row=0)
		        {-0.25f, -0.6666667f},// -Y  (col=1, row=2)
		        {-0.25f, 0.0f},       // +Z  (col=1, row=1)
		        {0.75f, 0.0f},        // -Z  (col=3, row=1)
		};


		for (uint_t face = 0; face < 6; ++face)
		{
			pipeline->rhi_bind();
			const Matrix4f face_translate = Math::translate(Matrix4f(1.f), Vector3f(face_offsets[face], 0.f));

			pipeline->update_face(face)
			        .update_mip(level)
			        .update_texture(src)
			        .update_transform(transform * (face_translate * face_scale))
			        .update_mask(mask)
			        .update_range(range);

			rhi->draw(6, 0);
		}
	}


	struct TextureViewVisitor {
		TextureEditorClient* client;
		Vector2u pos;
		Vector2u size;

		void operator()(const Pointer<Texture2D>& texture) const { client->rhi_render(texture, size); }
		void operator()(const Pointer<TextureCube>& texture) const { client->rhi_render(texture, size); }
		void operator()(const Pointer<RenderSurface>& texture) const { client->rhi_render(texture, size); }
	};

	trinex_implement_engine_class(TextureEditorClient, 0)
	{
		register_client(Texture2D::static_reflection(), static_reflection());
		register_client(TextureCube::static_reflection(), static_reflection());
		register_client(RenderSurface::static_reflection(), static_reflection());
	}

	TextureEditorClient::TextureEditorClient()
	{
		menu_bar.create("")->actions.push([this]() {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			static ImU32 colors[4] = {IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255),
			                          IM_COL32(255, 255, 255, 255)};

			float height                       = ImGui::GetContentRegionAvail().y;
			static const char* channel_names[] = {"###red", "###green", "###blue", "###alpha"};

			for (int i = 0; const char* name : channel_names)
			{
				bool* swizzle = &m_mask.x;
				if (ImGui::Selectable(name, swizzle[i], 0, {height, height}))
				{
					swizzle[i] = !swizzle[i];
				}

				auto min = ImGui::GetItemRectMin();
				auto max = ImGui::GetItemRectMax();

				auto center = (max + min) * 0.5f;

				if (swizzle[i])
					draw_list->AddCircleFilled(center, height / 2.f, colors[i]);
				else
					draw_list->AddCircle(center, height / 2.f, colors[i], 0, 2.f);

				++i;
			}

			ImGui::Text("Range");
			ImGui::PushItemWidth(ImGui::GetFontSize() * 5);
			ImGui::SliderFloat("##range1", &m_range.x, 0.f, m_range.y, "%.2f");
			ImGui::SliderFloat("##range2", &m_range.y, m_range.x, 1.f, "%.2f");
			ImGui::InputScalar("Mip", ImGuiDataType_U32, &m_mip);
			ImGui::PopItemWidth();
		});
	}

	TextureEditorClient& TextureEditorClient::on_bind_viewport(RenderViewport* vp)
	{
		Super::on_bind_viewport(vp);
		auto current = ImGuiWindow::current();
		ImGuiWindow::make_current(window());
		m_properties           = window()->widgets.create<PropertyRenderer>();
		m_properties->closable = false;

		ImGuiWindow::make_current(current);
		return *this;
	}

	TextureEditorClient& TextureEditorClient::update(float dt)
	{
		Super::update(dt);

		ImGui::Begin("Texture View###texture");
		{
			auto viewport_size      = ImGui::GetContentRegionAvail();
			auto half_viewport_size = viewport_size * 0.5f;
			auto cursor_pos         = ImGui::GetCursorScreenPos();
			auto viewport_pos       = cursor_pos - ImGui::GetCurrentWindow()->Viewport->Pos;

			TextureViewVisitor args;
			args.client = this;
			args.pos    = Vector2u(viewport_pos.x, viewport_pos.y);
			args.size   = Vector2u(viewport_size.x, viewport_size.y);

			ImDrawCallback callback = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
				TextureViewVisitor* args = reinterpret_cast<TextureViewVisitor*>(cmd->UserCallbackData);
				rhi->viewport(RHIViewport(args->size, args->pos));
				rhi->scissor(RHIScissors(args->size, args->pos));
				std::visit(*args, args->client->m_texture);
			};

			auto list = ImGui::GetWindowDrawList();
			list->AddCallback(callback, &args, sizeof(args));
			list->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

			ImGui::Dummy(viewport_size);

			if (ImGui::IsItemHovered())
			{
				ImGuiIO& io = ImGui::GetIO();

				if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				{
					Vector2f offset = Vector2f(io.MouseDelta.x / half_viewport_size.x, io.MouseDelta.y / half_viewport_size.y);
					m_translate += offset;
					m_smooth_translate += offset;
				}

				if (io.MouseWheel != 0.0f)
				{
					ImVec2 mouse_pos = ImGui::GetMousePos() - cursor_pos;
					Vector2f ndc = Vector2f(mouse_pos.x / half_viewport_size.x - 1.0f, mouse_pos.y / half_viewport_size.y - 1.0f);

					float prev_scale = m_scale;
					float scale      = Math::max(Math::exp(io.MouseWheel * 0.2f), 0.0001f);

					m_scale *= scale;
					m_translate += -ndc * (1.0f - prev_scale / m_scale);
					m_translate *= scale;
				}
			}
		}

		ImGui::End();

		m_smooth_scale     = Math::lerp(m_smooth_scale, m_scale, dt * 12.f);
		m_smooth_translate = Math::lerp(m_smooth_translate, m_translate, dt * 12.f);
		return *this;
	}

	TextureEditorClient& TextureEditorClient::select(Object* object)
	{
		if (TextureCube* texture = instance_cast<TextureCube>(object))
		{
			m_texture = texture;
		}
		else if (Texture2D* texture = instance_cast<Texture2D>(object))
		{
			m_texture = texture;
		}
		else if (RenderSurface* surface = instance_cast<RenderSurface>(object))
		{
			m_texture = surface;
		}
		else
		{
			return *this;
		}

		Super::select(object);
		m_properties->object(object);
		m_translate        = {0.f, 0.f};
		m_smooth_translate = {0.f, 0.f};
		m_scale            = 1.f;
		m_smooth_scale     = 1.f;
		return *this;
	}

	uint32_t TextureEditorClient::build_dock(uint32_t dock_id)
	{
		auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
		ImGui::DockBuilderDockWindow("Texture View###texture", dock_id);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right);
		return dock_id;
	}

	Matrix4f TextureEditorClient::build_projection(Vector2u texture_size, Vector2u viewport_size) const
	{
		float aspect_tex = static_cast<float>(texture_size.x) / static_cast<float>(texture_size.y);
		float aspect_vp  = static_cast<float>(viewport_size.x) / static_cast<float>(viewport_size.y);

		float scale_x = m_smooth_scale;
		float scale_y = m_smooth_scale;

		if (aspect_tex > aspect_vp)
		{
			scale_y *= aspect_vp / aspect_tex;
		}
		else if (aspect_tex < aspect_vp)
		{
			scale_x *= aspect_tex / aspect_vp;
		}

		Matrix4f translate = Math::translate(Matrix4f(1.0f), Vector3f(m_smooth_translate, 0.f));
		Matrix4f scale     = Math::scale(translate, Vector3f(scale_x, scale_y, 1.0f));
		return scale;
	}

	TextureEditorClient& TextureEditorClient::rhi_render(Texture2D* texture, Vector2u size)
	{
		if (texture == nullptr)
			return *this;

		Matrix4f projection = build_projection(texture->size(0), size);
		render_texture_2d(texture->rhi_texture(), projection, range(), mip(), mask());
		return *this;
	}

	TextureEditorClient& TextureEditorClient::rhi_render(TextureCube* texture, Vector2u size)
	{
		if (texture == nullptr)
			return *this;

		Matrix4f projection = build_projection(texture->size(0) * Vector2u(4, 3), size);
		render_texture_cube(texture->rhi_texture(), projection, range(), 0, mask());
		return *this;
	}

	TextureEditorClient& TextureEditorClient::rhi_render(RenderSurface* texture, Vector2u size)
	{
		if (texture == nullptr)
			return *this;

		Matrix4f projection = build_projection(texture->size(), size);
		render_texture_2d(texture->rhi_texture(), projection, range(), 0, mask());
		return *this;
	}
}// namespace Engine
