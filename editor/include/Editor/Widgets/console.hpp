#pragma once
#include <UI/widget.hpp>

namespace Trinex
{
	class ConsoleWidget : public UI::Widget
	{
	public:
		ConsoleWidget(const UI::WindowOptions& options = {}, bool open = false);
	};
}// namespace Trinex
