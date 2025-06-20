#include <Core/engine_loading_controllers.hpp>
#include <Core/enums.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine
{
	trinex_implement_engine_enum(OperationSystemType, Linux, Windows, Android);
	trinex_implement_engine_enum(PhysicalSizeMetric, Inch, Сentimeters);

	trinex_implement_engine_enum(WindowAttribute, None, Resizable, FullScreen, Shown, Hidden, BorderLess, MouseFocus, InputFocus,
	                             InputGrabbed, Minimized, Maximized, MouseCapture, MouseGrabbed, KeyboardGrabbed);

	trinex_implement_engine_enum(CursorMode, Normal, Hidden);
	trinex_implement_engine_enum(Orientation, Landscape, LandscapeFlipped, Portrait, PortraitFlipped);
	trinex_implement_engine_enum(MessageBoxType, Error, Warning, Info);
	trinex_implement_engine_enum(MaterialDomain, Surface, PostProcess);
	trinex_implement_engine_enum(SplashTextType, StartupProgress, VersionInfo, CopyrightInfo, GameName);
}// namespace Engine
