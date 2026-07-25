#pragma once
#include <Core/etl/registry.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Core/viewport_client.hpp>
#include <UI/menu_bar.hpp>

struct ImGuiContext;

namespace Trinex::UI
{
	class Document;

	class Client : public ViewportClient
	{
		trinex_class(Client, ViewportClient);

	private:
		struct DocumentEntry {
			Document* document = nullptr;
			Path path;
			Identifier watch_id = 0;
		};

		ImGuiContext* m_ctx        = nullptr;
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
		bool reload_document(Document* document);
		Client& add_document(Document* document);
		Client& remove_document(Document* document);

		//virtual Client& setup_dockspace(DockLayout& layout);
		virtual Client& select(Object* object);
		virtual Client& update(float dt);

		inline ImGuiContext* context() const { return m_ctx; }
		inline RenderViewport* viewport() const { return m_viewport; }
	};

	class ClientListener : public Registry<ClientListener>
	{
		trinex_registry(ClientListener);

	private:
		static bool m_dirty;
		u64 m_sort_index;

		static ClientListener* update_state(ClientListener*& value);

	public:
		ClientListener(u64 sort_index = ~0);

		static inline ClientListener* first() { return update_state(s_first); }
		static inline ClientListener* last() { return update_state(s_last); }

		virtual ClientListener& on_create(Client* client);
		virtual ClientListener& on_destroy(Client* client);

		virtual ClientListener& on_begin_frame(Client* client);
		virtual ClientListener& on_end_frame(Client* client);

		virtual ClientListener& on_render(Client* client);

		template<ClientListener& (ClientListener::*method)(Client*)>
		static inline void for_each(Client* client)
		{
			ClientListener* listener = ClientListener::first();

			while (listener)
			{
				(listener->*method)(client);
				listener = listener->next();
			}
		}

		template<ClientListener& (ClientListener::*method)(Client*)>
		static inline void reverse_for_each(Client* client)
		{
			ClientListener* listener = ClientListener::last();

			while (listener)
			{
				(listener->*method)(client);
				listener = listener->prev();
			}
		}
	};
}// namespace Trinex::UI
