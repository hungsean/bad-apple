#include <cstdio>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: player <binary_file>\n");
        return 1;
    }

    const std::string binary_file = argv[1];

    // TODO: 讀取二元資料並顯示動畫

    printf("player: binary_file=%s\n", binary_file.c_str());

    return 0;
}
