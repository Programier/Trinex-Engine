#include "trinex/attributes.slang"
#include "trinex/common.slang"
#include "trinex/platform.slang"

[is_globals()]
ConstantBuffer<GlobalParameters> globals;
uniform Sampler2D texture;
uniform float4x4 model;

[shader("vertex")]
float2 vs_main(in float3 position : POSITION0, out float4 out_position : SV_Position) : TEXCOORD0
{
	out_position = mul(globals.projview, mul(model, float4(position, 1.0)));
	return Platform::validate_uv((float2(position.x, position.y) * 0.5) + float2(0.5, 0.5));
}

[shader("fragment")]
float4 fs_main(in float2 uv : TEXCOORD0) : SV_Target
{
	float4 color = texture.Sample(uv);
	if(color.a < 0.1)
		discard;
	return color;
}
