#include "convert_image.h"
#include "stb_image.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

// 灰階閾值：>= THRESHOLD 視為白 (1)，否則為黑 (0)
static const int THRESHOLD = 128;

bool convert_image(const std::string& path,
                   int expected_w, int expected_h,
                   std::vector<uint8_t>& out_bits) {
    int w, h, channels;
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &channels, 3); // 強制 RGB
    if (!data) {
        fprintf(stderr, "ERROR: 無法讀取圖片 %s: %s\n", path.c_str(), stbi_failure_reason());
        return false;
    }

    if (expected_w != 0 && (w != expected_w || h != expected_h)) {
        fprintf(stderr, "ERROR: 圖片尺寸不符 %s (期望 %dx%d，實際 %dx%d)\n",
                path.c_str(), expected_w, expected_h, w, h);
        stbi_image_free(data);
        return false;
    }

    // 每張 frame 的 bit 數 = w * h，packed 成 bytes (ceil)
    int total_pixels = w * h;
    int total_bytes  = (total_pixels + 7) / 8;
    out_bits.assign(total_bytes, 0);

    for (int i = 0; i < total_pixels; i++) {
        uint8_t r = data[i * 3 + 0];
        uint8_t g = data[i * 3 + 1];
        uint8_t b = data[i * 3 + 2];
        // BT.601 灰階
        int gray = (r * 77 + g * 150 + b * 29) >> 8;
        if (gray >= THRESHOLD) {
            out_bits[i / 8] |= (1u << (7 - (i % 8))); // MSB first
        }
    }

    stbi_image_free(data);
    return true;
}
