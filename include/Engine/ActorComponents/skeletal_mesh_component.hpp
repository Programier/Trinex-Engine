#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>

namespace Engine
{
	class RHIBuffer;

	class ENGINE_EXPORT SkeletalMeshComponent : public MeshComponent
	{
		trinex_declare_class(SkeletalMeshComponent, MeshComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
			class SkeletalMesh* m_mesh;
			RHIBuffer* m_bones = nullptr;

		private:
			Proxy& update_mesh(SkeletalMesh* mesh);

		public:
			size_t lods_count() const override;
			size_t materials_count() const override;
			size_t surfaces_count(size_t lod = 0) const override;
			const MeshSurface* surface(size_t index, size_t lod = 0) const override;
			VertexBufferBase* find_vertex_buffer(RHIVertexSemantic semantic, Index index = 0, size_t lod = 0) override;
			IndexBuffer* find_index_buffer(size_t lod = 0) override;
			MaterialInterface* material(size_t index) const override;
			Proxy& render(Renderer* renderer, RenderPass* pass, const MaterialBindings* bindings) override;

			friend SkeletalMeshComponent;
		};

	private:
		class SkeletalMesh* m_mesh = nullptr;
		SkeletalMeshComponent& submit_new_mesh();

	public:
		Proxy* create_proxy() override;
		SkeletalMeshComponent& update_bounding_box() override;

		size_t materials_count() const override;

		using MeshComponent::material;
		MaterialInterface* material(size_t index) const override;

		inline SkeletalMesh* mesh() const { return m_mesh; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		SkeletalMeshComponent& mesh(SkeletalMesh* mesh)
		{
			m_mesh = mesh;
			return submit_new_mesh();
		}
	};
}// namespace Engine
