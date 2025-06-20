#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/enums.hpp>

namespace Engine
{
	trinex_implement_engine_enum(ViewMode, Lit, Unlit, Wireframe, WorldNormal, Metalic, Roughness, Specular, AO);
}
