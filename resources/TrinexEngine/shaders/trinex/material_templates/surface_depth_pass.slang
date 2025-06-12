import "trinex/attributes.slang";
import "trinex/material.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";

extern struct VertexInput : IVertexInput { }
extern Material main<Material : IMaterial>(in IMaterialInput input);

[parameter_type("LocalToWorld")]
uniform float4x4 local_to_world;

[shader("vertex")]
float4 vs_main(
	in float3 vs_position: POSITION0,
	in float3 vs_normal: NORMAL0,
	in float3 vs_tangent: TANGENT0,
    in VertexInput vs_input) : SV_Position
{
	struct MaterialInput : IMaterialInput
	{
		property float3 position { get { return vs_position; } }
		property float3 normal { get { return vs_normal; } }
		property float3 tangent { get { return vs_tangent; } }
		property float4x4 local_to_world { get { return ::local_to_world; } }
		property IVertexInput vertex_input { get { return vs_input; } }

		float4 sv_position;
		float4 screen_position;

		float3 world_position;
		float3 world_normal;
		float3 world_tangent;
		float3x3 TBN;

		float sv_depth;
	}

	MaterialInput material_input;

	float4 world_position = mul(local_to_world, float4(vs_position, 1.0f));
	float3x3 rotation     = transpose(Math::inverse((float3x3) local_to_world));

	material_input.world_position = world_position.xyz / world_position.w;
	material_input.world_normal  = normalize(mul(rotation, vs_normal));
	material_input.world_tangent = normalize(mul(rotation, vs_tangent));

	float3 world_bitangent = cross(material_input.world_normal, material_input.world_tangent);
	material_input.TBN     = Math::tangent_basis(material_input.world_normal, material_input.world_tangent, world_bitangent);

	float4 ndc = mul(scene_view.projview, float4(material_input.world_position, 1.0f));

	VertexMaterial material = main<VertexMaterial>(material_input);
	return mul(scene_view.projview, float4(material_input.world_position + material.position_offset, 1.0f));
}

[shader("fragment")]
float4 fs_main() : SV_Target { return 0; }