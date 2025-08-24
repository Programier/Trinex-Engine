import "trinex/scene_view.slang";
import "trinex/trinex.slang";

static float3 grid_plane[6] = {
    float3(1, 1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
    float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
};

uniform float fov;

struct VertexOutput
{
    float4 position : SV_Position;
    float3 near : POSITION0;
    float3 far : POSITION1;
};

struct Grid
{
    float scale;
    float alpha;

    float4 draw(in float3 frag_pos, in float fade) 
    {
        float thickness = 3.0 * lerp(0.6, 1.0, alpha);

        float2 coord = frag_pos.xz / scale; 
        float2 derivative = fwidth(coord);
        float2 grid = abs(fract(coord - 0.5) - 0.5) / (derivative * thickness);
        
        float minimumz = derivative.y * thickness * scale;
        float minimumx = derivative.x * thickness * scale;
    
        if (abs(frag_pos.x) < minimumx)
        {
            return float4(0.0, 0.0, 1.0, (1.0 - min(grid.x, 1.0)) * alpha); 
        }

        if (abs(frag_pos.z) < minimumz)
        {
            return float4(1.0, 0.0, 0.0, (1.0 - min(grid.y, 1.0)) * fade); 
        }

        return float4(1.0, 1.0, 1.0, (1.0 - min(min(grid.x, grid.y), 1.0)) * alpha);
    }
};

struct GridDrawer
{
    Grid entry1;
    Grid entry2;
    float3 frag_pos;
    float fade;

    __init(float3 pos)
    {
        constexpr float scale_step = 8.0;
        
        frag_pos = pos;
        
        entry1.scale  = 1.f;
        entry2.scale  = scale_step;
    
        float lower_height = 0.f;
        float upper_height = fov * scale_step;
        float camera_height = abs(scene_view.camera.location.y);

        while (upper_height < camera_height)
        {
            lower_height = upper_height;
            upper_height *= scale_step;

            entry1.scale = entry2.scale;
            entry2.scale = entry2.scale * scale_step;
        }

        float3 view_direction = normalize(scene_view.camera.location - pos);
        float3 view_direction_xz = normalize(float3(scene_view.camera.location.x, 0.0, scene_view.camera.location.z) - pos);

        fade = 1.0 - pow(abs(dot(view_direction, view_direction_xz)), 18);

        entry2.alpha = ((camera_height - lower_height) / upper_height);
        entry1.alpha = (1.0 - entry2.alpha);

        entry1.alpha *= fade;
        entry2.alpha *= fade;

    }

    float4 draw()
    {
        float4 grid1 = entry1.draw(frag_pos, fade);
        float4 grid2 = entry2.draw(frag_pos, fade);
        
        if(grid1.a > grid2.a)
        {
            return grid1;
        }

        return grid2;
    }
};

float3 unproject_point(float x, float y, float z)
{
    float4 unprojected_point =  scene_view.camera.inv_projview * float4(x, y, z, 1.0);
    return unprojected_point.xyz / unprojected_point.w;
}

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
float4 fragment_main(in VertexOutput input, out float depth : SV_Depth) : SV_Target
{
    float t = -input.near.y / (input.far.y - input.near.y);
    float3 frag_pos = input.near + t * (input.far - input.near);
    
    depth = compute_depth(frag_pos);
    
    GridDrawer drawer = GridDrawer(frag_pos);
    return drawer.draw() * float4(t > 0.0);
}

