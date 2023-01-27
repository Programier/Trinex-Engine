#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/skybox.hpp>

static Engine::Mesh<float> mesh;


static void init_mesh()
{
    static bool mesh_is_inited = false;
    if (mesh_is_inited)
        return;

    Engine::logger->log("Skybox: Init skybox mesh\n");
    mesh.gen();
    mesh.attributes = {{3, Engine::BufferValueType::FLOAT}};
    mesh.data = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
                 -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,
                 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,
                 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
                 -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                 -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
                 -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    mesh.set_data().update_atributes();
    mesh_is_inited = true;
}


namespace Engine
{
    declare_instance_info_cpp(Skybox);
    constructor_cpp(Skybox)
    {}

    Skybox& Skybox::load(const std::vector<String>& filenames, const bool& invert)
    {
        return *this;
    }

    Skybox& Skybox::load(const String& filename, const bool& invert)
    {
        init_mesh();
        Image img(filename, invert);

        if (!TextureCubeMap::has_object())
        {
            TextureParams params;
            params.border = false;
            params.format = img.channels() == 4 ? PixelFormat::RGBA : PixelFormat::RGB;
            params.type = TextureType::Texture_Cube_Map;
            params.pixel_type = BufferValueType::UNSIGNED_BYTE;
            TextureCubeMap::create(params);
        }

        if (img.empty())
        {
            logger->log("Skybox: Failed to load skybox\n");
            return *this;
        }
        int block_width = img.width() / 4;
        int block_height = img.height() / 3;

        Image _M_images[6];

        _M_images[(byte) TextureCubeMapFace::UP] =
                invert ? img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3})
                       : img.sub_image({block_width, 0}, {block_width * 2, block_height});


        _M_images[(byte) TextureCubeMapFace::DOWN] =
                invert ? img.sub_image({block_width, 0}, {block_width * 2, block_height})
                       : img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3});

        _M_images[(byte) TextureCubeMapFace::LEFT] = img.sub_image({0, block_height}, {block_width, block_height * 2});
        _M_images[(byte) TextureCubeMapFace::RIGHT] =
                img.sub_image({block_width * 2, block_height}, {block_width * 3, block_height * 2});
        _M_images[(byte) TextureCubeMapFace::BACK] =
                img.sub_image({block_width * 1, block_height}, {block_width * 2, block_height * 2});
        _M_images[(byte) TextureCubeMapFace::FRONT] =
                img.sub_image({block_width * 3, block_height}, {block_width * 4, block_height * 2});


        for (byte i = 0; i < 6; i++)
        {
            TextureCubeMap::attach_data((TextureCubeMapFace) i, _M_images[i].size(), (void*) _M_images[i].data(), 0);
        }

        return *this;
    }

    Skybox& Skybox::draw()
    {
        EngineInstance::get_instance()->depth_func(CompareFunc::Lequal);
        TextureCubeMap::bind(0);
        mesh.draw(Primitive::TRIANGLE);
        EngineInstance::get_instance()->depth_func(CompareFunc::Less);
        return *this;
    }


    Skybox::Skybox(const String& filename, const bool& invert)
    {
        load(filename, invert);
    }

    Skybox::Skybox(const std::vector<String>& filenames, const bool& invert)
    {
        load(filenames, invert);
    }
}// namespace Engine
