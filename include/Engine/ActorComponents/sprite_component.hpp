#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>


namespace Engine
{
	class ENGINE_EXPORT SpriteComponent : public PrimitiveComponent
	{
		declare_class(SpriteComponent, PrimitiveComponent);

		class Texture2D* m_texture = nullptr;

	public:
		Texture2D* texture() const;
		SpriteComponent& texture(Texture2D* texture);
		SpriteComponent& update_bounding_box() override;

		SpriteComponent& render(class SceneRenderer*) override;
		SpriteComponent& on_property_changed(const Refl::PropertyChangedEvent& event);
	};
}// namespace Engine
