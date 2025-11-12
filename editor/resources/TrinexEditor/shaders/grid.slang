import "trinex/scene_view.slang";
import "trinex/trinex.slang";

static float3 grid_plane[6] = {
    float3(1, 1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
    float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
};

struct Args
{
	float lower_scale;
	float lower_alpha;
	float upper_scale;
	float upper_alpha;
};

[parameter_type(meta::type::UniformBuffer)]
uniform Args args;

struct VertexOutput
{
    float4 position : SV_Position;
    float3 near : POSITION0;
    float3 far : POSITION1;
};

float4 draw_grid(in float3 pos, in float scale, in float alpha, in float fade)
{
	float thickness = 3.0 * lerp(0.6, 1.0, alpha);

	float2 coord      = pos.xz / scale;
	float2 derivative = fwidth(coord);
	float2 grid       = abs(fract(coord - 0.5) - 0.5) / (derivative * thickness);

	float minimumz = derivative.y * thickness * scale;
	float minimumx = derivative.x * thickness * scale;

	if (abs(pos.x) < minimumx)
	{
		return float4(0.0, 0.0, 1.0, (1.0 - min(grid.x, 1.0)) * alpha);
	}

	if (abs(pos.z) < minimumz)
	{
		return float4(1.0, 0.0, 0.0, (1.0 - min(grid.y, 1.0)) * fade);
	}

	return float4(1.0, 1.0, 1.0, (1.0 - min(min(grid.x, grid.y), 1.0)) * alpha);
}

float4 draw_grid(in float3 pos)
{
	float3 view_direction    = normalize(scene_view.camera.location - pos);
	float3 view_direction_xz = normalize(float3(scene_view.camera.location.x, 0.0, scene_view.camera.location.z) - pos);
	float fade               = 1.0 - pow(abs(dot(view_direction, view_direction_xz)), 18);

	float4 lower = draw_grid(pos, args.lower_scale, args.lower_alpha * fade, fade);
	float4 upper = draw_grid(pos, args.upper_scale, args.upper_alpha * fade, fade);

	if (lower.a > upper.a)
	{
		return lower;
	}

	return upper;
}

[ForceInline]
float3 unproject_point(float x, float y, float z)
{
    float4 unprojected_point =  scene_view.camera.inv_projview * float4(x, y, z, 1.0);
    return unprojected_point.xyz / unprojected_point.w;
}

[ForceInline]
float compute_depth(float3 pos) 
{
    float4 clip_space_pos = scene_view.camera.projview * float4(pos, 1.0);
    float result = (clip_space_pos.z / clip_space_pos.w);
    return result;
}

[shader("vertex")]
void vertex_main(out VertexOutput output, uint vertex : SV_VertexID)
{
    float3 p = grid_plane[vertex];
    output.position = float4(p, 1.0);

    output.near = unproject_point(p.x, p.y, 0.0);
    output.far = unproject_point(p.x, p.y, 1.0);
}

[shader("fragment")]
void fragment_main(in VertexOutput input, out float depth : SV_Depth, out float4 color : SV_Target)
{
    float t = -input.near.y / (input.far.y - input.near.y);
    float3 pos = input.near + t * (input.far - input.near);

	depth = compute_depth(pos);
    color = draw_grid(pos) * float4(t > 0.0);
}

