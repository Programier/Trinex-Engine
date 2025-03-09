#include "trinex/attributes.slang"
#include "trinex/common.slang"
#include "trinex/globals.slang"

uniform float4 	color;
uniform float3 	offset;
uniform float 	radius;


[shader("vertex")]
float4 vs_main(in float3 base_position : POSITION0) : SV_Position
{	
	return mul(globals.projview, float4((base_position * radius) + offset, 1.0));
}

[shader("fragment")]
float4 fs_main() : SV_TARGET0
{
	return color;
}
