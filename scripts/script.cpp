Engine::Transform transform;

double factorial(double value)
{
    return value < 1.0 ? 1.0 : value * factorial(value - 1.0);
}



void script_function()
{
    Engine::Object@ object = Engine::Object::find_object("TestResources::Camera");
    string name = "NULL VALUE";
    if(@object != null)
    {
        name = object.as_string();
    }

    printf("Hello World %0", name);
}
