#include <Graphics/sampler.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine::VisualMaterialGraph
{
	class MaterialRoot : public Node
	{
		trinex_declare_class(MaterialRoot, Node);

	public:
		InputPin* const base_color;
		InputPin* const opacity;
		InputPin* const emissive;
		InputPin* const specular;
		InputPin* const metalness;
		InputPin* const roughness;
		InputPin* const ao;
		InputPin* const normal;
		InputPin* const position_offset;

		MaterialRoot();
	};

	class Sampler : public Node
	{
		trinex_declare_class(Sampler, Node);

	public:
		Engine::Sampler sampler;
	};
}// namespace Engine::VisualMaterialGraph
