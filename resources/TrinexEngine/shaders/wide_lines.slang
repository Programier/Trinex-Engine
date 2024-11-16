#ifndef WIDE_LINES_SLANG
#define WIDE_LINES_SLANG

#include "math.slang"
#define WIDE_LINE_TRIANGLES_COUNT 6

namespace WideLines
{
    interface Vertex
    {
        float4 get_position();
        float get_thickness();
    };

    interface OutputVertex
    {
        __init(in Vertex vertex, in float4 new_position);
    };

    void add_half_circle<StreamElement : OutputVertex>(in TriangleStream<StreamElement> streamer, in Vertex vertex,
                                                       float4 line_point_to_connect, float f_point_w_component, float f_angle,
                                                       float thickness, float aspect_ratio)
    {
        float4 output;
        for (int n_i = 0; n_i < WIDE_LINE_TRIANGLES_COUNT; ++n_i)
        {
            output.x = cos(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * n_i)) * thickness / aspect_ratio;
            output.y = sin(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * n_i)) * thickness;
            output.z = 0.0f;
            output.w = 0.0f;
            output += line_point_to_connect;
            output *= f_point_w_component;
            streamer.Append(StreamElement(vertex, output));

            output = line_point_to_connect * f_point_w_component;
            streamer.Append(StreamElement(vertex, output));

            output.x = cos(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (n_i + 1))) * thickness / aspect_ratio;
            output.y = sin(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (n_i + 1))) * thickness;
            output.z = 0.0f;
            output.w = 0.0f;
            output += line_point_to_connect;
            output *= f_point_w_component;
            streamer.Append(StreamElement(vertex, output));

            streamer.RestartStrip();
        }
    }

    float4 lerp_ndc_point(float4 base, float4 second)
    {
        if (base.w > 0)
        {
            return base;
        }

        float t = (-base.w) / (second.w - base.w);
        return lerp(base, second, t);
    }

    void stream_line<StreamElement : OutputVertex>(in TriangleStream<StreamElement> streamer, in Vertex vertex_0,
                                                   in Vertex vertex_1, in float2 viewport)
    {
        float4 output;
        float4 position_0 = vertex_0.get_position();
        float4 position_1 = vertex_1.get_position();

        float4 point_0 = WideLines::lerp_ndc_point(position_0, position_1);
        float4 point_1 = WideLines::lerp_ndc_point(position_1, position_0);

        float f_point_0_w = point_0.w;
        float f_point_1_w = point_1.w;

        point_0.xyz = point_0.xyz / point_0.w;
        point_0.w   = 1.0f;
        point_1.xyz = point_1.xyz / point_1.w;
        point_1.w   = 1.0f;

        float thickness_0  = vertex_0.get_thickness() / viewport.y;
        float thickness_1  = vertex_1.get_thickness() / viewport.y;
        float aspect_ratio = viewport.x / viewport.y;

        float3 position_difference = normalize(float3(point_0.xy - point_1.xy, 0.0f));
        float3 coordinate_system   = float3(1.0f, 0.0f, 0.0f);

        float f_angle = -acos(dot(position_difference.xy, coordinate_system.xy)) - M_PI * 0.5f;

        WideLines::add_half_circle(streamer, vertex_0, point_0, f_point_0_w, f_angle, thickness_0, aspect_ratio);
        WideLines::add_half_circle(streamer, vertex_1, point_1, f_point_1_w, f_angle + M_PI, thickness_1, aspect_ratio);

        output.x = cos(f_angle) * thickness_0 / aspect_ratio;
        output.y = sin(f_angle) * thickness_0;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_0;
        output *= f_point_0_w;
        streamer.Append(StreamElement(vertex_0, output));

        output.x = cos(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_0 / aspect_ratio;
        output.y = sin(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_0;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_0;
        output *= f_point_0_w;
        streamer.Append(StreamElement(vertex_0, output));

        output.x = cos(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_1 / aspect_ratio;
        output.y = sin(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_1;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_1;
        output *= f_point_1_w;
        streamer.Append(StreamElement(vertex_1, output));

        output.x = cos(f_angle) * thickness_0 / aspect_ratio;
        output.y = sin(f_angle) * thickness_0;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_0;
        output *= f_point_0_w;
        streamer.Append(StreamElement(vertex_0, output));

        output.x = cos(f_angle) * thickness_1 / aspect_ratio;
        output.y = sin(f_angle) * thickness_1;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_1;
        output *= f_point_1_w;
        streamer.Append(StreamElement(vertex_1, output));

        output.x = cos(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_1 / aspect_ratio;
        output.y = sin(f_angle + (M_PI / WIDE_LINE_TRIANGLES_COUNT * (WIDE_LINE_TRIANGLES_COUNT))) * thickness_1;
        output.z = 0.0f;
        output.w = 0.0f;
        output += point_1;
        output *= f_point_1_w;
        streamer.Append(StreamElement(vertex_1, output));
    }
}// namespace WideLines

#undef WIDE_LINE_TRIANGLES_COUNT
#endif
