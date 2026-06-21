#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex
{
	trinex_implement_engine_class(PrimitiveComponent, Refl::Class::IsScriptable) {}

	PrimitiveComponent::PrimitiveComponent() {}

	PrimitiveComponent::~PrimitiveComponent() {}
}// namespace Trinex
