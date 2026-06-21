import "trinex/trinex.slang";
import "trinex/material.slang";

[shader("vertex")]
MaterialVertexStage::Output vertex_main(in MaterialVertexStage stage)
{
	return stage.execute();
}