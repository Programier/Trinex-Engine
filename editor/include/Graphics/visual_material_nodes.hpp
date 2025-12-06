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
		trinex_class(MaterialRoot, Node);

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
		trinex_class(Texture2D, Node);

	public:
		String name;
		Engine::Texture2D* texture;

		Texture2D();

		static Texture2D* static_find_node(Engine::Texture2D* texture, Compiler& compiler, uint16_t id);
		Expression compile(OutputPin* pin, Compiler& compiler) override;
		Texture2D& render() override;
		Texture2D& post_compile(VisualMaterial* material) override;
		static void static_post_compile(VisualMaterial* material, Engine::Texture2D* texture, uint16_t id,
		                                StringView name_override = "");

		inline OutputPin* texture_pin() const { return outputs()[0]; }
	};

	class Sampler : public Node
	{
		trinex_class(Sampler, Node);

	public:
		String name;
		Engine::Sampler sampler;

		Sampler();
		static Sampler* static_find_node(const Engine::Sampler& sampler, Compiler& compiler, uint16_t id);
		Expression compile(OutputPin* pin, Compiler& compiler) override;
		Sampler& post_compile(VisualMaterial* material) override;

		static void static_post_compile(VisualMaterial* material, const Engine::Sampler& sampler, uint16_t id,
		                                StringView name_override = "");

		inline OutputPin* sampler_pin() const { return outputs()[0]; }
	};

	class SampleTexture : public Node
	{
		trinex_class(SampleTexture, Node);

		Expression compile_texture(Compiler& compiler);
		Expression compile_uv(Compiler& compiler);
		Expression compile_sampler(Compiler& compiler);
		Engine::Texture2D* find_texture();

	public:
		Engine::Texture2D* texture;
		Engine::Sampler sampler;

		SampleTexture();
		Expression compile(OutputPin* pin, Compiler& compiler) override;
		SampleTexture& render() override;
		SampleTexture& post_compile(VisualMaterial* material) override;

		inline InputPin* texture_pin() const { return inputs()[0]; }
		inline InputPin* uv_pin() const { return inputs()[1]; }
		inline InputPin* sampler_pin() const { return inputs()[2]; }

		inline OutputPin* rgba_pin() const { return outputs()[0]; }
		inline OutputPin* r_pin() const { return outputs()[1]; }
		inline OutputPin* g_pin() const { return outputs()[2]; }
		inline OutputPin* b_pin() const { return outputs()[3]; }
		inline OutputPin* a_pin() const { return outputs()[4]; }
	};
}// namespace Engine::VisualMaterialGraph
