#include <Core/assimp_helpers.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/assimp.hpp>
#include <ImGui/ImGuiFileDialog.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_init.hpp>
#include <Window/window.hpp>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


#define on_clicked_event if (update_right_panel && ImGui::IsItemClicked(ImGuiMouseButton_Left))

using namespace Engine;

enum class PointerType
{
    aiScene,
    aiNode,
    aiMesh,
    aiBone,
    aiAnimation,
    aiNodeAnim,
    aiAnimMesh,
};


struct AssimpPointer {
    AssimpObject pointer;
    PointerType type;
};

using RenderAssimpObjectFunc = void (*)(AssimpObject);

static Map<PointerType, RenderAssimpObjectFunc> render_functions;


// Application data

Window window;
const aiScene* scene = nullptr;
AssimpPointer current_pointer = {nullptr, PointerType::aiScene};
ImGuiStyle* style = nullptr;
static std::size_t node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
static bool update_right_panel = true;


static void default_render(AssimpObject object)
{
    ImGui::Text("Undefined object: %p", object);
}

static RenderAssimpObjectFunc get_render_function(PointerType type)
{
    auto iter = render_functions.find(type);
    if (iter == render_functions.end())
        return default_render;
    return (*iter).second;
}

static void render_assimp_pointer(const AssimpPointer& pointer)
{
    get_render_function(pointer.type)(pointer.pointer);
}

static void color_settings()
{
    static const char* colorNames[] = {
            "ImText",
            "ImTextDisabled",
            "ImWindowBg",
            "ImChildBg",
            "ImPopupBg",
            "ImBorder",
            "ImBorderShadow",
            "ImFrameBg",
            "ImFrameBgHovered",
            "ImFrameBgActive",
            "ImTitleBg",
            "ImTitleBgActive",
            "ImTitleBgCollapsed",
            "ImMenuBarBg",
            "ImScrollbarBg",
            "ImScrollbarGrab",
            "ImScrollbarGrabHovered",
            "ImScrollbarGrabActive",
            "ImCheckMark",
            "ImSliderGrab",
            "ImSliderGrabActive",
            "ImButton",
            "ImButtonHovered",
            "ImButtonActive",
            "ImHeader",
            "ImHeaderHovered",
            "ImHeaderActive",
            "ImSeparator",
            "ImSeparatorHovered",
            "ImSeparatorActive",
            "ImResizeGrip",
            "ImResizeGripHovered",
            "ImResizeGripActive",
            "ImTab",
            "ImTabHovered",
            "ImTabActive",
            "ImTabUnfocused",
            "ImTabUnfocusedActive",
            "ImPlotLines",
            "ImPlotLinesHovered",
            "ImPlotHistogram",
            "ImPlotHistogramHovered",
            "ImTableHeaderBg",
            "ImTableBorderStrong",
            "ImTableBorderLight",
            "ImTableRowBg",
            "ImTableRowBgAlt",
            "ImTextSelectedBg",
            "ImDragDropTarget",
            "ImNavHighlight",
            "ImNavWindowingHighlight",
            "ImNavWindowingDimBg",
            "ImModalWindowDimBg",
    };

    ImGui::Text("ColorChanger");
    ImGui::Separator();
    std::size_t index = 0;

    for (auto name : colorNames)
    {
        ImGui::ColorEdit4(name, reinterpret_cast<float*>(&style->Colors[index++]));
        ImGui::Separator();
    }
}

#define THEME_ITEM(item_code)                                                                                          \
    item_code;                                                                                                         \
    ImGui::Separator();

static void theme_settings()
{
    //float Alpha;         // Global alpha applies to everything in Dear ImGui.
    THEME_ITEM(ImGui::Text("Theme Settings"));
    THEME_ITEM(ImGui::SliderFloat("Alpha Channel", &style->Alpha, 0.01f, 1));
    THEME_ITEM(ImGui::SliderFloat("Disabled Alpha Channel", &style->DisabledAlpha, 0.01f, 1, "%.3f"));
    THEME_ITEM(ImGui::InputFloat2("Window Padding", reinterpret_cast<float*>(&style->WindowPadding)));
    THEME_ITEM(ImGui::SliderFloat("Window Rounding", &style->WindowRounding, 0, 20));
    THEME_ITEM(ImGui::SliderFloat("Window Border size", &style->WindowBorderSize, 0, 4));
    THEME_ITEM(ImGui::SliderFloat("Child Rounding", &style->ChildRounding, 0, 20));
    THEME_ITEM(ImGui::SliderFloat("Child Border size", &style->ChildBorderSize, 0, 4));
    THEME_ITEM(ImGui::SliderFloat("Popup Rounding", &style->PopupRounding, 0, 20));
    THEME_ITEM(ImGui::SliderFloat("Popup Border size", &style->PopupBorderSize, 0, 4));
    THEME_ITEM(ImGui::SliderFloat("Frame Rounding", &style->FrameRounding, 0, 20));
    THEME_ITEM(ImGui::SliderFloat("Frame Border size", &style->FrameBorderSize, 0, 4));
    THEME_ITEM(ImGui::SliderFloat2("Frame Padding", reinterpret_cast<float*>(&style->FramePadding), 0, 10));

    THEME_ITEM({
        auto min_size = style->WindowMinSize;
        if (ImGui::InputFloat2("Window Min Size", reinterpret_cast<float*>(&min_size), "%.3f"))
        {
            if (min_size.x > 0)
                style->WindowMinSize.x = min_size.x;
            if (min_size.y > 0)
                style->WindowMinSize.y = min_size.y;
        }
    });

    THEME_ITEM(
            ImGui::DragFloat2("Window title align", reinterpret_cast<float*>(&style->WindowTitleAlign), 0.001, 0, 1));
    THEME_ITEM(ImGui::DragFloat2("Item Spacing", reinterpret_cast<float*>(&style->ItemSpacing), 0.001, 0, 100));
    THEME_ITEM(
            ImGui::DragFloat2("Item Inner Spacing", reinterpret_cast<float*>(&style->ItemInnerSpacing), 0.001, 0, 100));
    THEME_ITEM(ImGui::SliderFloat("Indent Spacing", &style->IndentSpacing, 0, 100));
    THEME_ITEM(ImGui::SliderFloat("Column Min Spacing", &style->ColumnsMinSpacing, 0, 100));
    THEME_ITEM(ImGui::DragFloat2("Cell Padding", reinterpret_cast<float*>(&style->CellPadding), 0.001, 0, 100));
    THEME_ITEM(ImGui::SliderFloat("ScrollBar Size", &style->ScrollbarSize, 0, 20));
    THEME_ITEM(ImGui::SliderFloat("ScrollBar Rounding", &style->ScrollbarRounding, 0, 20));


    //        ImGuiDir
    //                WindowMenuButtonPosition;// Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left.
    //        ImVec2 TouchExtraPadding;// Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    //        float IndentSpacing;// Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    //        float ColumnsMinSpacing;// Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    //        float GrabMinSize;      // Minimum width/height of a grab box for slider/scrollbar.
    //        float GrabRounding;     // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    //        float LogSliderDeadzone;// The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    //        float TabRounding;      // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    //        float TabBorderSize;    // Thickness of border around tabs.
    //        float TabMinWidthForCloseButton;// Minimum width for close button to appears on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
    //        ImGuiDir
    //                ColorButtonPosition;// Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
    //        ImVec2 ButtonTextAlign;// Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
    //        ImVec2 SelectableTextAlign;// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    //        ImVec2 DisplayWindowPadding;// Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    //        ImVec2 DisplaySafeAreaPadding;// If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!
    //        float MouseCursorScale;// Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
    //        bool AntiAliasedLines;// Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
    //        bool AntiAliasedLinesUseTex;// Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to ImDrawList).
    //        bool AntiAliasedFill;// Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
    //        float CurveTessellationTol;// Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    //        float CircleTessellationMaxError;// Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.
}


static void render_menubar()
{
    ImGui::BeginMainMenuBar();
    auto instance = ImGuiFileDialog::Instance();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Load..."))
        {

            instance->OpenDialog("LoadScene", "Open File", "*.*", "./");
        }

        ImGui::EndMenu();
    }

    static bool render_settings = false;

    if (ImGui::MenuItem("Settings"))
        render_settings = true;

    ImGui::Checkbox("Update right panel", &update_right_panel);


    if (render_settings)
    {
        static bool is_first_frame = true;

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({1280, 720});


        ImGui::Begin("Setting", &render_settings, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Columns(2, nullptr);

        ImGui::BeginChild("Settings.LeftChild");
        static std::size_t choiced_index = 0;
        static Vector<std::pair<const char*, void (*)()>> menu = {
                {"Colors", color_settings},
                {"Theme", theme_settings},
        };


        for (std::size_t i = 0; i < menu.size(); i++)
        {
            if (ImGui::MenuItem(menu[i].first))
            {
                choiced_index = i;
                break;
            }
        }

        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("Settings.RightChild");
        menu[choiced_index % menu.size()].second();
        ImGui::EndChild();

        ImGui::End();
        if ((is_first_frame = !render_settings))
        {
            // Write config
            std::ofstream("theme.pcnf", std::ios_base::binary).write((char*) style, sizeof(*style));
        }
    }

    if (instance->IsOpened("LoadScene"))
    {
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({Engine::Window::width(), Engine::Window::height()});
        if (instance->Display("LoadScene"))
        {
            if (instance->IsOk())
            {
                if (scene)
                {
                    AssimpLibrary::close_scene(scene);
                    current_pointer = {nullptr, PointerType::aiScene};
                }
                scene = AssimpLibrary::load_scene(Strings::to_string(instance->GetFilePathName()));
            }
            instance->Close();
        }
    }

    ImGui::EndMainMenuBar();
}


static void render()
{
    Engine::ImGuiInit::new_frame();
    ImGui::NewFrame();

    {
        static bool is_first_frame = true;
        if (is_first_frame)
        {
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({1280, 720});
            is_first_frame = false;
        }
    }

    ImGui::Begin("MainFrame", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    render_menubar();

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 1280 / 2);
    ImGui::SetColumnWidth(1, 1280 / 2);
    ImGui::BeginChild("LeftPanel", {0, 0}, true, ImGuiWindowFlags_HorizontalScrollbar);
    render_assimp_pointer({scene, PointerType::aiScene});
    ImGui::EndChild();
    ImGui::NextColumn();

    ImGui::BeginChild("RightPanel", {0, 0}, true, ImGuiWindowFlags_HorizontalScrollbar);
    render_assimp_pointer(current_pointer);
    ImGui::EndChild();
    ImGui::NextColumn();


    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    Engine::ImGuiInit::render();
}

static void application_loop()
{
    while (window.is_open())
    {
        window.clear_buffer();

        // Render
        render();

        window.swap_buffers();
        window.event.wait_for_event();
    }
}

static void render_matrix(const aiMatrix4x4& __matrix__, const char* title)
{
    if (ImGui::TreeNode(title))
    {
        Matrix4f matrix = AssimpHelpers::get_matrix4(&__matrix__);
        static const char* names[] = {"##1", "##2", "##3", "##4"};
        for (int i = 0; i < 4; i++)
            ImGui::DragFloat4(names[i], glm::value_ptr(matrix[i]), 0.0f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);

        ImGui::TreePop();
    }
}

static void aiAnimMesh_render(const aiAnimMesh* mesh)
{
    bool status = ImGui::TreeNodeEx(Strings::format("AnimMesh {}", mesh->mName.data).c_str(), node_flags);
    on_clicked_event current_pointer = {mesh, PointerType::aiAnimMesh};

    if (status)
    {

        ImGui::TreePop();
    }
}

static void aiNodeAnim_render(const aiNodeAnim* node)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Animation {}", node->mNodeName.data).c_str(), node_flags);
    on_clicked_event current_pointer = {node, PointerType::aiNodeAnim};

    if (status)
    {
        ImGui::Text("Position key nums: %d", node->mNumPositionKeys);
        ImGui::Text("Rotation key nums: %d", node->mNumRotationKeys);
        ImGui::Text("Scaling key nums: %d", node->mNumScalingKeys);

        if (ImGui::TreeNodeEx(Strings::format("Animation<{}> Position keys").c_str(), node_flags))
        {
            for (unsigned int i = 0; i < node->mNumPositionKeys; i++)
            {
                ImGui::Text("Time: %lf", node->mRotationKeys[i].mTime);
                auto key = AssimpHelpers::get_vector3(&node->mPositionKeys[i].mValue);
                ImGui::DragFloat3(Strings::format("##Anim<{}> Position Key {}", node->mNodeName.data, i + 1).c_str(),
                                  glm::value_ptr(key), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
            }
            ImGui::TreePop();
        }


        if (ImGui::TreeNodeEx(Strings::format("Animation<{}> Scaling keys").c_str(), node_flags))
        {
            for (unsigned int i = 0; i < node->mNumScalingKeys; i++)
            {
                ImGui::Text("Time: %lf", node->mScalingKeys[i].mTime);
                auto key = AssimpHelpers::get_vector3(&node->mScalingKeys[i].mValue);
                ImGui::DragFloat3(Strings::format("##Anim<{}> Scaling Key {}", node->mNodeName.data, i + 1).c_str(),
                                  glm::value_ptr(key), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
            }
            ImGui::TreePop();
        }


        if (ImGui::TreeNodeEx(Strings::format("Animation<{}> Rotation keys").c_str(), node_flags))
        {
            for (unsigned int i = 0; i < node->mNumRotationKeys; i++)
            {
                auto key = node->mRotationKeys[i];
                ImGui::Separator();
                ImGui::Text("Time: %lf", key.mTime);
                auto quat = AssimpHelpers::get_quaternion(&key.mValue);
                ImGui::DragFloat4(Strings::format("##Anim<{}> Scaling Key {}", node->mNodeName.data, i + 1).c_str(),
                                  glm::value_ptr(quat), 0.f, 0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
                ImGui::Separator();
            }
            ImGui::TreePop();
        }


        ImGui::TreePop();
    }
}

static void aiAnimation_render(const aiAnimation* anim)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Animation {}", anim->mName.C_Str()).c_str(), node_flags);
    on_clicked_event current_pointer = {anim, PointerType::aiAnimation};

    if (status)
    {
        ImGui::Text("Ticks per seconts: %lf", anim->mTicksPerSecond);
        ImGui::Text("Duration: %f", anim->mDuration);
        ImGui::Text("Channels: %d", anim->mNumChannels);
        if (ImGui::TreeNodeEx(Strings::format("Anim<{}> channels", anim->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < anim->mNumChannels; i++)
            {
                aiNodeAnim_render(anim->mChannels[i]);
            }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

static void aiScene_render(const aiScene* scene)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Scene {}", scene->mName.C_Str()).c_str(), node_flags);


    on_clicked_event current_pointer = {scene, PointerType::aiScene};


    if (status)
    {
        render_assimp_pointer({scene->mRootNode, PointerType::aiNode});
        ImGui::Text("Animations: %d", scene->mNumAnimations);
        if (ImGui::TreeNodeEx(Strings::format("Scene<{}> animations").c_str(), node_flags))
        {
            for (unsigned int i = 0; i < scene->mNumAnimations; i++)
            {
                auto anim = scene->mAnimations[i];
                aiAnimation_render(anim);
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

static void aiBone_render(const aiBone* bone)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Bone {}", bone->mName.data).c_str(), node_flags);
    on_clicked_event current_pointer = {bone, PointerType::aiBone};

    if (status)
    {
        ImGui::Separator();
        ImGui::Text("Armature");
        render_assimp_pointer({bone->mArmature, PointerType::aiNode});
        ImGui::Separator();

        ImGui::Separator();
        ImGui::Text("Node");
        render_assimp_pointer({bone->mNode, PointerType::aiNode});
        ImGui::Separator();

        render_matrix(bone->mOffsetMatrix, Strings::format("Bone<{}> offset", bone->mName.data).c_str());

        ImGui::Text("Weights count: %d", bone->mNumWeights);
        if (ImGui::TreeNodeEx(Strings::format("Bone<{}> weights", bone->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < bone->mNumWeights; i++)
            {
                auto weight = bone->mWeights[i];
                ImGui::Text("Weight: %f", weight.mWeight);
                ImGui::Text("Vertex ID: %d", weight.mVertexId);
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

static void aiNode_render(const aiNode* node)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Node {}", node->mName.data).c_str(), node_flags);
    on_clicked_event current_pointer = {node, PointerType::aiNode};

    if (status)
    {
        ImGui::Text("Meshes count: %d", node->mNumMeshes);

        if (ImGui::TreeNode(Strings::format("Meshes<{}>", node->mName.C_Str()).c_str()))
        {
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                if (ImGui::TreeNode(Strings::format("Mesh {}", node->mMeshes[i]).c_str()))
                {
                    render_assimp_pointer({scene->mMeshes[node->mMeshes[i]], PointerType::aiMesh});
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        render_matrix(node->mTransformation, "Transformation");
        ImGui::NewLine();
        for (unsigned int i = 0; i < node->mNumChildren; i++) aiNode_render(node->mChildren[i]);
        ImGui::TreePop();
    }
}

static void aiMesh_render(const aiMesh* mesh)
{
    bool status = ImGui::TreeNodeEx(Strings::format("Mesh {}", mesh->mName.data).c_str(), node_flags);
    on_clicked_event current_pointer = {mesh, PointerType::aiMesh};


    if (status)
    {
        ImGui::Text("Vertices count: %d", mesh->mNumVertices);
        if (ImGui::TreeNodeEx(Strings::format("Mesh<{}> vertices", mesh->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                Vector3D point = AssimpHelpers::get_vector3(&mesh->mVertices[i]);
                ImGui::DragFloat3(Strings::format("##Vert{}", i).c_str(), glm::value_ptr(point), 0.f, 0.f, 0.f, "%.3f",
                                  ImGuiSliderFlags_NoInput);
            }
            ImGui::TreePop();
        }


        ImGui::Text("Faces count: %d", mesh->mNumFaces);
        if (ImGui::TreeNodeEx(Strings::format("Mesh<{}> faces", mesh->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                if (ImGui::TreeNodeEx(Strings::format("Mesh<{}> face {}", mesh->mName.data, i + 1).c_str(), node_flags))
                {
                    aiFace& face = mesh->mFaces[i];
                    ImGui::Text("Indeces count: %d", face.mNumIndices);
                    for (unsigned int j = 0; j < face.mNumIndices; j++)
                    {
                        Vector3D point = AssimpHelpers::get_vector3(&mesh->mVertices[face.mIndices[j]]);
                        ImGui::DragFloat3(Strings::format("##FaceVert{}{}", i, j).c_str(), glm::value_ptr(point), 0.f,
                                          0.f, 0.f, "%.3f", ImGuiSliderFlags_NoInput);
                    }

                    auto normal = AssimpHelpers::get_vector3(&mesh->mNormals[face.mIndices[0]]);
                    ImGui::DragFloat3(Strings::format("##Normal{}", i).c_str(), glm::value_ptr(normal), 0.f, 0.f, 0.f,
                                      "%.3f", ImGuiSliderFlags_NoInput);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::Text("Bones count: %d", mesh->mNumBones);
        if (ImGui::TreeNodeEx(Strings::format("Mesh<{}> bones", mesh->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < mesh->mNumBones; i++)
            {
                aiBone_render(mesh->mBones[i]);
            }
            ImGui::TreePop();
        }

        ImGui::Text("Anim meshes count: %d", mesh->mNumAnimMeshes);
        if (ImGui::TreeNodeEx(Strings::format("Mesh<{}> anim mesh", mesh->mName.data).c_str(), node_flags))
        {
            for (unsigned int i = 0; i < mesh->mNumAnimMeshes; i++)
            {
                aiAnimMesh_render(mesh->mAnimMeshes[i]);
            }
            ImGui::TreePop();
        }


        /*
        unsigned int mNumAnimMeshes;
        C_STRUCT aiAnimMesh **mAnimMeshes;
        */

        ImGui::TreePop();
    }
}

static void init_application()
{
    init();
    window.init({1280, 720}, STR("AssimpViewer"));

    const char* glsl_ver = "#version 300 es";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    Engine::ImGuiInit::init(glsl_ver);
    AssimpLibrary::init();

    style = &ImGui::GetStyle();

    // Try to read style

    {
        std::ifstream file("theme.pcnf", std::ios_base::binary);
        if (file.is_open())
        {
            file.read((char*) style, sizeof(ImGuiStyle));
        }
    }

    // Init render functions
#define create_render(type)                                                                                            \
    render_functions.insert({PointerType::type, [](AssimpObject __object__) {                                          \
                                 const type* object = static_cast<const type*>(__object__);                            \
                                 if (object == nullptr)                                                                \
                                 {                                                                                     \
                                     ImGui::Text("Object: NULL");                                                      \
                                     return;                                                                           \
                                 }                                                                                     \
                                 type##_render(object);                                                                \
                             }})

    create_render(aiScene);
    create_render(aiNode);
    create_render(aiMesh);
    create_render(aiBone);
    create_render(aiAnimation);
    create_render(aiNodeAnim);
    create_render(aiAnimMesh);
}


static void ui_terminate()
{
    Engine::ImGuiInit::terminate_imgui();
    ImGui::DestroyContext();
    if (scene)
        AssimpLibrary::close_scene(scene);
    terminate();
}

int main()
{
    init_application();
    application_loop();
    ui_terminate();
    return 0;
}
