#include "skybox.hpp"
#include "mesh.hpp"
#include <GL/glew.h>
#include <iostream>

#define RIGHT 0
#define LEFT 1
#define BOTTOM 2
#define TOP 3
#define FRONT 4
#define BACK 5


static Engine::Mesh mesh;


void init_mesh()
{
    static bool mesh_is_inited = false;
    if (mesh_is_inited)
        return;
    std::clog << "Skybox: Init skybox mesh" << std::endl;
    mesh.attributes({3});
    mesh.data() = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
                   -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                   1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,
                   1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,
                   1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
                   -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                   -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
                   -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
    mesh.vertices_count(mesh.data().size() / 3);
    mesh.update_buffers();
    mesh_is_inited = true;
}


namespace Engine
{
    void Skybox::delete_skybox()
    {
        if (_M_ID != 0)
        {
            glDeleteTextures(1, &_M_ID);
            _M_ID = 0;
        }
    }

    Skybox::Skybox()
    {}

    void Skybox::update_id()
    {
        init_mesh();
        glGenTextures(1, &_M_ID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _M_ID);
        unsigned i = 0;

        for (auto& img : _M_images)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++, 0, GL_RGB, img.width(), img.height(), 0,
                         img.channels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.data());
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    Skybox& Skybox::load(const std::vector<std::string>& filenames, const bool& invert)
    {
        delete_skybox();
        if (filenames.size() != 6)
        {
            std::cerr << "SkyBox: Failed to load faces, faces count must be 6" << std::endl;
            return *this;
        }

        int index = 0;
        for (auto& file : filenames)
        {
            std::clog << "Skybox: Loading " << file << std::endl;
            _M_images[index].load(file, invert);
            if (_M_images[index++].empty())
            {
                std::clog << "Skybox: Failed to load " << file << std::endl;
                return *this;
            }
        };

        update_id();
        return *this;
    }

    Skybox& Skybox::load(const std::string& filename, const bool& invert)
    {
        delete_skybox();
        Image img(filename, invert);

        if (img.empty())
        {
            std::cerr << "Skybox: Failed to load skybox" << std::endl;
            return *this;
        }
        int block_width = img.width() / 4;
        int block_height = img.height() / 3;


        _M_images[TOP] = invert ? img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3})
                                : img.sub_image({block_width, 0}, {block_width * 2, block_height});


        _M_images[BOTTOM] =
                invert ? img.sub_image({block_width, 0}, {block_width * 2, block_height})
                       : img.sub_image({block_width, block_height * 2}, {block_width * 2, block_height * 3});

        _M_images[LEFT] = img.sub_image({0, block_height}, {block_width, block_height * 2});
        _M_images[RIGHT] = img.sub_image({block_width * 2, block_height}, {block_width * 3, block_height * 2});
        _M_images[FRONT] = img.sub_image({block_width * 1, block_height}, {block_width * 2, block_height * 2});
        _M_images[BACK] = img.sub_image({block_width * 3, block_height}, {block_width * 4, block_height * 2});

        update_id();
        return *this;
    }

    Skybox& Skybox::draw()
    {
        glDepthFunc(GL_LEQUAL);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _M_ID);
        mesh.draw(TRIANGLE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDepthFunc(GL_LESS);
        return *this;
    }

    Skybox::~Skybox()
    {
        delete_skybox();
    }

    Skybox::Skybox(const std::string& filename, const bool& invert)
    {
        load(filename, invert);
    }

    Skybox::Skybox(const std::vector<std::string>& filenames, const bool& invert)
    {
        load(filenames, invert);
    }

    Skybox::Skybox(const Skybox& skybox)
    {
        *this = skybox;
    }

    Skybox& Skybox::operator=(const Skybox& skybox)
    {
        if (this == &skybox)
            return *this;
        delete_skybox();
        for (int i = 0; i < 6; i++) _M_images[i] = skybox._M_images[i];
        update_id();
        return *this;
    }
}// namespace Engine
