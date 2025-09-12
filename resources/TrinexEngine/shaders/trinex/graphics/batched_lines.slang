import "trinex/trinex.slang";
import "trinex/wide_lines.slang";

uniform float4x4 projview;
uniform float2 viewport;

struct VertexInput 
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float thickness : POSITION1;
};

struct VertexOutput : WideLines::Vertex 
{
    float4 position : SV_Position;
    float4 color : COLOR0;
    noperspective float thickness : POSITION1;

    float4 get_position()
    {
        return position;
    }

    float get_thickness()
    {
        return thickness;
    }
};

struct GeometryOutput : WideLines::OutputVertex 
{
    float4 position : SV_Position;
    float4 color : COLOR0;

    __init(in WideLines::Vertex vertex, in float4 new_position)
    {
        position = new_position;

        if (let v = vertex as VertexOutput)
        {
            color = v.color;
        }
    }
};

[shader("vertex")]
VertexOutput vertex_main(in VertexInput input) 
{
    VertexOutput output;
    output.color     = input.color;
    output.position  = projview * float4(input.position, 1.0);
    output.thickness = input.thickness;
    return output;
}

[shader("geometry")]
[maxvertexcount(6)] 
void geometry_main(line VertexOutput input[2], inout TriangleStream<GeometryOutput> stream)
{
    WideLines::stream_line(stream, input[0], input[1], viewport);
}

[shader("fragment")]
float4 fragment_main(in GeometryOutput output) : SV_Target
{
    return output.color;
}
