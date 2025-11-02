#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/enums.hpp>

namespace Engine
{
	trinex_implement_engine_enum(ShowFlags, 0, Statistics, PointLights, SpotLights, DirectionalLights, PostProcess, StaticMesh,
	                             PrimitiveBounds, PrimitiveOctree);
	trinex_implement_engine_enum(ViewMode, 0, Lit, Unlit, Wireframe, WorldNormal, Metalic, Roughness, Specular, Emissive, AO);
	trinex_implement_engine_enum(CameraProjectionMode, 0, Perspective, Orthographic);
}// namespace Engine
