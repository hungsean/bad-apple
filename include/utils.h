#pragma once

#include <string>
#include <vector>

// 列出目錄下所有 .png 檔，排序後回傳
std::vector<std::string> list_pngs(const std::string& dir);
