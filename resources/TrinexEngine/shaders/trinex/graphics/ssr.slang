import "trinex/quad.slang";
import "trinex/scene_view.slang";
import "trinex/trinex.slang";

uniform Texture2D scene_color;
uniform Texture2D scene_normal;
uniform Texture2D scene_depth;
uniform SamplerState sampler;

[shader("vertex")] 
float4 vertex_main(in uint index : SV_VertexID) : SV_Position
{
	return FullScreenQuad::vertex_by_index(index);
}

[ForceInline] bool is_out_of_screen(float2 ray) { return ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0; }


float3 trace_ray(float3 ray_pos, float3 direction, uint iterations)
{
	float3 hit_color = float3(0.f);
	bool hit         = false;

	for (int i = 0; i < iterations; i++)
	{
		ray_pos += direction;

		if (is_out_of_screen(ray_pos.xy))
		{
			break;
		}

		float depthDif = ray_pos.z - scene_depth.Sample(sampler, ray_pos.xy).r;

		if (depthDif >= 0 && depthDif < 0.00001)
		{
			hit       = true;
			hit_color = scene_color.Sample(sampler, ray_pos.xy).rgb;
			break;
		}
	}

	return hit_color;
}

[shader("fragment")] 
float4 fragment_main(in float4 position : SV_Position) : SV_Target
{
	float max_ray_distance = 100.0f;

	// View Space ray calculation
	float2 uv                        = scene_view.screen_uv(position.xy);
	float3 primitive_normal          = scene_normal.Sample(sampler, uv).xyz;
	float1 primitive_depth           = scene_depth.Sample(sampler, uv).r;
	float3 primitive_position        = scene_view.reconstruct_position(position.xy, primitive_depth);
	float3 ray_start                 = float3(uv, primitive_depth);

	float3 reflected_direction = normalize(reflect(primitive_position - scene_view.camera.location, primitive_normal));

	// if (reflected_direction.z > 0.0f) 
	// 	return float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 ray_end_position = primitive_position.xyz + reflected_direction * max_ray_distance;

	// Texture Space ray calculation
	float4 ray_end = scene_view.camera.projview * float4(ray_end_position, 1.0f);
	ray_end /= ray_end.w;
	ray_end.xy = (ray_end.xy + float2(1.0f, 1.0f)) * 0.5f;

	float3 ray_direction = ray_end.xyz - ray_start;

	int2 screen_space_start_position = int2(uv * scene_view.viewport.size);
	int2 screen_space_end_position   = int2(ray_end.xy * scene_view.viewport.size);
	int2 screen_space_distance       = screen_space_end_position - screen_space_start_position;

	int screen_space_max_distance = max(abs(screen_space_distance.x), abs(screen_space_distance.y)) / 2;
	ray_direction /= max((float) screen_space_max_distance, 0.001f);

	// Trace the ray
	float3 out_color = trace_ray(ray_start, ray_direction, screen_space_max_distance);
	return float4(out_color, 1.0f);	
}
