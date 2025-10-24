#include <Core/engine_loading_controllers.hpp>
#include <Core/enums.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine
{
	trinex_implement_engine_enum(OperationSystemType, Refl::Enum::IsScriptable, Linux, Windows, Android);
	trinex_implement_engine_enum(PhysicalSizeMetric, Refl::Enum::IsScriptable, Inch, Сentimeters);

	trinex_implement_engine_enum(WindowAttribute, Refl::Enum::IsScriptable, None, Resizable, FullScreen, Shown, Hidden,
	                             BorderLess, MouseFocus, InputFocus, InputGrabbed, Minimized, Maximized, MouseCapture,
	                             MouseGrabbed, KeyboardGrabbed);

	trinex_implement_engine_enum(CursorMode, Refl::Enum::IsScriptable, Normal, Hidden);
	trinex_implement_engine_enum(Orientation, Refl::Enum::IsScriptable, Landscape, LandscapeFlipped, Portrait, PortraitFlipped);
	trinex_implement_engine_enum(MessageBoxType, Refl::Enum::IsScriptable, Error, Warning, Info);
	trinex_implement_engine_enum(MaterialDomain, Refl::Enum::IsScriptable, Surface, PostProcess);
	trinex_implement_engine_enum(SplashTextType, Refl::Enum::IsScriptable, StartupProgress, VersionInfo, CopyrightInfo, GameName);
}// namespace Engine
