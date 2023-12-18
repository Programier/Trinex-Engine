class Viewport
{
	Engine::Object@ native;

	void update(float dt)
	{
		
	}

	void on_create(Engine::Object@ owner)
	{
		@native = @owner;
	}
}
