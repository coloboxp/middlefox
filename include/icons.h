#ifndef ICONS_H
#define ICONS_H

#include <pgmspace.h>
#include "esp_log.h"

namespace Icons
{
    // Add tag for ESP logging
    static const char* TAG = "Icons";

    // Icon dimensions
    enum class Size
    {
        SMALL = 32,
        LARGE = 64
    };

    // Icon types
    enum class Type
    {
        FOX_1,
        FOX_2,
        FOX_3
    };

    // Icon polarity
    enum class Polarity
    {
        POSITIVE,
        NEGATIVE
    };

    // Structure to hold icon metadata
    struct IconInfo
    {
        const unsigned char *data;
        Size size;
        Type type;
        Polarity polarity;
        u8g2_uint_t width;
        u8g2_uint_t height;
    };

    // Original image data (kept as-is)
    // 'fox_5605972 (2)', 32x32px
    const unsigned char epd_bitmap_fox_32x32_1_p[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x20, 0x10, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x14, 0x44, 0x00, 0x01, 0x13, 0x70, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x04,
        0x00, 0x04, 0xa0, 0x04, 0x00, 0x02, 0x80, 0x02, 0x00, 0x00, 0x48, 0x02, 0x00, 0x01, 0x04, 0x0a,
        0x00, 0x00, 0x83, 0x10, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x60, 0xba,
        0x00, 0x0d, 0x08, 0x08, 0x00, 0x10, 0x06, 0xe0, 0x00, 0x22, 0x00, 0x20, 0x00, 0x44, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x40, 0x00, 0x84, 0xa0, 0x40, 0x03, 0x04, 0x40, 0x80, 0x08, 0x04, 0xa5, 0x40,
        0xe8, 0x0c, 0xa6, 0x00, 0x80, 0x05, 0x08, 0x80, 0x40, 0x14, 0x2a, 0x80, 0x18, 0x22, 0x08, 0x80,
        0x07, 0x84, 0x12, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};

    const unsigned char epd_bitmap_fox_32x32_2_p[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x50, 0x14, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x94, 0x50, 0x00, 0x02, 0x13, 0xf4, 0x00, 0x03, 0x14, 0x0c, 0x00, 0x04, 0x10, 0x94,
        0x00, 0x02, 0xa5, 0x04, 0x00, 0x02, 0xd0, 0x52, 0x00, 0x01, 0x15, 0x0a, 0x00, 0x01, 0x4c, 0x54,
        0x00, 0x00, 0x43, 0x92, 0x00, 0x00, 0x50, 0x0a, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x61, 0x75,
        0x00, 0x0d, 0x98, 0x08, 0x00, 0x12, 0x06, 0xe0, 0x00, 0x2a, 0xa0, 0x20, 0x00, 0x54, 0x10, 0x20,
        0x00, 0x4d, 0x48, 0x80, 0x00, 0xa4, 0xa0, 0x40, 0x03, 0x55, 0x68, 0xc0, 0x0a, 0x14, 0xa5, 0x00,
        0xe8, 0xad, 0x2e, 0x80, 0x80, 0xaa, 0x22, 0x80, 0x40, 0x55, 0x1a, 0x80, 0x19, 0x24, 0x11, 0x00,
        0x06, 0x85, 0x19, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};
    // 'fox_5606000 (1)', 64x64px
    const unsigned char epd_bitmap_fox_64x64_1_p[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x30,
        0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x01, 0x98, 0x00, 0x01, 0x90,
        0x00, 0x00, 0x00, 0x03, 0x4e, 0x00, 0x03, 0x50, 0x00, 0x00, 0x00, 0x02, 0x73, 0x00, 0x06, 0xb0,
        0x00, 0x00, 0x00, 0x06, 0xd9, 0x80, 0x0d, 0xb0, 0x00, 0x00, 0x00, 0x05, 0x8c, 0xc0, 0x19, 0x10,
        0x00, 0x00, 0x00, 0x0c, 0x86, 0x60, 0x15, 0x30, 0x00, 0x00, 0x00, 0x09, 0x83, 0x30, 0x31, 0x30,
        0x00, 0x00, 0x00, 0x0b, 0x01, 0x5f, 0xfd, 0x90, 0x00, 0x00, 0x00, 0x09, 0x01, 0x84, 0x2f, 0x30,
        0x00, 0x00, 0x00, 0x1b, 0x01, 0x21, 0x41, 0x90, 0x00, 0x00, 0x00, 0x11, 0x01, 0x4a, 0x14, 0xd0,
        0x00, 0x00, 0x00, 0x1b, 0x03, 0x10, 0x82, 0x70, 0x00, 0x00, 0x00, 0x11, 0x8a, 0x84, 0x51, 0x30,
        0x00, 0x00, 0x00, 0x1a, 0x86, 0x52, 0x8a, 0x58, 0x00, 0x00, 0x00, 0x08, 0x8c, 0x08, 0x20, 0x10,
        0x00, 0x00, 0x00, 0x0a, 0xd9, 0x52, 0x95, 0x58, 0x00, 0x00, 0x00, 0x0c, 0x72, 0x04, 0x40, 0x08,
        0x00, 0x00, 0x00, 0x05, 0x29, 0x51, 0x15, 0x58, 0x00, 0x00, 0x00, 0x06, 0x45, 0x88, 0x80, 0xa8,
        0x00, 0x00, 0x00, 0x03, 0x20, 0xe5, 0x2a, 0x2c, 0x00, 0x00, 0x00, 0x01, 0xca, 0x70, 0x41, 0xe4,
        0x00, 0x00, 0x00, 0x00, 0x64, 0x9f, 0x16, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x31, 0x05, 0xc3, 0x54,
        0x00, 0x00, 0x00, 0x00, 0x14, 0x52, 0x28, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x82, 0x56,
        0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x0f, 0x02, 0x00, 0x00, 0x00, 0x00, 0x31, 0x08, 0x09, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x67, 0x4e, 0x00, 0x00, 0x00, 0x1f, 0xfe, 0x20, 0x3f, 0xb8,
        0x00, 0x00, 0x00, 0x75, 0xc3, 0x80, 0x00, 0xe0, 0x00, 0x00, 0x01, 0xc7, 0x10, 0xe0, 0x07, 0x00,
        0x00, 0x00, 0x01, 0x2c, 0x44, 0x34, 0x56, 0x00, 0x00, 0x00, 0x07, 0x4d, 0x12, 0x1f, 0xbc, 0x00,
        0x00, 0x00, 0x04, 0x98, 0x49, 0x01, 0x44, 0x00, 0x00, 0x00, 0x0d, 0x2a, 0x84, 0x80, 0x0c, 0x00,
        0x00, 0x00, 0x18, 0x98, 0x28, 0x08, 0x28, 0x00, 0x00, 0x00, 0x35, 0x55, 0x42, 0x81, 0x08, 0x00,
        0x00, 0x00, 0x28, 0x50, 0x11, 0x20, 0x18, 0x00, 0x00, 0x00, 0x65, 0x55, 0x4a, 0x00, 0x10, 0x00,
        0x00, 0x00, 0xd2, 0x53, 0x98, 0x80, 0x30, 0x00, 0x00, 0x01, 0xa9, 0x70, 0xca, 0x42, 0x20, 0x00,
        0x00, 0x03, 0x05, 0x34, 0xf8, 0x00, 0x60, 0x00, 0x00, 0x06, 0xa9, 0x62, 0x5d, 0x60, 0xe0, 0x00,
        0x00, 0x1c, 0x45, 0x34, 0x68, 0xa1, 0xa0, 0x00, 0x01, 0x72, 0x94, 0xe9, 0x4c, 0x27, 0x60, 0x00,
        0xff, 0x80, 0x52, 0x70, 0xc5, 0x7e, 0xa0, 0x00, 0xa0, 0x02, 0x95, 0xe5, 0x8c, 0x4c, 0x40, 0x00,
        0xc0, 0x00, 0xaa, 0xb3, 0x05, 0x45, 0x40, 0x00, 0x60, 0x11, 0x49, 0xa6, 0x04, 0xc4, 0xc0, 0x00,
        0x31, 0x00, 0x25, 0x34, 0x06, 0x46, 0xc0, 0x00, 0x1c, 0x04, 0x17, 0x24, 0x05, 0x44, 0x80, 0x00,
        0x07, 0x20, 0x86, 0x14, 0x02, 0xc6, 0x80, 0x00, 0x01, 0xc0, 0x38, 0x34, 0x06, 0x44, 0x80, 0x00,
        0x00, 0x7f, 0xe0, 0x16, 0x02, 0xc6, 0x80, 0x00, 0x00, 0x05, 0x00, 0x16, 0x02, 0x42, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x1e, 0x02, 0xc2, 0x80, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x43, 0xc0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0xe2, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x41, 0xc0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0xa0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};

    const unsigned char epd_bitmap_fox_32x32_1_n[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x20, 0x10, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x14, 0x44, 0x00, 0x01, 0x13, 0x70, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x04,
        0x00, 0x04, 0xa0, 0x04, 0x00, 0x02, 0x80, 0x02, 0x00, 0x00, 0x48, 0x02, 0x00, 0x01, 0x04, 0x0a,
        0x00, 0x00, 0x83, 0x10, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x60, 0xba,
        0x00, 0x0d, 0x08, 0x08, 0x00, 0x10, 0x06, 0xe0, 0x00, 0x22, 0x00, 0x20, 0x00, 0x44, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x40, 0x00, 0x84, 0xa0, 0x40, 0x03, 0x04, 0x40, 0x80, 0x08, 0x04, 0xa5, 0x40,
        0xe8, 0x0c, 0xa6, 0x00, 0x80, 0x05, 0x08, 0x80, 0x40, 0x14, 0x2a, 0x80, 0x18, 0x22, 0x08, 0x80,
        0x07, 0x84, 0x12, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};
    // 'fox_5606000', 32x32px
    const unsigned char epd_bitmap_fox_32x32_2_n[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x50, 0x14, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x94, 0x50, 0x00, 0x02, 0x13, 0xf4, 0x00, 0x03, 0x14, 0x0c, 0x00, 0x04, 0x10, 0x94,
        0x00, 0x02, 0xa5, 0x04, 0x00, 0x02, 0xd0, 0x52, 0x00, 0x01, 0x15, 0x0a, 0x00, 0x01, 0x4c, 0x54,
        0x00, 0x00, 0x43, 0x92, 0x00, 0x00, 0x50, 0x0a, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x61, 0x75,
        0x00, 0x0d, 0x98, 0x08, 0x00, 0x12, 0x06, 0xe0, 0x00, 0x2a, 0xa0, 0x20, 0x00, 0x54, 0x10, 0x20,
        0x00, 0x4d, 0x48, 0x80, 0x00, 0xa4, 0xa0, 0x40, 0x03, 0x55, 0x68, 0xc0, 0x0a, 0x14, 0xa5, 0x00,
        0xe8, 0xad, 0x2e, 0x80, 0x80, 0xaa, 0x22, 0x80, 0x40, 0x55, 0x1a, 0x80, 0x19, 0x24, 0x11, 0x00,
        0x06, 0x85, 0x19, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};
    // 'fox_5606000 (1)', 64x64px
    const unsigned char epd_bitmap_fox_64x64_1_n[] U8X8_PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x30,
        0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x01, 0x98, 0x00, 0x01, 0x90,
        0x00, 0x00, 0x00, 0x03, 0x4e, 0x00, 0x03, 0x50, 0x00, 0x00, 0x00, 0x02, 0x73, 0x00, 0x06, 0xb0,
        0x00, 0x00, 0x00, 0x06, 0xd9, 0x80, 0x0d, 0xb0, 0x00, 0x00, 0x00, 0x05, 0x8c, 0xc0, 0x19, 0x10,
        0x00, 0x00, 0x00, 0x0c, 0x86, 0x60, 0x15, 0x30, 0x00, 0x00, 0x00, 0x09, 0x83, 0x30, 0x31, 0x30,
        0x00, 0x00, 0x00, 0x0b, 0x01, 0x5f, 0xfd, 0x90, 0x00, 0x00, 0x00, 0x09, 0x01, 0x84, 0x2f, 0x30,
        0x00, 0x00, 0x00, 0x1b, 0x01, 0x21, 0x41, 0x90, 0x00, 0x00, 0x00, 0x11, 0x01, 0x4a, 0x14, 0xd0,
        0x00, 0x00, 0x00, 0x1b, 0x03, 0x10, 0x82, 0x70, 0x00, 0x00, 0x00, 0x11, 0x8a, 0x84, 0x51, 0x30,
        0x00, 0x00, 0x00, 0x1a, 0x86, 0x52, 0x8a, 0x58, 0x00, 0x00, 0x00, 0x08, 0x8c, 0x08, 0x20, 0x10,
        0x00, 0x00, 0x00, 0x0a, 0xd9, 0x52, 0x95, 0x58, 0x00, 0x00, 0x00, 0x0c, 0x72, 0x04, 0x40, 0x08,
        0x00, 0x00, 0x00, 0x05, 0x29, 0x51, 0x15, 0x58, 0x00, 0x00, 0x00, 0x06, 0x45, 0x88, 0x80, 0xa8,
        0x00, 0x00, 0x00, 0x03, 0x20, 0xe5, 0x2a, 0x2c, 0x00, 0x00, 0x00, 0x01, 0xca, 0x70, 0x41, 0xe4,
        0x00, 0x00, 0x00, 0x00, 0x64, 0x9f, 0x16, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x31, 0x05, 0xc3, 0x54,
        0x00, 0x00, 0x00, 0x00, 0x14, 0x52, 0x28, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x82, 0x56,
        0x00, 0x00, 0x00, 0x00, 0x10, 0x02, 0x0f, 0x02, 0x00, 0x00, 0x00, 0x00, 0x31, 0x08, 0x09, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x67, 0x4e, 0x00, 0x00, 0x00, 0x1f, 0xfe, 0x20, 0x3f, 0xb8,
        0x00, 0x00, 0x00, 0x75, 0xc3, 0x80, 0x00, 0xe0, 0x00, 0x00, 0x01, 0xc7, 0x10, 0xe0, 0x07, 0x00,
        0x00, 0x00, 0x01, 0x2c, 0x44, 0x34, 0x56, 0x00, 0x00, 0x00, 0x07, 0x4d, 0x12, 0x1f, 0xbc, 0x00,
        0x00, 0x00, 0x04, 0x98, 0x49, 0x01, 0x44, 0x00, 0x00, 0x00, 0x0d, 0x2a, 0x84, 0x80, 0x0c, 0x00,
        0x00, 0x00, 0x18, 0x98, 0x28, 0x08, 0x28, 0x00, 0x00, 0x00, 0x35, 0x55, 0x42, 0x81, 0x08, 0x00,
        0x00, 0x00, 0x28, 0x50, 0x11, 0x20, 0x18, 0x00, 0x00, 0x00, 0x65, 0x55, 0x4a, 0x00, 0x10, 0x00,
        0x00, 0x00, 0xd2, 0x53, 0x98, 0x80, 0x30, 0x00, 0x00, 0x01, 0xa9, 0x70, 0xca, 0x42, 0x20, 0x00,
        0x00, 0x03, 0x05, 0x34, 0xf8, 0x00, 0x60, 0x00, 0x00, 0x06, 0xa9, 0x62, 0x5d, 0x60, 0xe0, 0x00,
        0x00, 0x1c, 0x45, 0x34, 0x68, 0xa1, 0xa0, 0x00, 0x01, 0x72, 0x94, 0xe9, 0x4c, 0x27, 0x60, 0x00,
        0xff, 0x80, 0x52, 0x70, 0xc5, 0x7e, 0xa0, 0x00, 0xa0, 0x02, 0x95, 0xe5, 0x8c, 0x4c, 0x40, 0x00,
        0xc0, 0x00, 0xaa, 0xb3, 0x05, 0x45, 0x40, 0x00, 0x60, 0x11, 0x49, 0xa6, 0x04, 0xc4, 0xc0, 0x00,
        0x31, 0x00, 0x25, 0x34, 0x06, 0x46, 0xc0, 0x00, 0x1c, 0x04, 0x17, 0x24, 0x05, 0x44, 0x80, 0x00,
        0x07, 0x20, 0x86, 0x14, 0x02, 0xc6, 0x80, 0x00, 0x01, 0xc0, 0x38, 0x34, 0x06, 0x44, 0x80, 0x00,
        0x00, 0x7f, 0xe0, 0x16, 0x02, 0xc6, 0x80, 0x00, 0x00, 0x05, 0x00, 0x16, 0x02, 0x42, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x1e, 0x02, 0xc2, 0x80, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x43, 0xc0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0xe2, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x41, 0xc0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0xa0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};

    // Keep all other existing image data...
    // ... (your existing image data stays exactly the same) ...

    // Helper array containing all icons with metadata
    const IconInfo ICONS[] = {
        {epd_bitmap_fox_32x32_1_p, Size::SMALL, Type::FOX_1, Polarity::POSITIVE, 32, 32},
        {epd_bitmap_fox_32x32_2_p, Size::SMALL, Type::FOX_2, Polarity::POSITIVE, 32, 32},
        {epd_bitmap_fox_64x64_1_p, Size::LARGE, Type::FOX_3, Polarity::POSITIVE, 64, 64},
        {epd_bitmap_fox_32x32_1_n, Size::SMALL, Type::FOX_1, Polarity::NEGATIVE, 32, 32},
        {epd_bitmap_fox_32x32_2_n, Size::SMALL, Type::FOX_2, Polarity::NEGATIVE, 32, 32},
        {epd_bitmap_fox_64x64_1_n, Size::LARGE, Type::FOX_3, Polarity::NEGATIVE, 64, 64}
    };

    // Helper functions
    inline const IconInfo *getIcon(Type type, Polarity polarity = Polarity::POSITIVE)
    {
        ESP_LOGD(TAG, "Getting icon - Type: %d, Polarity: %d", 
                 static_cast<int>(type), static_cast<int>(polarity));
        
        for (const auto &icon : ICONS)
        {
            if (icon.type == type && icon.polarity == polarity)
            {
                ESP_LOGD(TAG, "Found matching icon - Size: %dx%d", 
                         icon.width, icon.height);
                return &icon;
            }
        }
        ESP_LOGW(TAG, "No matching icon found");
        return nullptr;
    }

    inline const IconInfo *getIconBySize(Size size, Polarity polarity = Polarity::POSITIVE)
    {
        ESP_LOGD(TAG, "Getting icon by size - Size: %d, Polarity: %d",
                 static_cast<int>(size), static_cast<int>(polarity));
        
        for (const auto &icon : ICONS)
        {
            if (icon.size == size && icon.polarity == polarity)
            {
                ESP_LOGD(TAG, "Found matching icon - Size: %dx%d",
                         icon.width, icon.height);
                return &icon;
            }
        }
        ESP_LOGW(TAG, "No matching icon found for size");
        return nullptr;
    }

    inline const IconInfo *getAllIcons()
    {
        ESP_LOGD(TAG, "Getting all icons array");
        return ICONS;
    }

    inline size_t getIconCount()
    {
        size_t count = sizeof(ICONS) / sizeof(ICONS[0]);
        ESP_LOGD(TAG, "Getting icon count: %d", count);
        return count;
    }

    inline int getSizePixels(Size size)
    {
        int pixels = static_cast<int>(size);
        ESP_LOGD(TAG, "Converting size enum to pixels: %d", pixels);
        return pixels;
    }

    // Keep compatibility with old code
    const int epd_bitmap_foxes_positive_array_LEN = 3;
    const unsigned char *epd_bitmap_foxes_positive_array[3] = {
        epd_bitmap_fox_32x32_1_p,
        epd_bitmap_fox_32x32_2_p,
        epd_bitmap_fox_64x64_1_p};

    const int epd_bitmap_foxes_negative_array_LEN = 3;
    const unsigned char *epd_bitmap_foxes_negative_array[3] = {
        epd_bitmap_fox_32x32_1_n,
        epd_bitmap_fox_32x32_2_n,
        epd_bitmap_fox_64x64_1_n};

    // Conversion utilities
    inline void convertToXBM(const unsigned char* input, unsigned char* output, 
                            u8g2_uint_t width, u8g2_uint_t height) 
    {
        ESP_LOGD(TAG, "Converting bitmap to XBM format - Size: %dx%d", width, height);
        
        size_t bytes = (width * height + 7) / 8;
        for (size_t i = 0; i < bytes; i++) 
        {
            // XBM format requires bits to be reversed within each byte
            unsigned char byte = input[i];
            unsigned char reversed = 0;
            
            // Reverse the bits
            for (int b = 0; b < 8; b++) 
            {
                reversed = (reversed << 1) | (byte & 1);
                byte >>= 1;
            }
            
            output[i] = reversed;
        }
        ESP_LOGD(TAG, "Bitmap conversion completed - Processed %d bytes", bytes);
    }

    // Get icon in XBM format
    inline const IconInfo getIconXBM(Type type, Polarity polarity = Polarity::POSITIVE) 
    {
        ESP_LOGD(TAG, "Getting icon in XBM format - Type: %d, Polarity: %d",
                 static_cast<int>(type), static_cast<int>(polarity));
        
        const IconInfo* originalIcon = getIcon(type, polarity);
        if (!originalIcon) {
            ESP_LOGE(TAG, "Failed to get original icon for XBM conversion");
            return IconInfo{nullptr, Size::SMALL, type, polarity, 0, 0};
        }

        // Calculate buffer size
        size_t bufferSize = (originalIcon->width * originalIcon->height + 7) / 8;
        
        // Allocate buffer for converted data
        unsigned char* convertedData = new unsigned char[bufferSize];
        if (!convertedData) {
            ESP_LOGE(TAG, "Failed to allocate memory for XBM conversion");
            return IconInfo{nullptr, Size::SMALL, type, polarity, 0, 0};
        }

        // Convert the bitmap
        convertToXBM(originalIcon->data, convertedData, 
                    originalIcon->width, originalIcon->height);
        
        ESP_LOGD(TAG, "XBM conversion successful - Buffer size: %d bytes", bufferSize);

        // Return new IconInfo with converted data
        return IconInfo{
            convertedData,
            originalIcon->size,
            originalIcon->type,
            originalIcon->polarity,
            originalIcon->width,
            originalIcon->height
        };
    }

    // Helper to clean up converted XBM data
    inline void freeXBMData(const IconInfo& iconInfo) 
    {
        ESP_LOGD(TAG, "Freeing XBM converted data");
        if (iconInfo.data != nullptr) {
            delete[] iconInfo.data;
            ESP_LOGD(TAG, "XBM data freed successfully");
        }
    }

    // Smart wrapper class for XBM icons
    class XBMIcon {
    private:
        IconInfo icon;

    public:
        // Constructor that immediately converts to XBM
        XBMIcon(Type type, Polarity polarity = Polarity::POSITIVE) {
            icon = getIconXBM(type, polarity);
        }

        // Destructor automatically frees memory
        ~XBMIcon() {
            if (icon.data) {
                freeXBMData(icon);
            }
        }

        // Prevent copying to avoid double-free
        XBMIcon(const XBMIcon&) = delete;
        XBMIcon& operator=(const XBMIcon&) = delete;

        // Allow checking if conversion was successful
        bool isValid() const { return icon.data != nullptr; }

        // Getters for the icon properties
        const unsigned char* data() const { return icon.data; }
        u8g2_uint_t width() const { return icon.width; }
        u8g2_uint_t height() const { return icon.height; }
    };
}

#endif