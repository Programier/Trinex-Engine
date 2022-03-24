#pragma once


namespace Engine
{

    class Color
    {
    private:
        float values[4];
        float& red = values[0];
        float& green = values[1];
        float& blue = values[2];
        float& alpha = values[3];
        void validate();

    public:
        Color(float red = 0, float green = 0, float blue = 0, float alpha = 1);
        Color(const Color& color);
        Color& operator = (const Color& color);
        float R() const;
        float G() const;
        float B() const;
        float A() const;

        Color& R(float red);
        Color& G(float green);
        Color& B(float blue);
        Color& A(float alpha);
    };


#define IndianRed                                                                                  \
    {                                                                                              \
        205, 92, 92                                                                                \
    }
#define LightCoral                                                                                 \
    {                                                                                              \
        240, 128, 128                                                                              \
    }
#define Salmon                                                                                     \
    {                                                                                              \
        250, 128, 114                                                                              \
    }
#define DarkSalmon                                                                                 \
    {                                                                                              \
        233, 150, 122                                                                              \
    }
#define LightSalmon                                                                                \
    {                                                                                              \
        255, 160, 122                                                                              \
    }
#define Crimson                                                                                    \
    {                                                                                              \
        220, 20, 60                                                                                \
    }
#define Red                                                                                        \
    {                                                                                              \
        255, 0, 0                                                                                  \
    }
#define FireBrick                                                                                  \
    {                                                                                              \
        178, 34, 34                                                                                \
    }
#define DarkRed                                                                                    \
    {                                                                                              \
        139, 0, 0                                                                                  \
    }

#define Pink                                                                                       \
    {                                                                                              \
        255, 192, 203                                                                              \
    }
#define LightPink                                                                                  \
    {                                                                                              \
        255, 182, 193                                                                              \
    }
#define HotPink                                                                                    \
    {                                                                                              \
        255, 105, 180                                                                              \
    }
#define DeepPink                                                                                   \
    {                                                                                              \
        255, 20, 147                                                                               \
    }
#define MediumVioletRed                                                                            \
    {                                                                                              \
        199, 21, 133                                                                               \
    }
#define PaleVioletRed                                                                              \
    {                                                                                              \
        219, 112, 147                                                                              \
    }
#define LightSalmon                                                                                \
    {                                                                                              \
        255, 160, 122                                                                              \
    }
#define Coral                                                                                      \
    {                                                                                              \
        255, 127, 80                                                                               \
    }
#define Tomato                                                                                     \
    {                                                                                              \
        255, 99, 71                                                                                \
    }
#define OrangeRed                                                                                  \
    {                                                                                              \
        255, 69, 0                                                                                 \
    }
#define DarkOrange                                                                                 \
    {                                                                                              \
        255, 140, 0                                                                                \
    }
#define Orange                                                                                     \
    {                                                                                              \
        255, 165, 0                                                                                \
    }
#define Gold                                                                                       \
    {                                                                                              \
        255, 215, 0                                                                                \
    }
#define Yellow                                                                                     \
    {                                                                                              \
        255, 255, 0                                                                                \
    }
#define LightYellow                                                                                \
    {                                                                                              \
        255, 255, 224                                                                              \
    }
#define LemonChiffon                                                                               \
    {                                                                                              \
        255, 250, 205                                                                              \
    }
#define LightGoldenrodYellow                                                                       \
    {                                                                                              \
        250, 250, 210                                                                              \
    }
#define PapayaWhip                                                                                 \
    {                                                                                              \
        255, 239, 213                                                                              \
    }
#define Moccasin                                                                                   \
    {                                                                                              \
        255, 228, 181                                                                              \
    }
#define PeachPuff                                                                                  \
    {                                                                                              \
        255, 218, 185                                                                              \
    }
#define PaleGoldenrod                                                                              \
    {                                                                                              \
        238, 232, 170                                                                              \
    }
#define Khaki                                                                                      \
    {                                                                                              \
        240, 230, 140                                                                              \
    }
#define DarkKhaki                                                                                  \
    {                                                                                              \
        189, 183, 107                                                                              \
    }
#define Lavender                                                                                   \
    {                                                                                              \
        230, 230, 250                                                                              \
    }
#define Thistle                                                                                    \
    {                                                                                              \
        216, 191, 216                                                                              \
    }
#define Plum                                                                                       \
    {                                                                                              \
        221, 160, 221                                                                              \
    }
#define Violet                                                                                     \
    {                                                                                              \
        238, 130, 238                                                                              \
    }
#define Orchid                                                                                     \
    {                                                                                              \
        218, 112, 214                                                                              \
    }
#define Fuchsia                                                                                    \
    {                                                                                              \
        255, 0, 255                                                                                \
    }
#define Magenta                                                                                    \
    {                                                                                              \
        255, 0, 255                                                                                \
    }
#define MediumOrchid                                                                               \
    {                                                                                              \
        186, 85, 211                                                                               \
    }
#define MediumPurple                                                                               \
    {                                                                                              \
        147, 112, 219                                                                              \
    }
#define BlueViolet                                                                                 \
    {                                                                                              \
        138, 43, 226                                                                               \
    }
#define DarkViolet                                                                                 \
    {                                                                                              \
        148, 0, 211                                                                                \
    }
#define DarkOrchid                                                                                 \
    {                                                                                              \
        153, 50, 204                                                                               \
    }
#define DarkMagenta                                                                                \
    {                                                                                              \
        139, 0, 139                                                                                \
    }
#define Purple                                                                                     \
    {                                                                                              \
        128, 0, 128                                                                                \
    }
#define Indigo                                                                                     \
    {                                                                                              \
        75, 0, 130                                                                                 \
    }
#define SlateBlue                                                                                  \
    {                                                                                              \
        106, 90, 205                                                                               \
    }
#define DarkSlateBlue                                                                              \
    {                                                                                              \
        72, 61, 139                                                                                \
    }
#define Cornsilk                                                                                   \
    {                                                                                              \
        255, 248, 220                                                                              \
    }
#define BlanchedAlmond                                                                             \
    {                                                                                              \
        255, 235, 205                                                                              \
    }
#define Bisque                                                                                     \
    {                                                                                              \
        255, 228, 196                                                                              \
    }
#define NavajoWhite                                                                                \
    {                                                                                              \
        255, 222, 173                                                                              \
    }
#define Wheat                                                                                      \
    {                                                                                              \
        245, 222, 179                                                                              \
    }
#define BurlyWood                                                                                  \
    {                                                                                              \
        222, 184, 135                                                                              \
    }
#define Tan                                                                                        \
    {                                                                                              \
        210, 180, 140                                                                              \
    }
#define RosyBrown                                                                                  \
    {                                                                                              \
        188, 143, 143                                                                              \
    }
#define SandyBrown                                                                                 \
    {                                                                                              \
        244, 164, 96                                                                               \
    }
#define Goldenrod                                                                                  \
    {                                                                                              \
        218, 165, 32                                                                               \
    }
#define DarkGoldenRod                                                                              \
    {                                                                                              \
        184, 134, 11                                                                               \
    }
#define Peru                                                                                       \
    {                                                                                              \
        205, 133, 63                                                                               \
    }
#define Chocolate                                                                                  \
    {                                                                                              \
        210, 105, 30                                                                               \
    }
#define SaddleBrown                                                                                \
    {                                                                                              \
        139, 69, 19                                                                                \
    }
#define Sienna                                                                                     \
    {                                                                                              \
        160, 82, 45                                                                                \
    }
#define Brown                                                                                      \
    {                                                                                              \
        165, 42, 42                                                                                \
    }
#define Maroon                                                                                     \
    {                                                                                              \
        128, 0, 0                                                                                  \
    }
#define GreenYellow                                                                                \
    {                                                                                              \
        173, 255, 47                                                                               \
    }
#define Chartreuse                                                                                 \
    {                                                                                              \
        127, 255, 0                                                                                \
    }
#define LawnGreen                                                                                  \
    {                                                                                              \
        124, 252, 0                                                                                \
    }
#define Lime                                                                                       \
    {                                                                                              \
        0, 255, 0                                                                                  \
    }
#define LimeGreen                                                                                  \
    {                                                                                              \
        50, 205, 50                                                                                \
    }
#define PaleGreen                                                                                  \
    {                                                                                              \
        152, 251, 152                                                                              \
    }
#define LightGreen                                                                                 \
    {                                                                                              \
        144, 238, 144                                                                              \
    }
#define MediumSpringGreen                                                                          \
    {                                                                                              \
        0, 250, 154                                                                                \
    }
#define SpringGreen                                                                                \
    {                                                                                              \
        0, 255, 127                                                                                \
    }
#define MediumSeaGreen                                                                             \
    {                                                                                              \
        60, 179, 113                                                                               \
    }
#define SeaGreen                                                                                   \
    {                                                                                              \
        46, 139, 87                                                                                \
    }
#define ForestGreen                                                                                \
    {                                                                                              \
        34, 139, 34                                                                                \
    }
#define Green                                                                                      \
    {                                                                                              \
        0, 128, 0                                                                                  \
    }
#define DarkGreen                                                                                  \
    {                                                                                              \
        0, 100, 0                                                                                  \
    }
#define YellowGreen                                                                                \
    {                                                                                              \
        154, 205, 50                                                                               \
    }
#define OliveDrab                                                                                  \
    {                                                                                              \
        107, 142, 35                                                                               \
    }
#define Olive                                                                                      \
    {                                                                                              \
        128, 128, 0                                                                                \
    }
#define DarkOliveGreen                                                                             \
    {                                                                                              \
        85, 107, 47                                                                                \
    }
#define MediumAquamarine                                                                           \
    {                                                                                              \
        102, 205, 170                                                                              \
    }
#define DarkSeaGreen                                                                               \
    {                                                                                              \
        143, 188, 143                                                                              \
    }
#define LightSeaGreen                                                                              \
    {                                                                                              \
        32, 178, 170                                                                               \
    }
#define DarkCyan                                                                                   \
    {                                                                                              \
        0, 139, 139                                                                                \
    }
#define Teal                                                                                       \
    {                                                                                              \
        0, 128, 128                                                                                \
    }
#define Aqua                                                                                       \
    {                                                                                              \
        0, 255, 255                                                                                \
    }
#define Cyan                                                                                       \
    {                                                                                              \
        0, 255, 255                                                                                \
    }
#define LightCyan                                                                                  \
    {                                                                                              \
        224, 255, 255                                                                              \
    }
#define PaleTurquoise                                                                              \
    {                                                                                              \
        175, 238, 238                                                                              \
    }
#define Aquamarine                                                                                 \
    {                                                                                              \
        127, 255, 212                                                                              \
    }
#define Turquoise                                                                                  \
    {                                                                                              \
        64, 224, 208                                                                               \
    }
#define MediumTurquoise                                                                            \
    {                                                                                              \
        72, 209, 204                                                                               \
    }
#define DarkTurquoise                                                                              \
    {                                                                                              \
        0, 206, 209                                                                                \
    }
#define CadetBlue                                                                                  \
    {                                                                                              \
        95, 158, 160                                                                               \
    }
#define SteelBlue                                                                                  \
    {                                                                                              \
        70, 130, 180                                                                               \
    }
#define LightSteelBlue                                                                             \
    {                                                                                              \
        176, 196, 222                                                                              \
    }
#define PowderBlue                                                                                 \
    {                                                                                              \
        176, 224, 230                                                                              \
    }
#define LightBlue                                                                                  \
    {                                                                                              \
        173, 216, 230                                                                              \
    }
#define SkyBlue                                                                                    \
    {                                                                                              \
        135, 206, 235                                                                              \
    }
#define LightSkyBlue                                                                               \
    {                                                                                              \
        135, 206, 250                                                                              \
    }
#define DeepSkyBlue                                                                                \
    {                                                                                              \
        0, 191, 255                                                                                \
    }
#define DodgerBlue                                                                                 \
    {                                                                                              \
        30, 144, 255                                                                               \
    }
#define CornflowerBlue                                                                             \
    {                                                                                              \
        100, 149, 237                                                                              \
    }
#define MediumSlateBlue                                                                            \
    {                                                                                              \
        123, 104, 238                                                                              \
    }
#define RoyalBlue                                                                                  \
    {                                                                                              \
        65, 105, 225                                                                               \
    }
#define Blue                                                                                       \
    {                                                                                              \
        0, 0, 255                                                                                  \
    }
#define MediumBlue                                                                                 \
    {                                                                                              \
        0, 0, 205                                                                                  \
    }
#define DarkBlue                                                                                   \
    {                                                                                              \
        0, 0, 139                                                                                  \
    }
#define Navy                                                                                       \
    {                                                                                              \
        0, 0, 128                                                                                  \
    }
#define MidnightBlue                                                                               \
    {                                                                                              \
        25, 25, 112                                                                                \
    }
#define White                                                                                      \
    {                                                                                              \
        255, 255, 255                                                                              \
    }
#define Snow                                                                                       \
    {                                                                                              \
        255, 250, 250                                                                              \
    }
#define Honeydew                                                                                   \
    {                                                                                              \
        240, 255, 240                                                                              \
    }
#define MintCream                                                                                  \
    {                                                                                              \
        245, 255, 250                                                                              \
    }
#define Azure                                                                                      \
    {                                                                                              \
        240, 255, 255                                                                              \
    }
#define AliceBlue                                                                                  \
    {                                                                                              \
        240, 248, 255                                                                              \
    }
#define GhostWhite                                                                                 \
    {                                                                                              \
        248, 248, 255                                                                              \
    }
#define WhiteSmoke                                                                                 \
    {                                                                                              \
        245, 245, 245                                                                              \
    }
#define Seashell                                                                                   \
    {                                                                                              \
        255, 245, 238                                                                              \
    }
#define Beige                                                                                      \
    {                                                                                              \
        245, 245, 220                                                                              \
    }
#define OldLace                                                                                    \
    {                                                                                              \
        253, 245, 230                                                                              \
    }
#define FloralWhite                                                                                \
    {                                                                                              \
        255, 250, 240                                                                              \
    }
#define Ivory                                                                                      \
    {                                                                                              \
        255, 255, 240                                                                              \
    }
#define AntiqueWhite                                                                               \
    {                                                                                              \
        250, 235, 215                                                                              \
    }
#define Linen                                                                                      \
    {                                                                                              \
        250, 240, 230                                                                              \
    }
#define LavenderBlush                                                                              \
    {                                                                                              \
        255, 240, 245                                                                              \
    }
#define MistyRose                                                                                  \
    {                                                                                              \
        255, 228, 225                                                                              \
    }
#define Gainsboro                                                                                  \
    {                                                                                              \
        220, 220, 220                                                                              \
    }
#define LightGrey                                                                                  \
    {                                                                                              \
        211, 211, 211                                                                              \
    }
#define LightGray                                                                                  \
    {                                                                                              \
        211, 211, 211                                                                              \
    }
#define Silver                                                                                     \
    {                                                                                              \
        192, 192, 192                                                                              \
    }
#define DarkGray                                                                                   \
    {                                                                                              \
        169, 169, 169                                                                              \
    }
#define DarkGrey                                                                                   \
    {                                                                                              \
        169, 169, 169                                                                              \
    }
#define Gray                                                                                       \
    {                                                                                              \
        128, 128, 128                                                                              \
    }
#define Grey                                                                                       \
    {                                                                                              \
        128, 128, 128                                                                              \
    }
#define DimGray                                                                                    \
    {                                                                                              \
        105, 105, 105                                                                              \
    }
#define DimGrey                                                                                    \
    {                                                                                              \
        105, 105, 105                                                                              \
    }
#define LightSlateGray                                                                             \
    {                                                                                              \
        119, 136, 153                                                                              \
    }
#define LightSlateGrey                                                                             \
    {                                                                                              \
        119, 136, 153                                                                              \
    }
#define SlateGray                                                                                  \
    {                                                                                              \
        112, 128, 144                                                                              \
    }
#define SlateGrey                                                                                  \
    {                                                                                              \
        112, 128, 144                                                                              \
    }
#define DarkSlateGray                                                                              \
    {                                                                                              \
        47, 79, 79                                                                                 \
    }
#define DarkSlateGrey                                                                              \
    {                                                                                              \
        47, 79, 79                                                                                 \
    }
#define Black                                                                                      \
    {                                                                                              \
        0, 0, 0                                                                                    \
    }

}// namespace Engine
