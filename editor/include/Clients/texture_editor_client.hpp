#pragma once
#include <Clients/imgui_client.hpp>

namespace Engine
{
	class Texture2D;
	class Sampler;

	class TextureEditorClient : public ImGuiEditorClient
	{
		declare_class(TextureEditorClient, ImGuiEditorClient);

		Pointer<Texture2D> m_texture;
		Pointer<Sampler> m_sampler;

		void render_dock();
		void render_texture();
		void render_properties();

	public:
		TextureEditorClient();
		TextureEditorClient& update(float dt) override;
		TextureEditorClient& select(Object* object) override;
	};
}// namespace Engine
