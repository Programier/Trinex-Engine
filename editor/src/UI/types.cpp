#include <imgui.h>
#include <UI/types.hpp>

namespace Trinex::UI
{
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

}// namespace Trinex::UI
