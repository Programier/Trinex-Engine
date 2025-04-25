#include <Graphics/sampler.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine
{
	class Texture2D;
}

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

	class Texture2D : public Node
	{
		trinex_declare_class(Texture2D, Node);

	public:
		String name;
		Engine::Texture2D* texture;

		Texture2D();

		Expression compile(OutputPin* pin, Compiler& compiler) override;
		Texture2D& render() override;
		Texture2D& post_compile(VisualMaterial* material) override;
	};

	class Sampler : public Node
	{
		trinex_declare_class(Sampler, Node);

	public:
		Engine::Sampler sampler;

		Sampler();
		Expression compile(OutputPin* pin, Compiler& compiler) override;
	};

	class SampleTexture : public Node
	{
		trinex_declare_class(SampleTexture, Node);

	public:
		Engine::Sampler sampler;

		SampleTexture();
		Expression compile(OutputPin* pin, Compiler& compiler) override;

		inline InputPin* texture_pin() const { return inputs()[0]; }
		inline InputPin* uv_pin() const { return inputs()[1]; }
		inline InputPin* sampler_pin() const { return inputs()[2]; }
	};
}// namespace Engine::VisualMaterialGraph
