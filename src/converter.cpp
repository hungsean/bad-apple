#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: converter <frames_dir> <output_file>\n");
        return 1;
    }

    const std::string frames_dir = argv[1];
    const std::string output_file = argv[2];

    // TODO: 讀取所有 frame，轉成 1-bit 二元資料並寫入 output_file

    printf("converter: frames_dir=%s, output=%s\n",
           frames_dir.c_str(), output_file.c_str());

    return 0;
}
