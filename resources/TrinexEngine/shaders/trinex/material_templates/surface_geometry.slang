import "trinex/trinex.slang";
import "trinex/material.slang";

struct GeometryFragmentOutput
{
	float4 base_color : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 emissive : SV_TARGET2;
	float4 msra : SV_TARGET3;

	static GeometryFragmentOutput from(in MaterialFragmentStage::Output input)
	{
		GeometryFragmentOutput output;
		output.base_color = float4(input.base_color, 1.0);
		output.normal     = float4(input.normal * 0.5f + 0.5f, 1.f);
		output.emissive   = float4(input.emissive, 1.0);
		output.msra       = float4(input.metalness, input.specular, input.roughness, input.ao);

		return output;
	}
}


[shader("vertex")]
MaterialVertexStage::Output vertex_main(in MaterialVertexStage stage)
{
	return stage.execute();
}

[shader("fragment")]
GeometryFragmentOutput fragment_main(in MaterialFragmentStage stage)
{
	return GeometryFragmentOutput::from(stage.execute());
}
