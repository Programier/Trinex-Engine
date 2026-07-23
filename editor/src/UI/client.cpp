#include <Core/etl/algorithm.hpp>
#include <Core/file_manager.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/render_viewport.hpp>
#include <UI/Elements/document.hpp>
#include <UI/api.hpp>
#include <UI/client.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Trinex::UI
{
	namespace
	{
		static Map<Trinex::Refl::Class*, Client*>& opened_clients()
		{
			static Map<Trinex::Refl::Class*, Client*> value;
			return value;
		}

		static Map<Trinex::Refl::Class*, Trinex::Refl::Class*>& registered_clients()
		{
			static Map<Trinex::Refl::Class*, Trinex::Refl::Class*> value;
			return value;
		}

		static Client* open_client(Trinex::Refl::Class* client_class)
		{
			if (client_class == nullptr)
				return nullptr;

			WindowConfig config;
			config.client = client_class->full_name();

			if (Window* window = WindowManager::instance()->create_window(config))
			{
				return Object::instance_cast<Client>(window->render_viewport()->client());
			}

			return nullptr;
		}
	}// namespace

	trinex_implement_class(Trinex::UI::Client, 0) {}

	bool Client::register_client(Trinex::Refl::Class* object_type, Trinex::Refl::Class* renderer)
	{
		if (object_type == nullptr || renderer == nullptr)
			return false;

		registered_clients().insert({object_type, renderer});
		return true;
	}

	Client* Client::client_of(Trinex::Refl::Class* object_type, bool create_if_not_exist)
	{
		Trinex::Refl::Class* client_class = nullptr;

		while (object_type && client_class == nullptr)
		{
			auto it = registered_clients().find(object_type);

			if (it != registered_clients().end())
			{
				client_class = it->second;
				break;
			}

			object_type = object_type->parent();
		}

		if (client_class)
		{
			auto it = opened_clients().find(client_class);

			if (it != opened_clients().end())
				return it->second;

			if (create_if_not_exist)
				return open_client(client_class);
		}

		return nullptr;
	}

	Client& Client::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		m_ctx      = UI::create_context(viewport->window());
		m_viewport = viewport;
		opened_clients().insert({class_instance(), this});
		return *this;
	}

	Client& Client::deattach(class RenderViewport* viewport)
	{
		opened_clients().erase(class_instance());

		for (Document* document : m_documents)
		{
			document->release();
		}

		m_documents.clear();

		Super::deattach(viewport);
		UI::destroy_context(m_ctx);
		m_ctx      = nullptr;
		m_viewport = nullptr;
		return *this;
	}

	Client& Client::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);

		if (UI::begin_frame(m_ctx))
		{
			DockLayoutOptions options = {};

			if (UI::begin_viewport_dockspace(options))
			{
				DockLayout layout;
				if (layout.begin(options.id, options.size, options.flags))
				{
					setup_dockspace(layout);
					layout.end();
				}

				UI::end_viewport_dockspace();
			}

			if (!menu_bar.is_empty())
			{
				if (UI::begin_main_menu_bar())
				{
					menu_bar.render();
					UI::end_main_menu_bar();
				}
			}

			update(dt);

			for (Document* document : m_documents)
			{
				if (document->is_open())
				{
					document->update();
				}
			}

			UI::end_frame();
		}

		return *this;
	}

	Document* Client::load_document(const Path& path)
	{
		FileReader reader(path);

		if (!reader.is_open())
		{
			trinex_error(Log::Editor, "Failed to open UI document '%s'", path.c_str());
			return nullptr;
		}

		return create_document(reader.read_string());
	}

	Document* Client::create_document(StringView source)
	{
		Document* document = UI::create_document(source);

		if (document)
		{
			add_document(document);
			document->release();
		}

		return document;
	}

	Client& Client::add_document(Document* document)
	{
		if (document)
		{
			document->add_reference();
			m_documents.push_back(document);
		}

		return *this;
	}

	Client& Client::remove_document(Document* document)
	{
		auto it = etl::find(m_documents.begin(), m_documents.end(), document);

		if (it != m_documents.end())
		{
			m_documents.erase(it);
			document->release();
		}

		return *this;
	}

	Client& Client::update(float dt)
	{
		return *this;
	}

	Client& Client::setup_dockspace(DockLayout& layout)
	{
		return *this;
	}

	Client& Client::select(Object* object)
	{
		return *this;
	}
}// namespace Trinex::UI
