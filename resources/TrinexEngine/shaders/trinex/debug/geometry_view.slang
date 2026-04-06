import "trinex/trinex.slang";
import "trinex/scene_view.slang";

struct VertexOutput
{
	float4 screen : SV_Position;
	float3 world : WORLD;
}

static const float3 s_positions[8] = {
	float3(-0.5, -0.5, -0.5), float3(0.5, -0.5, -0.5), float3(0.5, 0.5, -0.5), float3(-0.5, 0.5, -0.5),
	float3(-0.5, -0.5, 0.5),  float3(0.5, -0.5, 0.5),  float3(0.5, 0.5, 0.5),  float3(-0.5, 0.5, 0.5),
};

static const uint s_indices[36] = {
	0, 1, 2, 2, 3, 0,// Front
	1, 5, 6, 6, 2, 1,// Right
	5, 4, 7, 7, 6, 5,// Back
	4, 0, 3, 3, 7, 4,// Left
	3, 2, 6, 6, 7, 3,// Top
	4, 5, 1, 1, 0, 4 // Bottom
};

float3 load_vertex(uint id)
{
	float4x4 local_to_world = scene_view.scene[0].transforms[0];
	float4 vertex = local_to_world * float4(s_positions[s_indices[id]], 1.f);
	return vertex.xyz / vertex.w;
}

[shader("vertex")]
VertexOutput vertex_main(uint index : SV_VertexID)
{
	VertexOutput output;

	float3 vertex = load_vertex(index);
	output.world = vertex;
	output.screen = scene_view.camera.world_to_clip(vertex);
	return output;
}

[shader("fragment")]
float4 fragment_main(VertexOutput input) : SV_Target 
{
	float3 dx = ddx(input.world);
	float3 dy = ddy(input.world);

	float3 normal  = normalize(cross(dx, dy));
	float3 view    = normalize( scene_view.camera.location - input.world);
	float brightness = pow(abs(dot(normal, view)), 2.f);
	return float4(brightness, brightness, brightness, 1.0);
}