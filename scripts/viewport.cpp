

string scripted_window_name = "Scripted Window";

void update()
{
    Engine::Package@ root = Engine::Object::root_package();
    printf(string(root));
}
