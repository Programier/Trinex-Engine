#include <Editor/Widgets/console.hpp>
#include <UI/Elements/document.hpp>

namespace Trinex
{
	ConsoleWidget::ConsoleWidget(const UI::WindowOptions& options, bool open) : Widget("Console", options, open)
	{
		if (UI::Document* document = load_document("[ui]:/TrinexEditor/widgets/console.ui"))
		{
			document->open();
		}
	}
}// namespace Trinex
