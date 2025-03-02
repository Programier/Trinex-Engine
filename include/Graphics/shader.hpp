#pragma once
#include <Core/enums.hpp>
#include <Core/render_resource.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
	class ENGINE_EXPORT Shader : public RenderResource
	{
		declare_class(Shader, RenderResource);

	public:
		Buffer source_code;

		bool serialize(Archive& ar) override;
		bool serialize_source_code(Archive& ar);
		virtual ShaderType type() const = 0;
	};


	class ENGINE_EXPORT VertexShader : public Shader
	{
		declare_class(VertexShader, Shader);

	public:
		struct Attribute {
			declare_struct(Attribute, void);

			Name name;
			VertexBufferElementType type;
			VertexAttributeInputRate rate;
			VertexBufferSemantic semantic;
			byte semantic_index;
			byte location;
			byte stream_index;
			uint16_t offset;

			FORCE_INLINE Attribute(VertexAttributeInputRate rate = VertexAttributeInputRate::Vertex,
			                       VertexBufferSemantic semantic = VertexBufferSemantic::Position, byte semantic_index = 0,
			                       byte location = 0, byte stream = 0, uint16_t offset = 0, const Name& name = Name::none)
			    : name(name), rate(rate), semantic(semantic), semantic_index(semantic_index), location(location), offset(offset)
			{}

			bool serialize(Archive& ar);
		};

		Vector<Attribute> attributes;

	public:
		VertexShader& rhi_init() override;
		bool serialize(Archive& ar) override;
		ShaderType type() const override;
	};

	class ENGINE_EXPORT FragmentShader : public Shader
	{
		declare_class(FragmentShader, Shader);

	public:
		FragmentShader& rhi_init() override;
		ShaderType type() const override;
	};

	class ENGINE_EXPORT TessellationControlShader : public Shader
	{
		declare_class(TessellationControlShader, Shader);

	public:
		TessellationControlShader& rhi_init() override;
		ShaderType type() const override;
	};

	class ENGINE_EXPORT TessellationShader : public Shader
	{
		declare_class(TessellationShader, Shader);

	public:
		TessellationShader& rhi_init() override;
		ShaderType type() const override;
	};

	class ENGINE_EXPORT GeometryShader : public Shader
	{
		declare_class(GeometryShader, Shader);

	public:
		GeometryShader& rhi_init() override;
		ShaderType type() const override;
	};

	class ENGINE_EXPORT ComputeShader : public Shader
	{
		declare_class(ComputeShader, Shader);

	public:
		ComputeShader& rhi_init() override;
		ShaderType type() const override;
	};
}// namespace Engine
