#ifndef GBUFFER_MATERIAL_TEMPLATE
#define GBUFFER_MATERIAL_TEMPLATE 1

#include "common.slang"


/*
struct Material
{
	float3 base_color;
	float3 emissive;
	float3 world_normal;
	float3 position_offset;

	float specular;
	float metalness;
	float roughness;
	float opacity;
    float AO;
};
*/
[shader("fragment")]
GBufferFragmentOutput fs_main(in FragmentInput input)
{
	MaterialInput material_input = (MaterialInput)0;
	material_input.init_fs(input);

	Material material = get_material_fs(material_input);
	
	GBufferFragmentOutput output;
	output.base_color = float4(material.base_color, 1.0);
    output.position = input.geometry.world_position;   
    output.normal   =  float4(input.geometry.world_normal, material_input.front_face);  
    output.emissive = float4(material.emissive, 1.0);      
    output.msra = float4(material.metalness, material.specular, material.roughness, material.AO);
	return output;	
}

#endif
