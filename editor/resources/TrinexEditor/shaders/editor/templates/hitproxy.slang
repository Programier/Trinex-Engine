import "trinex/attributes.slang";
import "trinex/material.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";
import "trinex/vertex_input.slang";

extern Material main<Material : IMaterial>(in IMaterialInput input);

extern struct VertexInput : IVertexInput
{
	float3 m_position : POSITION;
	float3 m_normal : NORMAL;
	float3 m_tangent : TANGENT;
	float2 m_uv0 : TEXCOORD0;

	property float3 position { [ForceInline] get { return m_position; } }
	property float3 normal { [ForceInline] get { return m_normal; } }
	property float4 tangent { [ForceInline] get { return float4(m_tangent, 1.0); } }
	property float2 uv0 { [ForceInline] get { return m_uv0; } }
	property float2 uv1 { [ForceInline] get { return 0; } }
	property float4 color { [ForceInline] get { return 0; } }
}

struct HitproxyInput
{
	[parameter_flags(meta::LocalToWorld)]
	float4x4 local_to_world;

	[parameter_flags(meta::ExcludeMaterialParameter)]
	uint2 id;
}

uniform ConstantBuffer<HitproxyInput> hitproxy;

[shader("vertex")]
float4 vs_main(in VertexInput vs_input) : SV_Position
{
	struct MaterialInput : IMaterialInput
	{
		float3 m_world_position;
		float3x3 m_tbn;

		property IVertexInput vertex { [ForceInline] get { return vs_input; } }
		property float3 world_position { [ForceInline] get { return m_world_position; } }
		property float4x4 local_to_world { [ForceInline] get { return hitproxy.local_to_world; } }
		property float3x3 TBN { [ForceInline] get { return m_tbn; } }
	}

	MaterialInput material_input;

	material_input.m_world_position = mul(hitproxy.local_to_world, float4(vs_input.position, 1.0f)).xyz;
	float3x3 rotation               = transpose(Math::inverse((float3x3) hitproxy.local_to_world));

	float3 world_normal  = normalize(mul(rotation, vs_input.normal));
	float3 world_tangent = normalize(mul(rotation, vs_input.tangent.xyz));

	float3 world_bitangent = cross(world_normal, world_tangent);
	material_input.m_tbn   = Math::tangent_basis(world_normal, world_tangent, world_bitangent);

	float4 ndc = mul(scene_view.projview, float4(material_input.world_position, 1.0f));

	VertexMaterial material = main<VertexMaterial>(material_input);
	return mul(scene_view.projview, float4(material_input.world_position + material.position_offset, 1.0f));
}

[shader("fragment")]
uint2 fs_main() : SV_Target { return hitproxy.id; }