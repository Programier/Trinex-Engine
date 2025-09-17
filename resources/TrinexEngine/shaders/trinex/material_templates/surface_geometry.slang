import "trinex/trinex.slang";
import "trinex/material.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";
import "trinex/vertex_input.slang";

extern Material main<Material : IMaterial>(in IMaterialInput input);

extern struct VertexInput : IVertexInput;

struct GeometryUniforms 
{
	[parameter_type(meta::type::LocalToWorld)]
	float4x4 local_to_world;
}

uniform ConstantBuffer<GeometryUniforms> uniforms;

struct GeometryFragmentOutput
{
	float4 base_color : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 emissive : SV_TARGET2;
	float4 msra : SV_TARGET3;
};

struct VertexOutput
{
	GeometryVertexInput vertex_input;
	float4 sv_position : SV_Position;
	float3 world_position : WORLD_POSITION;
	float3x3 TBN : TBN;
}

[shader("vertex")]
VertexOutput vertex_main(in VertexInput vs_input)
{
	VertexOutput output;
	vs_input.vertex_input_main(output.vertex_input);

	output.world_position = (uniforms.local_to_world * float4(vs_input.position, 1.0f)).xyz;

	float3x3 rotation     = transpose(Math::inverse((float3x3) uniforms.local_to_world));
	float3 world_normal   = normalize(rotation * vs_input.normal);
	float3 world_tangent  = normalize(rotation * vs_input.tangent.xyz);

	float3 world_bitangent = cross(world_normal, world_tangent);
	output.TBN             = Math::tangent_basis(world_normal, world_tangent, world_bitangent);

	struct MaterialInput : IMaterialInput
	{
		property IVertexInput vertex { [ForceInline] get { return output.vertex_input; } }
		property float3 world_position { [ForceInline] get { return output.world_position; } }
		property float4x4 local_to_world { [ForceInline] get { return uniforms.local_to_world; } }
		property float3x3 TBN { [ForceInline] get { return output.TBN; } }
	}

	VertexMaterial material = main<VertexMaterial>(MaterialInput());
	output.world_position += material.position_offset;
	output.sv_position = scene_view.camera.projview * float4(output.world_position + material.position_offset, 1.0f);
	return output;
}

[shader("fragment")]
GeometryFragmentOutput fragment_main(in VertexOutput input)
{
	struct MaterialInput : IMaterialInput
	{
		property IVertexInput vertex { [ForceInline] get { return input.vertex_input; } }
		property float3 world_position { [ForceInline] get { return input.world_position; } }
		property float4x4 local_to_world { [ForceInline] get { return uniforms.local_to_world; } }
		property float3x3 TBN { [ForceInline] get { return input.TBN; } }
	}

	FragmentMaterial material = main<FragmentMaterial>(MaterialInput());

	GeometryFragmentOutput output;

	float3 normal = normalize(input.TBN * (material.normal * 2.0 - 1.0));
	float3 view   = scene_view.camera.location - input.world_position;
	normal        = normal * sign(dot(normal, view));

	output.base_color    = float4(material.base_color, 1.0);
	output.normal        = float4(normal, 1.f);
	output.emissive      = float4(material.emissive, 1.0);
	output.msra          = float4(material.metalness, material.specular, material.roughness, material.ao);
	return output;
}
