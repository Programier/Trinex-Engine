#include <Core/class.hpp>
#include <Core/color.hpp>


namespace Engine
{

    Color::Color()        = default;
    Color::Color(Color&&) = default;

    Color& Color::operator=(const Color& color)
    {
        return *this = dynamic_cast<const glm::vec4&>(color);
    }

    Color& Color::operator=(const glm::vec4& color)
    {
        if (dynamic_cast<const glm::vec4*>(this) == &color)
            return *this;

        for (int i = 0; i < 4; i++)
        {
            float& value = glm::vec4::operator[](i);
            value        = glm::abs(color[i]);
            if (value > 1.0f)
                value = glm::min(1.f, value / 255.f);
        }
        return *this;
    }

    Color& Color::operator=(Color&&) = default;
    Color::Color(float r, float g, float b, float a) : Color(glm::vec4(r, g, b, a))
    {}

    Color::Color(const glm::vec4& color)
    {
        *this = color;
    }

    Color::Color(const Color& color)
    {
        *this = dynamic_cast<const glm::vec4&>(color);
    }


    const ENGINE_EXPORT Color Color::IndianRed(205.f, 92.f, 92.f, 255.f);
    const ENGINE_EXPORT Color Color::LightCoral(240.f, 128.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::Salmon(250.f, 128.f, 114.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkSalmon(233.f, 150.f, 122.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSalmon(255.f, 160.f, 122.f, 255.f);
    const ENGINE_EXPORT Color Color::Crimson(220.f, 20.f, 60.f, 255.f);
    const ENGINE_EXPORT Color Color::Red(255.f, 0.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::FireBrick(178.f, 34.f, 34.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkRed(139.f, 0.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::Pink(255.f, 192.f, 203.f, 255.f);
    const ENGINE_EXPORT Color Color::LightPink(255.f, 182.f, 193.f, 255.f);
    const ENGINE_EXPORT Color Color::HotPink(255.f, 105.f, 180.f, 255.f);
    const ENGINE_EXPORT Color Color::DeepPink(255.f, 20.f, 147.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumVioletRed(199.f, 21.f, 133.f, 255.f);
    const ENGINE_EXPORT Color Color::PaleVioletRed(219.f, 112.f, 147.f, 255.f);
    const ENGINE_EXPORT Color Color::Coral(255.f, 127.f, 80.f, 255.f);
    const ENGINE_EXPORT Color Color::Tomato(255.f, 99.f, 71.f, 255.f);
    const ENGINE_EXPORT Color Color::OrangeRed(255.f, 69.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkOrange(255.f, 140.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::Orange(255.f, 165.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::Gold(255.f, 215.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::Yellow(255.f, 255.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::LightYellow(255.f, 255.f, 224.f, 255.f);
    const ENGINE_EXPORT Color Color::LemonChiffon(255.f, 250.f, 205.f, 255.f);
    const ENGINE_EXPORT Color Color::LightGoldenrodYellow(250.f, 250.f, 210.f, 255.f);
    const ENGINE_EXPORT Color Color::PapayaWhip(255.f, 239.f, 213.f, 255.f);
    const ENGINE_EXPORT Color Color::Moccasin(255.f, 228.f, 181.f, 255.f);
    const ENGINE_EXPORT Color Color::PeachPuff(255.f, 218.f, 185.f, 255.f);
    const ENGINE_EXPORT Color Color::PaleGoldenrod(238.f, 232.f, 170.f, 255.f);
    const ENGINE_EXPORT Color Color::Khaki(240.f, 230.f, 140.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkKhaki(189.f, 183.f, 107.f, 255.f);
    const ENGINE_EXPORT Color Color::Lavender(230.f, 230.f, 250.f, 255.f);
    const ENGINE_EXPORT Color Color::Thistle(216.f, 191.f, 216.f, 255.f);
    const ENGINE_EXPORT Color Color::Plum(221.f, 160.f, 221.f, 255.f);
    const ENGINE_EXPORT Color Color::Violet(238.f, 130.f, 238.f, 255.f);
    const ENGINE_EXPORT Color Color::Orchid(218.f, 112.f, 214.f, 255.f);
    const ENGINE_EXPORT Color Color::Fuchsia(255.f, 0.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::Magenta(255.f, 0.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumOrchid(186.f, 85.f, 211.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumPurple(147.f, 112.f, 219.f, 255.f);
    const ENGINE_EXPORT Color Color::BlueViolet(138.f, 43.f, 226.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkViolet(148.f, 0.f, 211.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkOrchid(153.f, 50.f, 204.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkMagenta(139.f, 0.f, 139.f, 255.f);
    const ENGINE_EXPORT Color Color::Purple(128.f, 0.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::Indigo(75.f, 0.f, 130.f, 255.f);
    const ENGINE_EXPORT Color Color::SlateBlue(106.f, 90.f, 205.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkSlateBlue(72.f, 61.f, 139.f, 255.f);
    const ENGINE_EXPORT Color Color::Cornsilk(255.f, 248.f, 220.f, 255.f);
    const ENGINE_EXPORT Color Color::BlanchedAlmond(255.f, 235.f, 205.f, 255.f);
    const ENGINE_EXPORT Color Color::Bisque(255.f, 228.f, 196.f, 255.f);
    const ENGINE_EXPORT Color Color::NavajoWhite(255.f, 222.f, 173.f, 255.f);
    const ENGINE_EXPORT Color Color::Wheat(245.f, 222.f, 179.f, 255.f);
    const ENGINE_EXPORT Color Color::BurlyWood(222.f, 184.f, 135.f, 255.f);
    const ENGINE_EXPORT Color Color::Tan(210.f, 180.f, 140.f, 255.f);
    const ENGINE_EXPORT Color Color::RosyBrown(188.f, 143.f, 143.f, 255.f);
    const ENGINE_EXPORT Color Color::SandyBrown(244.f, 164.f, 96.f, 255.f);
    const ENGINE_EXPORT Color Color::Goldenrod(218.f, 165.f, 32.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkGoldenRod(184.f, 134.f, 11.f, 255.f);
    const ENGINE_EXPORT Color Color::Peru(205.f, 133.f, 63.f, 255.f);
    const ENGINE_EXPORT Color Color::Chocolate(210.f, 105.f, 30.f, 255.f);
    const ENGINE_EXPORT Color Color::SaddleBrown(139.f, 69.f, 19.f, 255.f);
    const ENGINE_EXPORT Color Color::Sienna(160.f, 82.f, 45.f, 255.f);
    const ENGINE_EXPORT Color Color::Brown(165.f, 42.f, 42.f, 255.f);
    const ENGINE_EXPORT Color Color::Maroon(128.f, 0.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::GreenYellow(173.f, 255.f, 47.f, 255.f);
    const ENGINE_EXPORT Color Color::Chartreuse(127.f, 255.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::LawnGreen(124.f, 252.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::Lime(0.f, 255.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::LimeGreen(50.f, 205.f, 50.f, 255.f);
    const ENGINE_EXPORT Color Color::PaleGreen(152.f, 251.f, 152.f, 255.f);
    const ENGINE_EXPORT Color Color::LightGreen(144.f, 238.f, 144.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumSpringGreen(0.f, 250.f, 154.f, 255.f);
    const ENGINE_EXPORT Color Color::SpringGreen(0.f, 255.f, 127.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumSeaGreen(60.f, 179.f, 113.f, 255.f);
    const ENGINE_EXPORT Color Color::SeaGreen(46.f, 139.f, 87.f, 255.f);
    const ENGINE_EXPORT Color Color::ForestGreen(34.f, 139.f, 34.f, 255.f);
    const ENGINE_EXPORT Color Color::Green(0.f, 128.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkGreen(0.f, 100.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::YellowGreen(154.f, 205.f, 50.f, 255.f);
    const ENGINE_EXPORT Color Color::OliveDrab(107.f, 142.f, 35.f, 255.f);
    const ENGINE_EXPORT Color Color::Olive(128.f, 128.f, 0.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkOliveGreen(85.f, 107.f, 47.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumAquamarine(102.f, 205.f, 170.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkSeaGreen(143.f, 188.f, 143.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSeaGreen(32.f, 178.f, 170.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkCyan(0.f, 139.f, 139.f, 255.f);
    const ENGINE_EXPORT Color Color::Teal(0.f, 128.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::Aqua(0.f, 255.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::Cyan(0.f, 255.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::LightCyan(224.f, 255.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::PaleTurquoise(175.f, 238.f, 238.f, 255.f);
    const ENGINE_EXPORT Color Color::Aquamarine(127.f, 255.f, 212.f, 255.f);
    const ENGINE_EXPORT Color Color::Turquoise(64.f, 224.f, 208.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumTurquoise(72.f, 209.f, 204.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkTurquoise(0.f, 206.f, 209.f, 255.f);
    const ENGINE_EXPORT Color Color::CadetBlue(95.f, 158.f, 160.f, 255.f);
    const ENGINE_EXPORT Color Color::SteelBlue(70.f, 130.f, 180.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSteelBlue(176.f, 196.f, 222.f, 255.f);
    const ENGINE_EXPORT Color Color::PowderBlue(176.f, 224.f, 230.f, 255.f);
    const ENGINE_EXPORT Color Color::LightBlue(173.f, 216.f, 230.f, 255.f);
    const ENGINE_EXPORT Color Color::SkyBlue(135.f, 206.f, 235.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSkyBlue(135.f, 206.f, 250.f, 255.f);
    const ENGINE_EXPORT Color Color::DeepSkyBlue(0.f, 191.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::DodgerBlue(30.f, 144.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::CornflowerBlue(100.f, 149.f, 237.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumSlateBlue(123.f, 104.f, 238.f, 255.f);
    const ENGINE_EXPORT Color Color::RoyalBlue(65.f, 105.f, 225.f, 255.f);
    const ENGINE_EXPORT Color Color::Blue(0.f, 0.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::MediumBlue(0.f, 0.f, 205.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkBlue(0.f, 0.f, 139.f, 255.f);
    const ENGINE_EXPORT Color Color::Navy(0.f, 0.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::MidnightBlue(25.f, 25.f, 112.f, 255.f);
    const ENGINE_EXPORT Color Color::White(255.f, 255.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::Snow(255.f, 250.f, 250.f, 255.f);
    const ENGINE_EXPORT Color Color::Honeydew(240.f, 255.f, 240.f, 255.f);
    const ENGINE_EXPORT Color Color::MintCream(245.f, 255.f, 250.f, 255.f);
    const ENGINE_EXPORT Color Color::Azure(240.f, 255.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::AliceBlue(240.f, 248.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::GhostWhite(248.f, 248.f, 255.f, 255.f);
    const ENGINE_EXPORT Color Color::WhiteSmoke(245.f, 245.f, 245.f, 255.f);
    const ENGINE_EXPORT Color Color::Seashell(255.f, 245.f, 238.f, 255.f);
    const ENGINE_EXPORT Color Color::Beige(245.f, 245.f, 220.f, 255.f);
    const ENGINE_EXPORT Color Color::OldLace(253.f, 245.f, 230.f, 255.f);
    const ENGINE_EXPORT Color Color::FloralWhite(255.f, 250.f, 240.f, 255.f);
    const ENGINE_EXPORT Color Color::Ivory(255.f, 255.f, 240.f, 255.f);
    const ENGINE_EXPORT Color Color::AntiqueWhite(250.f, 235.f, 215.f, 255.f);
    const ENGINE_EXPORT Color Color::Linen(250.f, 240.f, 230.f, 255.f);
    const ENGINE_EXPORT Color Color::LavenderBlush(255.f, 240.f, 245.f, 255.f);
    const ENGINE_EXPORT Color Color::MistyRose(255.f, 228.f, 225.f, 255.f);
    const ENGINE_EXPORT Color Color::Gainsboro(220.f, 220.f, 220.f, 255.f);
    const ENGINE_EXPORT Color Color::LightGrey(211.f, 211.f, 211.f, 255.f);
    const ENGINE_EXPORT Color Color::LightGray(211.f, 211.f, 211.f, 255.f);
    const ENGINE_EXPORT Color Color::Silver(192.f, 192.f, 192.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkGray(169.f, 169.f, 169.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkGrey(169.f, 169.f, 169.f, 255.f);
    const ENGINE_EXPORT Color Color::Gray(128.f, 128.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::Grey(128.f, 128.f, 128.f, 255.f);
    const ENGINE_EXPORT Color Color::DimGray(105.f, 105.f, 105.f, 255.f);
    const ENGINE_EXPORT Color Color::DimGrey(105.f, 105.f, 105.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSlateGray(119.f, 136.f, 153.f, 255.f);
    const ENGINE_EXPORT Color Color::LightSlateGrey(119.f, 136.f, 153.f, 255.f);
    const ENGINE_EXPORT Color Color::SlateGray(112.f, 128.f, 144.f, 255.f);
    const ENGINE_EXPORT Color Color::SlateGrey(112.f, 128.f, 144.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkSlateGray(47.f, 79.f, 79.f, 255.f);
    const ENGINE_EXPORT Color Color::DarkSlateGrey(47.f, 79.f, 79.f, 255.f);
    const ENGINE_EXPORT Color Color::Black(0.f, 0.f, 0.f, 255.f);
}// namespace Engine
