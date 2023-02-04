#include <Core/assimp_helpers.hpp>
#include <Core/check.hpp>
#include <Core/filesystem.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/assimp.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/shader_system.hpp>
#include <Graphics/skeleton.hpp>
#include <Graphics/ssbo.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/textured_object.hpp>
#include <assimp/scene.h>
#include <stack>


namespace Engine
{
    static void bind_ssbo()
    {
        struct Data {
            Vector3D color;
        };

        static SSBO<Data> ssbo;

        if (ssbo.data.empty())
        {
            ssbo.data.push_back({{1.0f, 1.f, 1.f}});
            ssbo.gen().set_data();
        }
        else
        {
            static auto frame = Event::frame_number();

            if (Event::frame_number() != frame)
                return;

            frame++;
            static float time = 0.f;
            time += Event::diff_time();
            ssbo.data.back().color =
                    Vector3D(glm::exp(glm::sin(time)), glm::exp(glm::cos(time)), glm::exp(glm::cos(glm::sin(time))));
            ssbo.update_data(0, 1);
        }


        ssbo.bind(1);
    }

    struct TextureMapNode {
        Texture2D* _M_diffuse;
    };

    using TextureMap = std::unordered_map<unsigned int, TextureMapNode>;

    ////////////////// TEXTURED OBJECT //////////////////


    declare_instance_info_cpp(TexturedObject);
    constructor_cpp(TexturedObject)
    {}

    Texture2D* TexturedObject::diffuse_texture()
    {
        return _M_diffuse_texture;
    }


    const Texture2D* TexturedObject::diffuse_texture() const
    {
        return _M_diffuse_texture;
    }

    TexturedObject& TexturedObject::diffuse_texture(Texture2D* texture)
    {
        _M_diffuse_texture = texture;
        return *this;
    }

    ////////////////// STATIC TEXTURED OBJECT //////////////////

    declare_instance_info_cpp(StaticTexturedObject);
    constructor_cpp(StaticTexturedObject)
    {}

    std::size_t StaticTexturedObject::render(const Matrix4f& matrix)
    {
        if (!_M_mesh)
            return 0;
        auto scene = Scene::get_active_scene();
        check_with_message(scene, "No active scene found!");

        auto camera = scene->active_camera();
        check_with_message(camera, "No active camera found!");

        auto global = matrix * model();
        namespace sh = ShaderSystem::Scene;
        sh::shader.use()
                .set(sh::model, global)
                .set(sh::projview, camera->projview())
                .set(sh::transposed_inversed_model, glm::inverse(glm::transpose(glm::mat3(global))))
                .set(sh::camera_pos, camera->position());

        if (_M_diffuse_texture)
            _M_diffuse_texture->bind();

        bind_ssbo();
        _M_mesh->draw(Primitive::TRIANGLE);
        return 1;
    }

    StaticTexturedObjectMesh* StaticTexturedObject::mesh()
    {
        return _M_mesh;
    }

    const StaticTexturedObjectMesh* StaticTexturedObject::mesh() const
    {
        return _M_mesh;
    }

    StaticTexturedObject& StaticTexturedObject::mesh(StaticTexturedObjectMesh* _mesh)
    {
        if (_M_mesh)
        {}

        _M_mesh = _mesh;
        return *this;
    }

    using LoadMeshFunction = Drawable*(const aiScene* scene, const aiMesh* mesh, TextureMap& map,
                                       const String& dirname);

    static Drawable* create_static_object_from_mesh(const aiScene* scene, const aiMesh* mesh, TextureMap& map,
                                                    const String& dirname)
    {
        StaticTexturedObject* object = Object::new_instance<StaticTexturedObject>();
        object->name(mesh->mName.data);
        object->aabb(
                BoxHB(AssimpHelpers::get_vector3(&mesh->mAABB.mMin), AssimpHelpers::get_vector3(&mesh->mAABB.mMax)));

        // Loading texture
        if (map.contains(mesh->mMaterialIndex))
        {
            auto& node = map.at(mesh->mMaterialIndex);
            object->diffuse_texture(node._M_diffuse);
        }
        else
        {
            auto material = scene->mMaterials[mesh->mMaterialIndex];
            String diffuse = dirname + AssimpLibrary::get_material_string(material, AI_MATKEY_TEXTURE_DIFFUSE(0));

            TextureMapNode node;
            node._M_diffuse = &Object::new_instance<Texture2D>()->load(diffuse);

            object->diffuse_texture(node._M_diffuse);

            map[mesh->mMaterialIndex] = node;
        }

        object->mesh(Object::new_instance<StaticTexturedObjectMesh>());
        auto engine_mesh = object->mesh();

        // Copy vertices
        engine_mesh->data.resize(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            engine_mesh->data[i].position = AssimpHelpers::get_vector3(&mesh->mVertices[i]);

            if (mesh->mTextureCoords[0])
                engine_mesh->data[i].texture_coords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

            if (mesh->mNormals)
                engine_mesh->data[i].normal = AssimpHelpers::get_vector3(&mesh->mNormals[i]);
        }

        // Copy indeces

        // After triangulate postprocess, each face contains only 3 indeces
        engine_mesh->indexes.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            auto& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) engine_mesh->indexes.push_back(face.mIndices[j]);
        }

        engine_mesh->attributes = {
                {3, BufferValueType::FLOAT, offsetof(StaticTexturedObjectVertex, position)},
                {2, BufferValueType::FLOAT, offsetof(StaticTexturedObjectVertex, texture_coords)},
                {3, BufferValueType::FLOAT, offsetof(StaticTexturedObjectVertex, normal)},
        };

        engine_mesh->mode = DrawMode::STATIC_DRAW;
        engine_mesh->gen();
        engine_mesh->set_data().update_atributes().update_indexes();

        Resources::meshes.push_back(engine_mesh);

        return object;
    }


    struct MeshNode {
        aiNode* node = nullptr;
        Matrix4f transform;
        String name;
        MeshNode* parent = nullptr;
        SceneTreeNode* scene_node = nullptr;

        std::vector<MeshNode*> childs;

        ~MeshNode()
        {
            for (auto ell : childs) delete ell;
        }
    };


    static MeshNode* load_meshes(aiNode* root)
    {
        MeshNode* current_node = nullptr;
        auto create_node = [&current_node]() {
            if (!current_node)
                current_node = new MeshNode();
        };

        for (unsigned int i = 0; i < root->mNumChildren; i++)
        {
            auto child = load_meshes(root->mChildren[i]);
            if (child)
            {
                create_node();
                current_node->childs.push_back(child);
                child->parent = current_node;
            }
        }

        if (root->mNumMeshes != 0)
        {
            create_node();
            current_node->node = root;
        }

        if (current_node)
        {
            current_node->name = root->mName.data;
            current_node->transform = AssimpHelpers::get_matrix4(&root->mTransformation);
        }
        return current_node;
    }


    template<typename ObjectType>
    static void foreach_mesh_in_scene(const String& filename, Scene* engine_scene, LoadMeshFunction callback)
    {
        check(engine_scene);

        const aiScene* scene = AssimpLibrary::load_scene(filename);
        if (!scene)
        {
            logger->log("TexturedObject Loader: Failed to load '%s'", filename.c_str());
            return;
        }

        logger->log("Textures: %d", scene->mNumTextures);

        // Loading meshes tree
        MeshNode* assimp_root = load_meshes(scene->mRootNode);
        if (!assimp_root)
        {
            AssimpLibrary::close_scene(scene);
            return;
        }

        assimp_root->scene_node = engine_scene->scene_head();

        String dirname = FileSystem::dirname_of(filename);
        TextureMap texture_map;

        std::stack<MeshNode*> _M_stack;
        _M_stack.push(assimp_root);

        while (!_M_stack.empty())
        {
            MeshNode* node = _M_stack.top();
            _M_stack.pop();

            SceneTreeNode* current_scene_node = Object::new_instance<SceneTreeNode>(node->scene_node);
            current_scene_node->model(node->transform);
            current_scene_node->name(node->name);


            if (node->node)
            {
                for (unsigned int i = 0; i < node->node->mNumMeshes; i++)
                {
                    auto mesh = scene->mMeshes[node->node->mMeshes[i]];
                    ObjectType* object = dynamic_cast<ObjectType*>(callback(scene, mesh, texture_map, dirname));
                    engine_scene->push(object, current_scene_node);
                }
            }

            for (auto child : node->childs)
            {
                child->scene_node = current_scene_node;
                _M_stack.push(child);
            }
        }

        delete assimp_root;
        AssimpLibrary::close_scene(scene);
    }

    ENGINE_EXPORT void StaticTexturedObject::load(const String& filename, Scene* engine_scene)
    {
        foreach_mesh_in_scene<StaticTexturedObject>(filename, engine_scene, create_static_object_from_mesh);
    }


    ////////////////// ANIMATED OBJECT //////////////////

    declare_instance_info_cpp(AnimatedTexturedObject);

    void AnimatedTexturedObjectMesh::clear_bones_info()
    {
        for (auto& vertex : data)
        {
            vertex.vertex_weights.weights = Constants::zero_vector;
            vertex.vertex_weights.bones_id = -Constants::int_identity_vector;
        }
    }

    constructor_cpp(AnimatedTexturedObject)
    {}

    std::size_t AnimatedTexturedObject::render(const Matrix4f& matrix)
    {
        if (!_M_mesh)
            return 0;
        auto scene = Scene::get_active_scene();
        check_with_message(scene, "No active scene found!");

        auto camera = scene->active_camera();
        check_with_message(camera, "No active camera found!");

        auto global = matrix * model();
        namespace sh = ShaderSystem::Anim;

        sh::shader.use()
                .set(sh::model, matrix)
                .set(sh::projview, camera->projview())
                .set(sh::transposed_inversed_model, glm::inverse(glm::transpose(glm::mat3(global))))
                .set("camera", camera->position());

        if (_M_diffuse_texture)
            _M_diffuse_texture->bind();

        _M_ssbo.bind(1);
        _M_mesh->draw(Primitive::TRIANGLE);
        return 1;
    }

    AnimatedTexturedObjectMesh* AnimatedTexturedObject::mesh()
    {
        return _M_mesh;
    }

    const AnimatedTexturedObjectMesh* AnimatedTexturedObject::mesh() const
    {
        return _M_mesh;
    }

    AnimatedTexturedObject& AnimatedTexturedObject::mesh(AnimatedTexturedObjectMesh* mesh)
    {
        _M_mesh = mesh;
        return *this;
    }

    VertexWeights* AnimatedTexturedObject::get_vertex_weights(ArrayIndex index)
    {
        if (_M_mesh && index < _M_mesh->data.size())
            return &_M_mesh->data[index].vertex_weights;
        return nullptr;
    }


    static inline std::unordered_map<const aiNode*, class Skeleton*>& get_skeletons()
    {
        static std::unordered_map<const aiNode*, class Skeleton*> _M_steletons;
        return _M_steletons;
    }

#define skeletons get_skeletons()

    static Bone* find_bone(const aiBone* bone)
    {

        Skeleton* skeleton = nullptr;
        auto it = skeletons.find(bone->mArmature);
        if (it == skeletons.end())
        {
            skeleton = Object::new_instance<Skeleton>();
            skeleton->load_skeleton(bone);
            skeletons[bone->mArmature] = skeleton;
        }
        else
        {
            skeleton = (*it).second;
        }

        return skeleton->root_bone()->find_bone_by_name(bone->mName.data);
    }

    static Drawable* create_animated_object_from_mesh(const aiScene* scene, const aiMesh* mesh, TextureMap& map,
                                                      const String& dirname)
    {
        AnimatedTexturedObject* object = Object::new_instance<AnimatedTexturedObject>();
        object->name(mesh->mName.data);
        object->aabb(
                BoxHB(AssimpHelpers::get_vector3(&mesh->mAABB.mMin), AssimpHelpers::get_vector3(&mesh->mAABB.mMax)));

        // Loading texture
        if (map.contains(mesh->mMaterialIndex))
        {
            auto& node = map.at(mesh->mMaterialIndex);
            object->diffuse_texture(node._M_diffuse);
        }
        else
        {
            auto material = scene->mMaterials[mesh->mMaterialIndex];
            String diffuse = dirname + AssimpLibrary::get_material_string(material, AI_MATKEY_TEXTURE_DIFFUSE(0));

            TextureMapNode node;
            node._M_diffuse = &(Object::new_instance<Texture2D>())->load(diffuse);

            object->diffuse_texture(node._M_diffuse);

            map[mesh->mMaterialIndex] = node;
        }

        object->mesh(Object::new_instance<AnimatedTexturedObjectMesh>());
        auto engine_mesh = object->mesh();

        // Copy vertices
        engine_mesh->data.resize(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            engine_mesh->data[i].position = AssimpHelpers::get_vector3(&mesh->mVertices[i]);

            if (mesh->mTextureCoords[0])
                engine_mesh->data[i].texture_coords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

            if (mesh->mNormals)
                engine_mesh->data[i].normal = AssimpHelpers::get_vector3(&mesh->mNormals[i]);
        }


        // Loading skeletal
        object->load_skeletal(scene, mesh).set_weights_to_vertices();

        // Copy indeces

        // After triangulate postprocess, each face contains only 3 indeces
        engine_mesh->indexes.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            auto& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) engine_mesh->indexes.push_back(face.mIndices[j]);
        }


        engine_mesh->attributes = {
                {3, BufferValueType::FLOAT, offsetof(AnimatedTexturedObjectVertex, position)},
                {2, BufferValueType::FLOAT, offsetof(AnimatedTexturedObjectVertex, texture_coords)},
                {3, BufferValueType::FLOAT, offsetof(AnimatedTexturedObjectVertex, normal)},
                {4, BufferValueType::INT, offsetof(AnimatedTexturedObjectVertex, vertex_weights.bones_id)},
                {4, BufferValueType::FLOAT, offsetof(AnimatedTexturedObjectVertex, vertex_weights.weights)},
        };


        engine_mesh->mode = DrawMode::STATIC_DRAW;
        engine_mesh->gen();
        engine_mesh->set_data().update_atributes().update_indexes();

        Resources::meshes.push_back(engine_mesh);

        return object;
    }// namespace Engine

    ENGINE_EXPORT void AnimatedTexturedObject::load(const String& filename, Scene* engine_scene)
    {
        foreach_mesh_in_scene<AnimatedTexturedObject>(filename, engine_scene, create_animated_object_from_mesh);
        skeletons.clear();
    }
}// namespace Engine
