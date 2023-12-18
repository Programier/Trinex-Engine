class Viewport
{
    void update(float dt)
    {

    }
    
    void on_create(Engine::Object@ object)
    {
        printf("Viewport using with %0", string(object));
    }
};
