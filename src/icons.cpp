#include "xbm_icon.h"
#include <pgmspace.h>
#include "esp_log.h"

namespace Icons
{
    static const char *TAG = "Icons";
    static constexpr int BITMAP_ARRAY_LEN = 3;
    // 32x32 fox bitmaps (positive)
    static const unsigned char epd_bitmap_fox_32x32_1_p[] PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x20, 0x10, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x14, 0x44, 0x00, 0x01, 0x13, 0x70, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x04,
        0x00, 0x04, 0xa0, 0x04, 0x00, 0x02, 0x80, 0x02, 0x00, 0x00, 0x48, 0x02, 0x00, 0x01, 0x04, 0x0a,
        0x00, 0x00, 0x83, 0x10, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x60, 0xba,
        0x00, 0x0d, 0x08, 0x08, 0x00, 0x10, 0x06, 0xe0, 0x00, 0x22, 0x00, 0x20, 0x00, 0x44, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x40, 0x00, 0x84, 0xa0, 0x40, 0x03, 0x04, 0x40, 0x80, 0x08, 0x04, 0xa5, 0x40,
        0xe8, 0x0c, 0xa6, 0x00, 0x80, 0x05, 0x08, 0x80, 0x40, 0x14, 0x2a, 0x80, 0x18, 0x22, 0x08, 0x80,
        0x07, 0x84, 0x12, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};

    static const unsigned char epd_bitmap_fox_32x32_2_p[] PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x50, 0x14, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x94, 0x50, 0x00, 0x02, 0x13, 0xf4, 0x00, 0x03, 0x14, 0x0c, 0x00, 0x04, 0x10, 0x94,
        0x00, 0x02, 0xa5, 0x04, 0x00, 0x02, 0xd0, 0x52, 0x00, 0x01, 0x15, 0x0a, 0x00, 0x01, 0x4c, 0x54,
        0x00, 0x00, 0x43, 0x92, 0x00, 0x00, 0x50, 0x0a, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x61, 0x75,
        0x00, 0x0d, 0x98, 0x08, 0x00, 0x12, 0x06, 0xe0, 0x00, 0x2a, 0xa0, 0x20, 0x00, 0x54, 0x10, 0x20,
        0x00, 0x4d, 0x48, 0x80, 0x00, 0xa4, 0xa0, 0x40, 0x03, 0x55, 0x68, 0xc0, 0x0a, 0x14, 0xa5, 0x00,
        0xe8, 0xad, 0x2e, 0x80, 0x80, 0xaa, 0x22, 0x80, 0x40, 0x55, 0x1a, 0x80, 0x19, 0x24, 0x11, 0x00,
        0x06, 0x85, 0x19, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};

    static const unsigned char epd_bitmap_fox_64x64_1_p[] PROGMEM = {
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

    // 32x32 fox bitmaps (negative)
    static const unsigned char epd_bitmap_fox_32x32_1_n[] PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x20, 0x10, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x14, 0x44, 0x00, 0x01, 0x13, 0x70, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x04,
        0x00, 0x04, 0xa0, 0x04, 0x00, 0x02, 0x80, 0x02, 0x00, 0x00, 0x48, 0x02, 0x00, 0x01, 0x04, 0x0a,
        0x00, 0x00, 0x83, 0x10, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x60, 0xba,
        0x00, 0x0d, 0x08, 0x08, 0x00, 0x10, 0x06, 0xe0, 0x00, 0x22, 0x00, 0x20, 0x00, 0x44, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x40, 0x00, 0x84, 0xa0, 0x40, 0x03, 0x04, 0x40, 0x80, 0x08, 0x04, 0xa5, 0x40,
        0xe8, 0x0c, 0xa6, 0x00, 0x80, 0x05, 0x08, 0x80, 0x40, 0x14, 0x2a, 0x80, 0x18, 0x22, 0x08, 0x80,
        0x07, 0x84, 0x12, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};

    static const unsigned char epd_bitmap_fox_32x32_2_n[] PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x01, 0x50, 0x14, 0x00, 0x02, 0xa8, 0x2c,
        0x00, 0x02, 0x94, 0x50, 0x00, 0x02, 0x13, 0xf4, 0x00, 0x03, 0x14, 0x0c, 0x00, 0x04, 0x10, 0x94,
        0x00, 0x02, 0xa5, 0x04, 0x00, 0x02, 0xd0, 0x52, 0x00, 0x01, 0x15, 0x0a, 0x00, 0x01, 0x4c, 0x54,
        0x00, 0x00, 0x43, 0x92, 0x00, 0x00, 0x50, 0x0a, 0x00, 0x00, 0x40, 0x31, 0x00, 0x03, 0x61, 0x75,
        0x00, 0x0d, 0x98, 0x08, 0x00, 0x12, 0x06, 0xe0, 0x00, 0x2a, 0xa0, 0x20, 0x00, 0x54, 0x10, 0x20,
        0x00, 0x4d, 0x48, 0x80, 0x00, 0xa4, 0xa0, 0x40, 0x03, 0x55, 0x68, 0xc0, 0x0a, 0x14, 0xa5, 0x00,
        0xe8, 0xad, 0x2e, 0x80, 0x80, 0xaa, 0x22, 0x80, 0x40, 0x55, 0x1a, 0x80, 0x19, 0x24, 0x11, 0x00,
        0x06, 0x85, 0x19, 0x80, 0x00, 0x02, 0x11, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};

    static const unsigned char epd_bitmap_fox_64x64_1_n[] PROGMEM = {
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

    // Make the arrays static and ensure they're properly initialized
    alignas(4) const unsigned char *const epd_bitmap_foxes_positive_array[BITMAP_ARRAY_LEN] = {
        epd_bitmap_fox_32x32_1_p,
        epd_bitmap_fox_32x32_2_p,
        epd_bitmap_fox_64x64_1_p};

    alignas(4) const unsigned char *const epd_bitmap_foxes_negative_array[BITMAP_ARRAY_LEN] = {
        epd_bitmap_fox_32x32_1_n,
        epd_bitmap_fox_32x32_2_n,
        epd_bitmap_fox_64x64_1_n};

    // Public accessors for the array lengths
    const int epd_bitmap_foxes_positive_array_LEN = BITMAP_ARRAY_LEN;
    const int epd_bitmap_foxes_negative_array_LEN = BITMAP_ARRAY_LEN;

    // Utility function to reverse bits in a byte
    unsigned char reverseBits(unsigned char b)
    {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return b;
    }

    IconInfo getIconXBM(Type type, Polarity polarity)
    {
        ESP_LOGD(TAG, "Getting icon XBM - Type: %d, Polarity: %d",
                 static_cast<int>(type), static_cast<int>(polarity));

        // Initialize with safe defaults
        IconInfo info = {
            nullptr,     // data
            Size::SMALL, // size
            type,        // type
            polarity,    // polarity
            32,          // width
            32           // height
        };

        try
        {
            // Validate type
            int index = static_cast<int>(type);
            if (index < 0 || index >= BITMAP_ARRAY_LEN)
            {
                ESP_LOGE(TAG, "Invalid icon type index: %d", index);
                return info;
            }

            // Get source data
            const unsigned char *sourceData = nullptr;
            if (polarity == Polarity::POSITIVE && epd_bitmap_foxes_positive_array != nullptr)
            {
                sourceData = epd_bitmap_foxes_positive_array[index];
            }
            else if (epd_bitmap_foxes_negative_array != nullptr)
            {
                sourceData = epd_bitmap_foxes_negative_array[index];
            }

            if (!sourceData)
            {
                ESP_LOGE(TAG, "Source data is null");
                return info;
            }

            // Set dimensions based on type
            if (type == Type::FOX_3)
            {
                info.size = Size::LARGE;
                info.width = info.height = static_cast<u8g2_uint_t>(Size::LARGE);
            }

            // Calculate buffer size and dimensions
            size_t bufferSize = (info.width * info.height + 7) / 8;
            size_t bytesPerRow = (info.width + 7) / 8;

            // Allocate memory
            unsigned char *convertedData = new unsigned char[bufferSize];
            if (!convertedData)
            {
                ESP_LOGE(TAG, "Memory allocation failed");
                return info;
            }

            // Convert to XBMP format (vertical-first, LSB)
            for (size_t y = 0; y < info.height; y++)
            {
                for (size_t x = 0; x < bytesPerRow; x++)
                {
                    size_t srcIndex = y * bytesPerRow + x;
                    if (srcIndex < bufferSize)
                    {
                        unsigned char byte = pgm_read_byte(&sourceData[srcIndex]);
                        // Reverse bits for XBMP format
                        unsigned char reversed = 0;
                        for (int b = 0; b < 8; b++)
                        {
                            reversed |= ((byte >> (7 - b)) & 0x01) << b;
                        }
                        convertedData[srcIndex] = polarity == Polarity::POSITIVE ? reversed : ~reversed;
                    }
                }
            }

            ESP_LOGD(TAG, "First 8 bytes of converted data: ");
            for (int i = 0; i < 8 && i < bufferSize; i++)
            {
                ESP_LOGD(TAG, "Byte %d: 0x%02X", i, convertedData[i]);
            }

            info.data = convertedData;
            ESP_LOGD(TAG, "XBM conversion successful");
        }
        catch (const std::exception &e)
        {
            ESP_LOGE(TAG, "Exception in getIconXBM: %s", e.what());
            if (info.data)
            {
                delete[] info.data;
                info.data = nullptr;
            }
        }

        return info;
    }

    void freeXBMData(const IconInfo &icon)
    {
        ESP_LOGD(TAG, "Freeing XBM data");
        if (icon.data)
        {
            delete[] icon.data;
            ESP_LOGD(TAG, "XBM data freed successfully");
        }
    }
} // namespace Icons