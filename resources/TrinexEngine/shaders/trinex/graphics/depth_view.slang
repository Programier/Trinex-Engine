import "trinex/scene_view.slang";
import "trinex/quad.slang";
import "trinex/math.slang";

uniform Texture2D<float> scene_depth;

[shader("vertex")]
float4 vertex_main(in uint vertex: SV_VertexID) : SV_Position
{
    return FullScreenQuad::vertex_by_index(vertex);
}

[shader("fragment")]
float4 fragment_main(in float4 position: SV_Position) : SV_Target
{
	const int2 coord  = scene_view.screen_coord(position.xy);
	// const float near  = scene_view.camera.near;
	// const float far   = scene_view.camera.far;

	// const float depth = Math::linearize_depth(scene_depth.Load(int3(coord, 0)), near, far);
	return float4(float3(pow(scene_depth.Load(int3(coord, 0)), 1.f / 8.f)), 1.f);
}
