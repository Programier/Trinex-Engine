

string scripted_window_name = "Scripted Window";

void update()
{
    ImGui::Begin("Scripted Window");
    ImGui::InputText("Input name of window", scripted_window_name, ImGuiInputTextFlags::CallbackResize);
    ImGui::End();
}
