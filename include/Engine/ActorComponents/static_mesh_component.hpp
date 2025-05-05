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
		public:
			MaterialInterface* material(size_t index) const override { return nullptr; }
			size_t materials_count() const override { return 0; }
			size_t surfaces() const override { return 0; }
			size_t lods() const override { return 0; }
			const MeshSurface* surface(size_t index) const override { return nullptr; }
			VertexBufferBase* find_vertex_buffer(VertexBufferSemantic semantic, Index index = 0, size_t lod = 0) override
			{
				return nullptr;
			}
			IndexBuffer* find_index_buffer(size_t lod = 0) override { return nullptr; }
		};

	public:
		class StaticMesh* mesh = nullptr;

		Proxy* create_proxy() override;

		StaticMeshComponent& render(class SceneRenderer*) override;
		StaticMeshComponent& update_bounding_box() override;
		MaterialInterface* material(size_t index) const;

		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
	};
}// namespace Engine
