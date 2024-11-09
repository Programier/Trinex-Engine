#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/sprite_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
	implement_engine_class(SpriteComponent, 0)
	{
		auto* self = This::static_class_instance();
		trinex_refl_prop(self, This, m_texture)->display_name("Texture").tooltip("Sprite texture");
	}

	implement_empty_rendering_methods_for(SpriteComponent);


	static FORCE_INLINE Matrix4f rotate_sprite(Transform input_transform, const SceneView& view)
	{
		return input_transform.look_at(view.camera_view().location, Constants::OY).matrix();
	}

	ColorSceneRenderer& ColorSceneRenderer::render_component(SpriteComponent* component)
	{
		render_base_component(component);
		//        Material* material                 = DefaultResources::sprite_material;
		//        PositionVertexBuffer* vertex_bufer = DefaultResources::screen_position_buffer;
		//        if (Mat4MaterialParameter* parameter = reinterpret_cast<Mat4MaterialParameter*>(material->find_parameter(Name::model)))
		//        {
		//            Matrix4f model   = rotate_sprite(component->proxy()->world_transform(), scene_view());
		//            parameter->param = model;
		//        }

		//        BindingMaterialParameter* texture_parameter =
		//                reinterpret_cast<BindingMaterialParameter*>(material->find_parameter(Name::texture));
		//        Texture* tmp     = nullptr;
		//        Texture* current = reinterpret_cast<Texture*>(component->texture());

		//        if (texture_parameter && current)
		//        {
		//            tmp = texture_parameter->texture_param();
		//            texture_parameter->texture_param(current);
		//        }

		//        material->apply(component);
		//        vertex_bufer->rhi_bind(0, 0);
		//        engine_instance->rhi()->draw(6, 0);

		//        if (texture_parameter && current)
		//        {
		//            texture_parameter->texture_param(tmp);
		//        }
		return *this;
	}

	Texture2D* SpriteComponent::texture() const
	{
		return m_texture;
	}

	SpriteComponent& SpriteComponent::texture(Texture2D* texture)
	{
		m_texture = texture;
		on_transform_changed();
		return *this;
	}

	SpriteComponent& SpriteComponent::update_bounding_box()
	{
		if (m_texture)
		{
			static AABB_3Df default_aabb({-.5, -.5, -.5}, {.5f, .5f, .5f});
			m_bounding_box = default_aabb.apply_transform(world_transform().matrix());
		}
		else
		{
			const Vector3D& location = world_transform().location();
			m_bounding_box.minmax(location, location);
		}

		submit_bounds_to_render_thread();
		return *this;
	}

	SpriteComponent& SpriteComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	SpriteComponent& SpriteComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_class_instance())
		{
			on_transform_changed();
		}
		return *this;
	}

}// namespace Engine
