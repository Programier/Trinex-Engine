void print_hello()
{
	print("Hello!");
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

		print(string(Engine::EventSystem::instance()));
	}

	void on_create(Engine::Object@ owner)
	{
		@native = @owner;
	}
}
