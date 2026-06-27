#pragma once
#include <UI/types.hpp>

namespace Trinex
{
	class ConsoleWidget : public UI::Widget
	{
	public:
		ConsoleWidget(const UI::WindowOptions& options = {}, bool open = false);
		void on_render() override;
	};
}// namespace Trinex
