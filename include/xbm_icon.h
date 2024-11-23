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
    Icons::IconInfo icon;

public:
    XBMIcon(Icons::Type type, Icons::Polarity polarity = Icons::Polarity::POSITIVE)
    {
        icon = Icons::getIconXBM(type, polarity);
    }

    ~XBMIcon()
    {
        if (icon.data)
        {
            Icons::freeXBMData(icon);
        }
    }

    // Prevent copying
    XBMIcon(const XBMIcon &) = delete;
    XBMIcon &operator=(const XBMIcon &) = delete;

    bool isValid() const { return icon.data != nullptr; }
    const unsigned char *data() const { return icon.data; }
    u8g2_uint_t width() const { return icon.width; }
    u8g2_uint_t height() const { return icon.height; }
};