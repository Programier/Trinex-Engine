#include <Window/color.hpp>
#include <stdexcept>
namespace Engine
{
    void Color::validate()
    {
        for (auto& value : values)
        {
            if (value < 0 || value > 255)
                throw std::runtime_error("Engine::Color: Failed to create color");
            if (value > 1)
            {
                value /= 255;
            }
        }
    }


    Color::Color(float red, float green, float blue, float alpha)
    {
        this->red = red;
        this->green = green;
        this->blue = blue;
        this->alpha = alpha;
        validate();
    }

    Color::Color(const Color& color) = default;
    Color& Color::operator=(const Color& color)
    {
        for (int i = 0; i < 4; i++) values[i] = color.values[i];
        return *this;
    }

    float Color::R() const
    {
        return red;
    }

    float Color::G() const
    {
        return green;
    }

    float Color::B() const
    {
        return blue;
    }

    float Color::A() const
    {
        return alpha;
    }

    Color& Color::R(float red)
    {
        this->red = red;
        validate();
        return *this;
    }

    Color& Color::G(float green)
    {
        this->green = green;
        validate();
        return *this;
    }

    Color& Color::B(float blue)
    {
        this->blue = blue;
        validate();
        return *this;
    }

    Color& Color::A(float alpha)
    {
        this->alpha = alpha;
        validate();
        return *this;
    }
}// namespace Engine
