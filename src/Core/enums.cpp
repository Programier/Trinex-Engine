#include <Core/enums.hpp>
#include <Core/reflection/enum.hpp>

namespace Trinex
{
	trinex_implement_engine_enum(OperationSystemType, Refl::Enum::IsScriptable, Linux, Windows, Android);
	trinex_implement_engine_enum(PhysicalSizeMetric, Refl::Enum::IsScriptable, Inch, Сentimeters);

	// trinex_implement_engine_enum(WindowAttribute, Refl::Enum::IsScriptable, Undefined, Fullscreen, Hidden, Borderless, Resizable,
	//                              Minimized, Maximized, Occluded, InputFocus, MouseFocus, MouseGrabbed, KeyboardGrabbed,
	//                              MouseCapture, MouseRelativeMode, AlwaysOnTop, HighPixelDensity, Transparent, NotFocusable, Modal,
	//                              Utility, Tooltip, PopupMenu, External, OpenGL, Vulkan, Metal);

	trinex_implement_engine_enum(CursorMode, Refl::Enum::IsScriptable, Normal, Hidden);
	trinex_implement_engine_enum(Orientation, Refl::Enum::IsScriptable, Landscape, LandscapeFlipped, Portrait, PortraitFlipped);
	trinex_implement_engine_enum(MessageBoxType, Refl::Enum::IsScriptable, Error, Warning, Info);
	trinex_implement_engine_enum(SplashTextType, Refl::Enum::IsScriptable, StartupProgress, VersionInfo, CopyrightInfo, GameName);
}// namespace Trinex
