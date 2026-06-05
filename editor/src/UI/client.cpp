#include <Core/reflection/class.hpp>
#include <Graphics/render_viewport.hpp>
#include <UI/api.hpp>
#include <UI/client.hpp>

namespace Trinex::UI
{
	trinex_implement_class(Trinex::UI::Client, 0) {}

	bool Client::register_client(Refl::Class* object_type, Refl::Class* renderer)
	{
		return false;
	}

	Client* Client::client_of(Refl::Class* object_type, bool create_if_not_exist)
	{
		return nullptr;
	}

	Client& Client::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		m_ctx      = UI::create_context(viewport->window());
		m_viewport = viewport;
		return *this;
	}

	Client& Client::deattach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		UI::destroy_context(m_ctx);
		m_ctx      = nullptr;
		m_viewport = viewport;
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
				DockLayoutBuilder builder;
				if (builder.begin(options.id, options.size, options.flags))
				{
					setup_dockspace(builder);
					builder.end();
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
			UI::end_frame();
		}

		return *this;
	}

	Client& Client::update(float dt)
	{
		return *this;
	}

	Client& Client::setup_dockspace(DockLayoutBuilder& builder)
	{
		return *this;
	}

	Client& Client::select(Object* object)
	{
		return *this;
	}
}// namespace Trinex::UI
