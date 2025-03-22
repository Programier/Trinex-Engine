#ifndef GBUFFER_MATERIAL_TEMPLATE
#define GBUFFER_MATERIAL_TEMPLATE 1

#include "trinex/common.slang"

[shader("fragment")]
GBufferFragmentOutput fs_main(in FragmentInput input)
{
	FragmentMaterialInput material_input = (FragmentMaterialInput)0;
	material_input.init(input);

	Material material = get_material_fs(material_input);
	
	GBufferFragmentOutput output;
	float3 result_normal = normalize(mul(material_input.TBN, material.normal * 2.0 - 1.0));
	output.base_color = float4(material.base_color, 1.0);
	output.normal   =  float4(result_normal, material_input.front_face);  
	output.emissive = float4(material.emissive, 1.0);      
	output.msra = float4(material.metalness, material.specular, material.roughness, material.AO);
	return output;	
}

#endif
