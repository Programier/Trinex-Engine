Boolean@ boolean = true;

Engine::Vector3D color;

string buffer = "Hello World";

void update()
{
    if (boolean.value)
    {
        ImGui::Begin("Hello World", boolean);
        ImGui::End();
    }
}
