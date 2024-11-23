#pragma once
#include <U8g2lib.h>

namespace Icons {
    // Icon dimensions
    enum class Size {
        SMALL = 32,
        LARGE = 64
    };

    // Icon types
    enum class Type {
        FOX_1,
        FOX_2,
        FOX_3
    };

    // Icon polarity
    enum class Polarity {
        POSITIVE,
        NEGATIVE
    };

    // Structure to hold icon metadata
    struct IconInfo {
        const unsigned char* data;
        Size size;
        Type type;
        Polarity polarity;
        u8g2_uint_t width;
        u8g2_uint_t height;
    };

    // Function declarations
    IconInfo getIconXBM(Type type, Polarity polarity = Polarity::POSITIVE);
    void freeXBMData(const IconInfo& icon);

    // External bitmap arrays
    extern const int epd_bitmap_foxes_positive_array_LEN;
    extern const int epd_bitmap_foxes_negative_array_LEN;
    extern const unsigned char* const epd_bitmap_foxes_positive_array[];
    extern const unsigned char* const epd_bitmap_foxes_negative_array[];
}

class XBMIcon
{
private:
    const uint8_t* data;
    u8g2_uint_t w;
    u8g2_uint_t h;

public:
    XBMIcon(const uint8_t* _data, u8g2_uint_t _width, u8g2_uint_t _height)
        : data(_data), w(_width), h(_height) {}

    u8g2_uint_t width() const { return w; }
    u8g2_uint_t height() const { return h; }
    const uint8_t* getData() const { return data; }
};