#include <Core/reflection/class.hpp>
#include <Core/viewport_client.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Trinex
{
	static ScriptFunction vc_update;
	static ScriptFunction vc_attach;
	static ScriptFunction vc_deattach;
	static ScriptFunction vc_render;

	trinex_implement_engine_class(ViewportClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		vc_update   = r.method("void update(RenderViewport viewport, float dt)", trinex_scoped_method(This, update));
		vc_attach   = r.method("void attach(RenderViewport)", trinex_scoped_method(This, attach));
		vc_deattach = r.method("void deattach(RenderViewport)", trinex_scoped_method(This, deattach));

		// Need to check, can we use script engine in multi-thread mode?
		//vc_render = r.method("void render(RenderViewport viewport)", trinex_scoped_method(This, render));

		ScriptEngine::on_terminate.push([]() {
			vc_update.release();
			vc_attach.release();
			vc_deattach.release();
			vc_render.release();
		});
	}

	ViewportClient& ViewportClient::attach(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::deattach(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::update(class RenderViewport* viewport, float dt)
	{
		return *this;
	}

	ViewportClient* ViewportClient::create(const StringView& name)
	{
		auto* client_class = Refl::Class::static_find(name);

		if (client_class)
		{
			return Object::instance_cast<ViewportClient>(client_class->create_object());
		}

		return nullptr;
	}
}// namespace Trinex
