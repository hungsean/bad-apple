#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "utils.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// 灰階閾值：>= THRESHOLD 視為白 (1)，否則為黑 (0)
static const int THRESHOLD = 128;

// 將單張圖片轉成 packed 1-bit 資料
// 回傳 false 表示失敗
static bool convert_image(const std::string& path,
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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: converter <frames_dir> <output_file>\n");
        return 1;
    }

    const std::string frames_dir  = argv[1];
    const std::string output_file = argv[2];

    // 收集所有 PNG
    auto files = list_pngs(frames_dir);
    if (files.empty()) {
        fprintf(stderr, "ERROR: 在 %s 找不到任何 .png 檔\n", frames_dir.c_str());
        return 1;
    }
    printf("找到 %zu 張圖片，開始轉換...\n", files.size());

    // 讀第一張取得寬高
    int ref_w = 0, ref_h = 0, ch;
    {
        uint8_t* tmp = stbi_load(files[0].c_str(), &ref_w, &ref_h, &ch, 3);
        if (!tmp) {
            fprintf(stderr, "ERROR: 無法讀取第一張圖片 %s\n", files[0].c_str());
            return 1;
        }
        stbi_image_free(tmp);
    }
    printf("解析度: %d x %d\n", ref_w, ref_h);

    // 開啟輸出檔
    FILE* out = fopen(output_file.c_str(), "wb");
    if (!out) {
        fprintf(stderr, "ERROR: 無法開啟輸出檔 %s\n", output_file.c_str());
        return 1;
    }

    // 寫 header: width, height, frame_count (各 uint32_t, little-endian)
    uint32_t frame_count = (uint32_t)files.size();
    uint32_t w32 = (uint32_t)ref_w;
    uint32_t h32 = (uint32_t)ref_h;
    fwrite(&w32,          4, 1, out);
    fwrite(&h32,          4, 1, out);
    fwrite(&frame_count,  4, 1, out);

    // 依序轉換每張圖
    int ok_count = 0;
    for (size_t i = 0; i < files.size(); i++) {
        std::vector<uint8_t> bits;
        if (!convert_image(files[i], ref_w, ref_h, bits)) {
            fclose(out);
            return 1;
        }
        fwrite(bits.data(), 1, bits.size(), out);
        ok_count++;

        if ((i + 1) % 100 == 0 || i + 1 == files.size())
            printf("  已處理 %d / %zu\r", ok_count, files.size());
    }
    printf("\n");

    fclose(out);
    printf("完成！輸出至 %s (header 12 bytes + %d frames x %d bytes)\n",
           output_file.c_str(),
           ok_count,
           (ref_w * ref_h + 7) / 8);
    return 0;
}
