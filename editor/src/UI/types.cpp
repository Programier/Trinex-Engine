#include <UI/types.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	namespace
	{
		class FunctionWidget : public Widget
		{
		private:
			Action m_action;

		public:
			FunctionWidget(StringView name, const WindowOptions& options, bool open, const Action& action)
			    : Widget(name, options, open), m_action(action)
			{}

			void on_render() override { m_action(); }
		};
	}// namespace


	ID ID::from(StringView id)
	{
		return ImGui::GetID(id.data(), id.data() + id.size());
	}

	ID ID::from(const void* id)
	{
		return ImGui::GetID(id);
	}

	ID ID::from(ValueType id)
	{
		return ImGui::GetID(id);
	}

	trinex_implement_registry(ContextListener);

	bool ContextListener::m_dirty = false;

	ContextListener::ContextListener(u64 sort_index) : m_sort_index(sort_index)
	{
		m_dirty = true;
	}

	ContextListener* ContextListener::update_state(ContextListener*& value)
	{
		if (m_dirty)
		{
			ContextListener::sort([](ContextListener* first, ContextListener* second) -> bool {
				return first->m_sort_index < second->m_sort_index;
			});

			m_dirty = false;
		}

		return value;
	}

	ContextListener& ContextListener::on_create(Context* context)
	{
		return *this;
	}

	ContextListener& ContextListener::on_destroy(Context* context)
	{
		return *this;
	}

	ContextListener& ContextListener::on_begin_frame(Context* context)
	{
		return *this;
	}

	ContextListener& ContextListener::on_end_frame(Context* context)
	{
		return *this;
	}

	ContextListener& ContextListener::on_render(Context* context)
	{
		return *this;
	}

	Widget* Widget::create(StringView name, const WindowOptions& options, bool open, const Action& action)
	{
		return trx_new FunctionWidget(name, options, open, action);
	}

	Widget::Widget(StringView name, const WindowOptions& options, bool open) : m_name(name), m_options(options), m_is_open(open)
	{}

	Widget::~Widget() {}

	void Widget::on_attach(Context* context) {}

	void Widget::on_deattach(Context* context) {}

	void Widget::on_open() {}

	void Widget::on_close() {}

	void Widget::on_render() {}

}// namespace Trinex::UI
