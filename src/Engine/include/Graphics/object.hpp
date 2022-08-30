#pragma once
#include <Graphics/basic_object.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{

    class SubObject : public BasicObject<Translate, Rotate, Scale>
    {
    public:
        struct Parameters {
            ReferenceWrapper<SubObject> _M_sub_objects[2][2][2];
            std::string _M_object_name;
            AABB_3D _M_aabb;
        };

    protected:
        Parameters _M_parameters;
    };


    class Object : public SubObject
    {
    public:
        struct ObjectData {
            struct ObjectTexture {
                Texture _M_diffuse_textures;
                Texture _M_ambient_texture;
                Texture _M_specular_texture;
                Texture _M_shinnines_texture;
            };

            struct ObjectMesh {
                Mesh _M_mesh;
                std::string _M_mesh_name;
                ReferenceWrapper<ObjectTexture> _M_texture;
            };

            std::unordered_map<std::string, ReferenceWrapper<ObjectTexture>> _M_textures;
            std::vector<ObjectMesh> _M_meshes;
        };

        class SubObject
        {
            Object& _M_base;
            std::list<ObjectID> _M_mesh_ids;

        public:
            SubObject(Object& base);
        };

    private:
        ObjectData _M_object_data;
        SubObject _M_sub_object;

        void _M_load();

    public:
        Object();
        Object(const std::string& model_file, const DrawMode& mode = Engine::LINEAR, const MipMapLevel& mipmap_level = 4,
               const bool& invert = true);
        Object(const Object&);
        Object& operator=(const Object& obj);

        Object& load(const std::string& model_file, const DrawMode& mode = Engine::LINEAR, const MipMapLevel& mipmap_level = 4,
                     const bool& invert = true);
    };
}// namespace Engine
