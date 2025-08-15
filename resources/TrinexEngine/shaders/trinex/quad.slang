namespace FullScreenQuad
{
	static const float2 full_screen_quad_vertices[] = {
		float2(-1.0, -1.0), 
        float2(-1.0,  1.0), 
        float2( 1.0, -1.0), 

        float2(-1.0,  1.0), 
        float2( 1.0,  1.0), 
        float2( 1.0, -1.0)  
	};

	float4 vertex_by_index(uint index)
	{
		return float4(full_screen_quad_vertices[index], 0.0, 1.0);
	}

	float2 uv_by_index(uint index)
	{
		float2 vertex = full_screen_quad_vertices[index];
		return (vertex + float2(1.0, 1.0)) / float2(2.0, 2.0);
	}
}