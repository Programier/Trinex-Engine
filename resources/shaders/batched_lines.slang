#include "common.slang"

#define TRIANGLES_COUNT 6
static const float PI = 3.1415926f;

struct VertexInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float thickness : POSITION1;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
    noperspective float thickness : POSITION1;
};

struct GeometryOutput
{
	float4 position : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOutput vs_main(in VertexInput input)
{
    VertexOutput output;
    output.color = input.color;
    output.position = mul(globals.projview, float4(input.position, 1.0));
    output.thickness = input.thickness / globals.viewport.w;
    return output;
}

void add_half_circle(inout TriangleStream<GeometryOutput> stream, float4 color, float thickness, float4 line_point_to_connect, float f_point_w_component, float f_angle)
{
    GeometryOutput output = (GeometryOutput)0;
    output.color = color;
    for (int n_i = 0; n_i < TRIANGLES_COUNT; ++n_i)
    {
        output.position.x = cos(f_angle + (PI / TRIANGLES_COUNT * n_i)) * thickness / globals.aspect_ratio;
        output.position.y = sin(f_angle + (PI / TRIANGLES_COUNT * n_i)) * thickness;
        output.position.z = 0.0f;
        output.position.w = 0.0f;
        output.position += line_point_to_connect;
        output.position *= f_point_w_component;
        stream.Append(output);

        output.position = line_point_to_connect * f_point_w_component;
        stream.Append(output);

        output.position.x = cos(f_angle + (PI / TRIANGLES_COUNT * (n_i + 1))) * thickness / globals.aspect_ratio;
        output.position.y = sin(f_angle + (PI / TRIANGLES_COUNT * (n_i + 1))) * thickness;
        output.position.z = 0.0f;
        output.position.w = 0.0f;
        output.position += line_point_to_connect;
        output.position *= f_point_w_component;
        stream.Append(output);

        stream.RestartStrip();
    }
}

float4 lerp_ndc_point(float4 base, float4 second)
{
	if(base.w > 0)
	{
		return base;
	}

    float t = (-base.w) / (second.w - base.w);
    return lerp(base, second, t);
}

[shader("geometry")]
[maxvertexcount(42)]
void gs_main(line VertexOutput input[2], inout TriangleStream<GeometryOutput> stream)
{
    GeometryOutput output = (GeometryOutput)0;

    float4 point_0 = lerp_ndc_point(input[0].position, input[1].position);
    float4 point_1 = lerp_ndc_point(input[1].position, input[0].position);
    float4 color_0 = input[0].color;
    float4 color_1 = input[1].color;

    float f_point_0_w = point_0.w;
    float f_point_1_w = point_1.w;

    point_0.xyz = point_0.xyz / point_0.w;
    point_0.w = 1.0f;
    point_1.xyz = point_1.xyz / point_1.w;
    point_1.w = 1.0f;

    float thickness_0 = input[0].thickness;
    float thickness_1 = input[1].thickness;

    float3 position_difference = normalize(float3(point_0.xy - point_1.xy, 0.0f));
    float3 coordinate_system = float3(1.0f, 0.0f, 0.0f);

    float f_angle = acos(dot(position_difference.xy, coordinate_system.xy));

    if (cross(position_difference, coordinate_system).z < 0.0f)
    {
        f_angle = 2.0f * PI - f_angle;
    }

    f_angle *= -1.0f;
    f_angle -= PI * 0.5f;

    //printf("%f\n", outColor.x)

    add_half_circle(stream, color_0, thickness_0, point_0, f_point_0_w, f_angle);
    add_half_circle(stream, color_1, thickness_1, point_1, f_point_1_w, f_angle + PI);

    output.position.x = cos(f_angle) * thickness_0 / globals.aspect_ratio;
    output.position.y = sin(f_angle) * thickness_0;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_0;
    output.position *= f_point_0_w;
    output.color = color_0;
    stream.Append(output);

    output.position.x = cos(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_0 / globals.aspect_ratio;
    output.position.y = sin(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_0;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_0;
    output.position *= f_point_0_w;
    output.color = color_0;
    stream.Append(output);

    output.position.x = cos(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_1 / globals.aspect_ratio;
    output.position.y = sin(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_1;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_1;
    output.position *= f_point_1_w;
    output.color = color_1;
    stream.Append(output);

    output.position.x = cos(f_angle) * thickness_0 / globals.aspect_ratio;
    output.position.y = sin(f_angle) * thickness_0;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_0;
    output.position *= f_point_0_w;
    output.color = color_0;
    stream.Append(output);

    output.position.x = cos(f_angle) * thickness_1 / globals.aspect_ratio;
    output.position.y = sin(f_angle) * thickness_1;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_1;
    output.position *= f_point_1_w;
    output.color = color_1;
    stream.Append(output);

    output.position.x = cos(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_1 / globals.aspect_ratio;
    output.position.y = sin(f_angle + (PI / TRIANGLES_COUNT * (TRIANGLES_COUNT))) * thickness_1;
    output.position.z = 0.0f;
    output.position.w = 0.0f;
    output.position += point_1;
    output.position *= f_point_1_w;
    output.color = color_1;
    stream.Append(output);
}

[shader("fragment")]
float4 fs_main(in GeometryOutput output) : SV_Target
{
    return output.color;
}