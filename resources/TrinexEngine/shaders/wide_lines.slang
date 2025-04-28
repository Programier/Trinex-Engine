#ifndef WIDE_LINES_SLANG
#define WIDE_LINES_SLANG

#include "trinex/math.slang"
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

	void stream_line<StreamElement : OutputVertex>(in TriangleStream<StreamElement> streamer, in Vertex vertex_0,
	                                               in Vertex vertex_1, in float2 viewport)
	{
		float4 position_0 = vertex_0.get_position();
		float4 position_1 = vertex_1.get_position();

		float4 ndc_0 = position_0 / position_0.w;
		float4 ndc_1 = position_1 / position_1.w;

		float2 line_dir = ndc_1.xy - ndc_0.xy;
		line_dir        = normalize(line_dir);

		float2 normal = float2(-line_dir.y, line_dir.x);

		float thickness_0  = (2.0f * vertex_0.get_thickness()) / viewport.y;
		float thickness_1  = (2.0f * vertex_1.get_thickness()) / viewport.y;
		float aspect_ratio = viewport.x / viewport.y;

		normal.x /= aspect_ratio;

		float2 offset0 = normal * thickness_0 * 0.5f;
		float2 offset1 = normal * thickness_1 * 0.5f;

		offset0 *= position_0.w;
		offset1 *= position_1.w;

		float4 v0 = position_0 + float4(+offset0, 0.0f, 0.0f);
		float4 v1 = position_0 + float4(-offset0, 0.0f, 0.0f);
		float4 v2 = position_1 + float4(+offset1, 0.0f, 0.0f);
		float4 v3 = position_1 + float4(-offset1, 0.0f, 0.0f);

		streamer.Append(StreamElement(vertex_0, v0));
		streamer.Append(StreamElement(vertex_0, v1));
		streamer.Append(StreamElement(vertex_1, v2));

		streamer.Append(StreamElement(vertex_0, v1));
		streamer.Append(StreamElement(vertex_1, v2));
		streamer.Append(StreamElement(vertex_1, v3));
	}
}// namespace WideLines

#undef WIDE_LINE_TRIANGLES_COUNT
#endif
