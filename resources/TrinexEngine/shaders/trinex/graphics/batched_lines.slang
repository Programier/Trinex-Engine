#include "trinex/attributes.slang"
#include "trinex/common.slang"
#include "trinex/globals.slang"
#include "wide_lines.slang"

struct VertexInput 
{
    [vertex_stream(0)]
    [vertex_offset(0)]
    float3 position : POSITION0;

    [vertex_stream(0)]
    [vertex_offset(12)]
    float4 color : COLOR0;

    [vertex_stream(0)]
    [vertex_offset(16)]
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
VertexOutput vs_main(in VertexInput input) 
{
    VertexOutput output;
    output.color     = input.color;
    output.position  = mul(globals.projview, float4(input.position, 1.0));
    output.thickness = input.thickness;
    return output;
}

[shader("geometry")]
[maxvertexcount(42)] 
void gs_main(line VertexOutput input[2], inout TriangleStream<GeometryOutput> stream)
{
    WideLines::stream_line(stream, input[0], input[1], globals.viewport.zw);
}

[shader("fragment")]
float4 fs_main(in GeometryOutput output) : SV_Target
{
    return output.color;
}
