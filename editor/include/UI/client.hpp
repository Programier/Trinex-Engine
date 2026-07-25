#pragma once
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Core/viewport_client.hpp>
#include <UI/menu_bar.hpp>

namespace Trinex::UI
{
	class Context;
	class Document;
	class DockLayout;

	class Client : public ViewportClient
	{
		trinex_class(Client, ViewportClient);

	private:
		struct DocumentEntry {
			Document* document = nullptr;
			Path path;
			Identifier watch_id = 0;
		};

		Context* m_ctx             = nullptr;
		RenderViewport* m_viewport = nullptr;
		Vector<DocumentEntry> m_documents;

	public:
		MenuBar menu_bar;

	public:
		static bool register_client(Trinex::Refl::Class* object_type, Trinex::Refl::Class* renderer);
		static Client* client_of(Trinex::Refl::Class* object_type, bool create_if_not_exist = false);

		Client& attach(class RenderViewport* viewport) override;
		Client& deattach(class RenderViewport* viewport) override;
		Client& update(class RenderViewport* viewport, float dt) final override;

		Document* load_document(const Path& path);
		Document* create_document(StringView source);
		bool reload_document(Document* document);
		Client& add_document(Document* document);
		Client& remove_document(Document* document);

		virtual Client& setup_dockspace(DockLayout& layout);
		virtual Client& select(Object* object);
		virtual Client& update(float dt);

		inline Context* context() const { return m_ctx; }
		inline RenderViewport* viewport() const { return m_viewport; }
	};
}// namespace Trinex::UI
