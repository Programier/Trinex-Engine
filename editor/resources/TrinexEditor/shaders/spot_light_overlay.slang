#include "trinex/attributes.slang"
#include "trinex/common.slang"

[is_globals()]
ConstantBuffer<GlobalParameters> globals;

uniform float4x4 model;
uniform float4 color;
uniform float radius;
uniform float height;

[shader("vertex")]
float4 vs_main(in float3 base_position : POSITION0) : SV_Position
{	
	float3 position = float3(base_position.x * radius, base_position.y * height, base_position.z * radius);
	return mul(globals.projview, mul(model, float4(position, 1.0)));
}

[shader("fragment")]
float4 fs_main() : SV_TARGET0
{
	return color;
}
