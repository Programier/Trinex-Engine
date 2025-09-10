#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/gpu_buffers.hpp>


namespace Engine
{
	class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
	{
		trinex_declare_class(StaticMeshComponent, MeshComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
			class StaticMesh* m_mesh;

		public:
			size_t lods_count() const override;
			size_t materials_count() const override;
			size_t surfaces_count(size_t lod = 0) const override;
			const MeshSurface* surface(size_t index, size_t lod = 0) const override;
			VertexBufferBase* find_vertex_buffer(RHIVertexSemantic semantic, Index index = 0, size_t lod = 0) override;
			IndexBuffer* find_index_buffer(size_t lod = 0) override;
			MaterialInterface* material(size_t index) const override;

			friend StaticMeshComponent;
		};

	private:
		class StaticMesh* m_mesh = nullptr;
		StaticMeshComponent& submit_new_mesh();

	public:
		Proxy* create_proxy() override;
		StaticMeshComponent& update_bounding_box() override;

		size_t materials_count() const override;

		using MeshComponent::material;
		MaterialInterface* material(size_t index) const override;

		inline StaticMesh* mesh() const { return m_mesh; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		StaticMeshComponent& mesh(StaticMesh* mesh)
		{
			m_mesh = mesh;
			return submit_new_mesh();
		}
	};
}// namespace Engine
