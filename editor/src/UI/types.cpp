#include <UI/types.hpp>
#include <imgui.h>

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


	Widget::~Widget() {}

	void Widget::on_init() {}

	void Widget::on_render() {}

	void Widget::on_close() {}
}// namespace Trinex::UI
