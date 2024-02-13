void event_callback(const Engine::Event& event)
{
	Engine::WindowEvent data = event.window_event();
	printf("New Pos: {%0, %1}", data.x, data.y);
}


void render_class_hierarchy(Engine::Class@ class_instance)
{
	if(@class_instance != null)
	{
		if(ImGui::CollapsingHeader(class_instance.name()))
		{
			ImGui::Indent(10);
			render_class_hierarchy(class_instance.parent());
			ImGui::Text("Size: %0".format(class_instance.sizeof_class()));
			ImGui::Unindent(10);
		}
	}
}

class Viewport
{
	Engine::Object@ native;
	uint64 listener_id = 0;
	Boolean is_window_open(true);

	void update(float dt)
	{
	}

	void on_create(Engine::Object@ owner)
	{
		@native = @owner;
		listener_id = Engine::EventSystem::instance().add_listener(Engine::EventType::WindowMoved, event_callback);
	}
}
