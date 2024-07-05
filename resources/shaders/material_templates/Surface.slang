#ifndef SURFACE_TEMPLATE
#define SURFACE_TEMPLATE

#include "common.slang"
#include "vertex_factory/static_mesh.slang"
#include "material_common.slang"

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif 

struct GeometryData
{
	float3 world_position : TEXCOORD0;
	float2 tex_coord[8] : TEXCOORD1;
	float3 world_normal : TEXCOORD2;
	float4 world_tangent : TEXCOORD3;

#if USE_VERTEX_COLOR
	float4 vertex_color : COLOR;
#endif
};

struct VertexOutput
{
	float4 position : SV_Position;
	GeometryData geometry;
};

struct FragmentInput
{
	float4 position : SV_Position;
	GeometryData geometry;
	bool is_front_face : SV_IsFrontFace;
};

struct MaterialInput
{
	float3 world_position;
	float two_sided_sign;
	float2 tex_coord[8];

#if USE_VERTEX_COLOR
	float4 vertex_color;
#endif
	float3x3 TBN;
	float4 sv_position;
};


MaterialInput get_geometry_material_input(in GeometryData geometry)
{
	MaterialInput output = (MaterialInput)0;
	output.world_position = geometry.world_position;
	
	for(int i = 0; i < 8; i++)
	{
		output.tex_coord[i] = geometry.tex_coord[i];
	}

#if USE_VERTEX_COLOR
	output.vertex_color = geometry.vertex_color;
#endif
	output.TBN = calc_tangent_basis(geometry.world_normal, geometry.world_tangent);
	return output;
}

MaterialInput get_material_input(in FragmentInput input)
{
	MaterialInput output = get_geometry_material_input(input.geometry);
	output.two_sided_sign = (input.is_front_face ? 1.0 : -1.0);
	output.sv_position = input.world_position;
	return output;
}

Material get_material_vs(in MaterialInput input)
{
	@1
}

Material get_material_fs(in MaterialInput input)
{
	@2
}

[shader("vertex")]
VertexOutput vs_main(in VertexFactory vertex_factory, out VertexFactory out_factory) : SV_Position
{
	out_factory = vertex_factory;
	return 
}



#endif