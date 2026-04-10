import "trinex/trinex.slang";
import "trinex/scene_view.slang";
import "trinex/scene.slang";

struct VertexOutput
{
	float4 screen : SV_Position;
	float3 world : WORLD;
}

float3 load_vertex(uint vertex_id, uint primitive_id)
{
	let primitive = scene_view.scene[0].primitives[primitive_id];

	if(primitive.flags & Scene::Primitive::IsActive)
	{
		
	}

	float4x4 local_to_world = scene_view.scene[0].heap.Load<float4x4>(primitive.transform);

	let geometry = scene_view.scene[0].geometries[primitive.geometry];
	let index    = geometry.index_stream.load<uint>(vertex_id);
	
	float4 vertex = local_to_world * float4(geometry.vertex_stream.load<float3>(index), 1.f);
	return vertex.xyz / vertex.w;
}

[shader("vertex")]
VertexOutput vertex_main(uint index : SV_VertexID, uint instance : SV_InstanceID)
{
	VertexOutput output;

	float3 vertex = load_vertex(index, instance);
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