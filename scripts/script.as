Engine::Vector3D v(2048);


class Test
{
    
}

void script_function()
{
    Engine::Vector3D test = glm::normalize(v) * 2.252f;
    print(formatFloat(glm::length(test), "\\", 0, 6));
    hello();
    
}

void hello()
{
    print("Hello World");
}
