#pragma once
#include <Core/executable_object.hpp>
#include <Engine/Render/scene_layer.hpp>

namespace Engine
{
	class MaterialInterface;
	class SceneComponent;
	class VertexBuffer;
	class IndexBuffer;

	class ENGINE_EXPORT CommandBufferLayer : public SceneLayer
	{
	private:
		Buffer m_commands;
		size_t m_allocated = 0;

		template<typename Type, typename... Args>
		Type* create_command(Args&&... args)
		{
			size_t start = m_allocated;
			m_allocated += sizeof(Type);

			if (m_allocated > m_commands.size())
			{
				m_commands.resize(m_allocated);
			}

			byte* data = m_commands.data() + start;
			return new (data) Type(std::forward<Args>(args)...);
		}

	public:
		CommandBufferLayer& draw(size_t vertices_count, size_t vertices_offset);
		CommandBufferLayer& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset);
		CommandBufferLayer& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances);
		CommandBufferLayer& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                           size_t instances);
		CommandBufferLayer& bind_material(class MaterialInterface* material, SceneComponent* component = nullptr);
		CommandBufferLayer& bind_vertex_buffer(class VertexBuffer* buffer, byte stream, size_t offset = 0);
		CommandBufferLayer& bind_index_buffer(class IndexBuffer* buffer, size_t offset = 0);

		template<typename VariableType>
		CommandBufferLayer& update_variable(VariableType& out, const VariableType& in)
		{
			class UpdateVar : public ExecutableObject
			{
			private:
				VariableType input;
				VariableType* m_output;

			public:
				UpdateVar(const VariableType& in, VariableType& out) : input(in), m_output(&out)
				{}

				int_t execute() override
				{
					(*m_output) = std::move(input);
					return sizeof(UpdateVar);
				}
			};

			create_command<UpdateVar>(in, out);
			return *this;
		}


		CommandBufferLayer& clear() override;
		CommandBufferLayer& render(SceneRenderer* renderer, RenderViewport* rt) override;
	};
}// namespace Engine
