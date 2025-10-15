#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/enums.hpp>

namespace Engine
{
	trinex_implement_engine_enum(ShowFlags, Statistics, PointLights, SpotLights, DirectionalLights, PostProcess, StaticMesh,
	                             PrimitiveBounds);
	trinex_implement_engine_enum(ViewMode, Lit, Unlit, Wireframe, WorldNormal, Metalic, Roughness, Specular, Emissive, AO);
	trinex_implement_engine_enum(CameraProjectionMode, Perspective, Orthographic);
}// namespace Engine
