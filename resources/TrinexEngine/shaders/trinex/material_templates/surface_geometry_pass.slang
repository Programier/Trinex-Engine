import "trinex/attributes.slang";
import "trinex/material.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";

extern struct VertexInput : IVertexInput { }
extern Material main<Material : IMaterial>(in IMaterialInput input);

[parameter_type("LocalToWorld")]
uniform float4x4 local_to_world;

struct GeometryFragmentOutput
{
	float4 base_color : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 emissive : SV_TARGET2;
	float4 msra : SV_TARGET3;
};

struct VertexOutput
{
	float4 sv_position : SV_Position;
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	
	float3 world_position : WORLD_POSITION;
	float3 world_normal : WORLD_NORMAL;
	float3 world_tangent : WORLD_TANGETN;

	float3x3 TBN : TBN;
	VertexInput vertex_input;
}

[shader("vertex")]
VertexOutput vs_main(
	in float3 vs_position: POSITION0,
	in float3 vs_normal: NORMAL0,
	in float3 vs_tangent: TANGENT0,
    in VertexInput vs_input)
{
	VertexOutput output;
	
	output.position     = vs_position;
	output.normal       = vs_normal;
	output.tangent      = vs_tangent;
	output.vertex_input = vs_input;

	float4 world_position = mul(local_to_world, float4(vs_position, 1.0f));
	float3x3 rotation     = transpose(Math::inverse((float3x3) local_to_world));

	output.world_position = world_position.xyz / world_position.w;
	output.world_normal   = normalize(mul(rotation, vs_normal));
	output.world_tangent  = normalize(mul(rotation, vs_tangent));

	float3 world_bitangent = cross(output.world_normal, output.world_tangent);
	output.TBN             = Math::tangent_basis(output.world_normal, output.world_tangent, world_bitangent);

	struct MaterialInput : IMaterialInput
	{
		property float3 position { [ForceInline] get { return vs_position; } }
		property float3 normal { [ForceInline] get { return vs_normal; } }
		property float3 tangent { [ForceInline] get { return vs_tangent; } }
		property float3 world_position { [ForceInline] get { return output.world_position; } }
		property float3 world_normal { [ForceInline] get { return output.world_normal; } }
		property float3 world_tangent { [ForceInline] get { return output.world_tangent; } }

		property float3x3 TBN { [ForceInline] get { return output.TBN; }}
		property float4x4 local_to_world { [ForceInline] get { return ::local_to_world; } }
		property IVertexInput vertex_input { [ForceInline] get { return vs_input; } }

		float4 sv_position     = 0;
		float4 screen_position = 0;
		float sv_depth         = 0;
	}

	VertexMaterial material      = main<VertexMaterial>(MaterialInput());
	output.sv_position = mul(scene_view.projview, float4(output.world_position + material.position_offset, 1.0f));
	return output;
}

[shader("fragment")]
GeometryFragmentOutput fs_main(in VertexOutput input)
{
	struct MaterialInput : IMaterialInput
	{
		property float3 position { [ForceInline] get { return input.position; } }
		property float3 normal { [ForceInline] get { return input.normal; } }
		property float3 tangent { [ForceInline] get { return input.tangent; } }
		property float3 world_position { [ForceInline] get { return input.world_position; } }
		property float3 world_normal { [ForceInline] get { return input.world_normal; } }
		property float3 world_tangent { [ForceInline] get { return input.world_tangent; } }

		property float3x3 TBN { [ForceInline] get { return input.TBN; }}
		property float4x4 local_to_world { [ForceInline] get { return ::local_to_world; } }
		property IVertexInput vertex_input { [ForceInline] get { return input.vertex_input; } }

		float4 sv_position     = 0;
		float4 screen_position = 0;
		float sv_depth         = 0;
	}

	FragmentMaterial material = main<FragmentMaterial>(MaterialInput());

	GeometryFragmentOutput output;
	float3 result_normal = normalize(mul(input.TBN, material.normal * 2.0 - 1.0));
	output.base_color    = float4(material.base_color, 1.0);
	output.normal        = float4(result_normal, 1.f);
	output.emissive      = float4(material.emissive, 1.0);
	output.msra          = float4(material.metalness, material.specular, material.roughness, material.ao);
	return output;
}
