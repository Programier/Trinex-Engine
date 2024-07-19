#ifndef SURFACE_TEMPLATE
#define SURFACE_TEMPLATE

#include "common.slang"
#include "vertex_factory/static_mesh.slang"
#include "material_common.slang"


#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif 

// Global parameters
@0

struct GeometryData
{
#if USE_VERTEX_COLOR
	float4 vertex_color : COLOR;
#endif

	float4 	world_position	: POSITION1;
	float3	world_normal	: NORMAL1;
	float3	world_tangent	: TANGENT1;
	float3	world_binormal	: BINORMAL1;

	float2 	uv[8]			: TEXCOORD0;

	[mutating]
	void init(in VertexFactory vertex_factory)
	{
		world_position 	= vertex_factory.get_world_position();
		world_normal 	= vertex_factory.get_world_normal();
		world_tangent 	= vertex_factory.get_world_tangent();
		world_binormal 	= vertex_factory.get_world_binormal();

		for(uint i = 0, count = vertex_factory.get_uv_count(); i < count; ++i)
		{
			uv[i] = vertex_factory.get_uv(i);
		}

#if USE_VERTEX_COLOR
		vertex_color = vertex_factory.get_color();
#endif
	}
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
	float3x3 TBN;

#if USE_VERTEX_COLOR
	float4 vertex_color;
#endif
	
	float4 	sv_position;
	float4 	world_position;
	float3	world_normal;
	float3	world_tangent;
	float3	world_binormal;

	float2 	uv[8];

	float front_face;

	[mutating]
	void init_base(in GeometryData data)
	{
		TBN = calc_tangent_basis(data.world_normal, data.world_tangent);
#if USE_VERTEX_COLOR
		vertex_color 	= data.vertex_color;
#endif

		world_position 	= data.world_position;
		world_normal 	= data.world_normal;
		world_tangent 	= data.world_tangent;
		world_binormal 	= data.world_binormal;

		for(uint i = 0; i < 8; ++i)
		{
			uv[i] = data.uv[i];
		}
	}

	[mutating]
	void init_vs(in GeometryData data)
	{
		init_base(data);
		sv_position 	= mul(globals.projview, data.world_position);
	}

	[mutating]
	void init_fs(in FragmentInput data)
	{
		init_base(data.geometry);
		front_face = data.is_front_face  ? 1.f : -1.f;
		sv_position = data.position;
	}
};

Material get_material_vs(in MaterialInput input)
{
	@1
}

Material get_material_fs(in MaterialInput input)
{
	@2
}

[shader("vertex")]
VertexOutput vs_main(in VertexFactory vertex_factory)
{
	VertexOutput output = (VertexOutput)0;
	output.geometry.init(vertex_factory);
	
	MaterialInput material_input = (MaterialInput)0;
	material_input.init_vs(output.geometry);
	output.position = material_input.sv_position;

	return output;
}

#include "editor/material_templates/gbuffer.slang"

#endif