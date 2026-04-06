import "trinex/trinex.slang";

struct RHIDrawIndirectCommand 
{
	uint vertex_count;
	uint instance_count;
	uint first_vertex;
	uint first_instance;
}

struct RHIDrawIndexedIndirectCommand 
{
	uint index_count;
	uint instance_count;
	uint first_index;
	int vertex_offset;
	uint first_instance;
}

struct RHIDispatchIndirectCommand 
{
	uint x;
	uint y;
	uint z;
}