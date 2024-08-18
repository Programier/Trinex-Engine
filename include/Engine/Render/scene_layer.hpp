#pragma once
#include <Core/implement.hpp>
#include <Core/name.hpp>

namespace Engine
{
	class SceneRenderer;
	class RenderViewport;
	class PrimitiveComponent;
	class LightComponent;
	class BatchedLines;
	class BatchedTriangles;

	class ENGINE_EXPORT SceneLayer
	{
	public:
		using FunctionCallback = void (*)(SceneRenderer*, RenderViewport*, SceneLayer*);

		List<FunctionCallback> on_begin_render;
		List<FunctionCallback> on_end_render;

	private:
		SceneLayer* create_next_internal(const Name& name, SceneLayer* (*allocator)(const Name& name));
		SceneLayer* create_parent_internal(const Name& name, SceneLayer* (*allocator)(const Name& name));

		template<typename LayerType>
		static SceneLayer* static_layer_allocator(const Name& name)
		{
			SceneLayer* layer = new LayerType();
			layer->m_name	  = name;
			return layer;
		}

	protected:
		SceneLayer* m_parent = nullptr;
		SceneLayer* m_next	 = nullptr;
		Name m_name;


		SceneLayer();
		SceneLayer(const Name& name);
		virtual bool can_create_parent_layer();
		virtual ~SceneLayer();

	public:
		delete_copy_constructors(SceneLayer);

		SceneLayer* parent() const;
		SceneLayer* next() const;
		const Name& name() const;

		void destroy();
		SceneLayer* find(const Name& name);

		template<typename Type = SceneLayer>
		Type* create_next(const Name& name)
		{
			return reinterpret_cast<Type*>(create_next_internal(name, static_layer_allocator<Type>));
		}

		template<typename Type = SceneLayer>
		Type* create_parent(const Name& name)
		{
			return reinterpret_cast<Type*>(create_parent_internal(name, static_layer_allocator<Type>));
		}

		virtual SceneLayer& clear();
		virtual SceneLayer& begin_render(SceneRenderer* renderer, RenderViewport*);
		virtual SceneLayer& render(SceneRenderer*, RenderViewport*);
		virtual SceneLayer& end_render(SceneRenderer* renderer, RenderViewport* render_target);

		friend class SceneRenderer;
	};

	class ENGINE_EXPORT RootLayer : public SceneLayer
	{
	protected:
		bool can_create_parent_layer() override;

	public:
		RootLayer();
	};


	class ENGINE_EXPORT DepthRenderingLayer : public SceneLayer
	{
		Set<LightComponent*> m_light_components;

	public:
		DepthRenderingLayer& clear() override;
		DepthRenderingLayer& add_light(LightComponent* component);
		DepthRenderingLayer& remove_light(LightComponent* component);
		DepthRenderingLayer& render(SceneRenderer*, RenderViewport*) override;
	};
}// namespace Engine
