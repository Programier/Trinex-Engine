float script_function()
{
    Engine::Vector3D vector(1.0);
    float l = glm::length(vector);
    print(l);

    return l;
}
