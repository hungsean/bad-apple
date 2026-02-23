#include "utils.h"

#include <cstdio>
#include <string>
#include <vector>

// 列出目錄下所有 .png 檔，排序後回傳
std::vector<std::string> list_pngs(const std::string& dir) {
    std::vector<std::string> result;

    // 用 popen 呼叫 shell 列出檔案（簡單可攜做法）
    std::string cmd = "ls \"" + dir + "\"/*.png 2>/dev/null | sort";
    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) return result;

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        std::string s(buf);
        // 去掉尾端換行
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
            s.pop_back();
        if (!s.empty())
            result.push_back(s);
    }
    pclose(fp);
    return result;
}
