#pragma once

#include <cstdint>
#include <string>
#include <vector>

// 將單張圖片轉成 packed 1-bit 資料
// 回傳 false 表示失敗
bool convert_image(const std::string& path,
                   int expected_w, int expected_h,
                   std::vector<uint8_t>& out_bits);
