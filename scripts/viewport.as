void event_callback(const Engine::Event& event)
{
	Engine::WindowEvent data = event.window_event();
	printf("New Pos: {%0, %1}", data.x, data.y);
}


class Viewport
{
	Engine::Object@ native;

	void update(float dt)
	{
		ImGui::Begin("Scripted Window");
		ImGui::BeginTabBar("Hello2");
		ImGui::TabItemButton("Hello");
		ImGui::EndTabBar();
		ImGui::End();
	}

	void on_create(Engine::Object@ owner)
	{
		@native = @owner;
		Engine::EventSystem::instance().add_listener(Engine::EventType::WindowMoved, event_callback);
	}
}
