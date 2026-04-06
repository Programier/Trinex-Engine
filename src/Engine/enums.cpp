#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/enums.hpp>

namespace Trinex
{
	trinex_implement_engine_enum(ShowFlags, 0, Statistics, PointLights, SpotLights, DirectionalLights, PostProcess, StaticMesh,
	                             PrimitiveBounds);
	trinex_implement_engine_enum(ViewMode, 0, Lit, Unlit, Wireframe, WorldNormal, Emissive, Metalic, Specular, Roughness, AO,
	                             Velocity, Depth);
	trinex_implement_engine_enum(CameraProjectionMode, 0, Perspective, Orthographic);
}// namespace Trinex
