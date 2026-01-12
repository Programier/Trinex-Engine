#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/etl/variant.hpp>

namespace Engine
{
	class Texture2D;
	class Texture3D;
	class TextureCube;
	class RenderSurface;
	class PropertyRenderer;

	class TextureEditorClient : public ImGuiViewportClient
	{
		trinex_class(TextureEditorClient, ImGuiViewportClient);

	private:
		using TextureVariant = Variant<Pointer<Texture2D>, Pointer<Texture3D>, Pointer<TextureCube>, Pointer<RenderSurface>>;

		PropertyRenderer* m_properties = nullptr;

		TextureVariant m_texture;

		Vector4b m_mask             = {true, true, true, true};
		Vector2f m_range            = {0.f, 1.f};
		Vector2f m_translate        = {0.f, 0.f};
		Vector2f m_smooth_translate = {0.f, 0.f};
		float m_scale               = 1.f;
		float m_smooth_scale        = 1.f;
		uint_t m_mip                = 0;

	public:
		TextureEditorClient();
		TextureEditorClient& on_bind_viewport(RenderViewport* vp) override;
		TextureEditorClient& update(float dt) override;
		TextureEditorClient& select(Object* object) override;
		uint32_t build_dock(uint32_t dock_id) override;
		Matrix4f build_projection(Vector2u texture_size, Vector2u viewport_size) const;

		TextureEditorClient& rhi_render(Texture2D* texture, Vector2u size);
		TextureEditorClient& rhi_render(Texture3D* texture, Vector2u size);
		TextureEditorClient& rhi_render(TextureCube* texture, Vector2u size);
		TextureEditorClient& rhi_render(RenderSurface* texture, Vector2u size);

		inline Vector4f mask() const { return Vector4f(m_mask); }
		inline Vector2f range() const { return m_range; }
		inline Vector2f translate() const { return m_smooth_translate; }
		inline float scale() const { return m_smooth_scale; }
		inline uint_t mip() const { return m_mip; }
	};
}// namespace Engine
