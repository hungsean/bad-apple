#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "player_utils.h"

static const int FPS = 30;

// 取出 packed bit 中第 idx 個 bit (MSB first)
static inline int get_bit(const uint8_t* bits, int idx) {
    return (bits[idx / 8] >> (7 - (idx % 8))) & 1;
}

// 將一幀 packed bits 縮放並渲染成字元串
// 使用半格字元：每個字元對應 1 列 x 2 行像素
//   ' ' = 00, '▀' = 10, '▄' = 01, '█' = 11
static void render_frame(const uint8_t* bits,
                          int src_w, int src_h,
                          int dst_cols, int dst_rows,
                          std::string& buf) {
    // dst_rows 個字元行，各代表 2 個像素行
    buf.clear();
    buf.reserve(dst_cols * dst_rows * 4 + dst_rows * 2);

    // 移到畫面左上角（不清空，直接覆寫，避免閃爍）
    buf += "\033[H";

    for (int cy = 0; cy < dst_rows; cy++) {
        // 上半像素行和下半像素行（來源座標）
        int py0 = (cy * 2)     * src_h / (dst_rows * 2);
        int py1 = (cy * 2 + 1) * src_h / (dst_rows * 2);

        for (int cx = 0; cx < dst_cols; cx++) {
            int px = cx * src_w / dst_cols;

            int top    = get_bit(bits, py0 * src_w + px);
            int bottom = get_bit(bits, py1 * src_w + px);

            // 0=黑, 1=白
            if      (!top && !bottom) buf += ' ';
            else if ( top && !bottom) buf += "\xe2\x96\x80"; // ▀ U+2580
            else if (!top &&  bottom) buf += "\xe2\x96\x84"; // ▄ U+2584
            else                      buf += "\xe2\x96\x88"; // █ U+2588
        }
        buf += '\n';
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: player <binary_file> [fps] [-d|--debug]\n");
        return 1;
    }

    bool debug = false;
    std::string binary_file;
    int fps = FPS;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug = true;
        } else if (binary_file.empty()) {
            binary_file = argv[i];
        } else {
            int v = atoi(argv[i]);
            if (v > 0) fps = v;
        }
    }

    if (binary_file.empty()) {
        fprintf(stderr, "Usage: player <binary_file> [fps] [-d|--debug]\n");
        return 1;
    }

    FILE* fp = fopen(binary_file.c_str(), "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: 無法開啟 %s\n", binary_file.c_str());
        return 1;
    }

    // 讀取 header
    uint32_t src_w, src_h, frame_count;
    if (fread(&src_w,       4, 1, fp) != 1 ||
        fread(&src_h,       4, 1, fp) != 1 ||
        fread(&frame_count, 4, 1, fp) != 1) {
        fprintf(stderr, "ERROR: header 讀取失敗\n");
        fclose(fp);
        return 1;
    }
    if (debug)
        fprintf(stderr, "解析度: %u x %u, 共 %u 幀, %d fps\n",
                src_w, src_h, frame_count, fps);

    int bytes_per_frame = ((int)(src_w * src_h) + 7) / 8;
    std::vector<uint8_t> frame_buf(bytes_per_frame);

    // 計算顯示大小（維持 4:3，半格字元讓高度 /2）
    int term_cols, term_rows;
    get_term_size(term_cols, term_rows, debug);

    // 寬度以 term_cols 為上限；字元高寬比約 2:1，故 pixel_h = char_rows * 2
    // src_w/src_h = dst_cols / (dst_rows * 2)  => dst_rows = dst_cols * src_h / (src_w * 2)
    int dst_cols = term_cols;
    int dst_rows = dst_cols * (int)src_h / ((int)src_w * 2);
    if (dst_rows > term_rows) {
        dst_rows = term_rows;
        dst_cols = dst_rows * 2 * (int)src_w / (int)src_h;
    }

    if (debug)
        fprintf(stderr, "顯示大小: %d cols x %d rows（字元）\n", dst_cols, dst_rows);

    // 清除畫面、隱藏 cursor
    printf("\033[2J\033[?25l");
    fflush(stdout);

    std::string render_buf;
    auto frame_duration = std::chrono::microseconds(1000000 / fps);
    auto play_start = std::chrono::steady_clock::now();
    auto prev_frame_start = play_start;

    for (uint32_t i = 0; i < frame_count; i++) {
        auto t0 = std::chrono::steady_clock::now();

        double interval_ms = std::chrono::duration<double, std::milli>(t0 - prev_frame_start).count();
        double actual_fps = (i > 0 && interval_ms > 0.0) ? 1000.0 / interval_ms : 0.0;

        if (fread(frame_buf.data(), 1, bytes_per_frame, fp) != (size_t)bytes_per_frame) {
            fprintf(stderr, "\n幀 %u 讀取失敗\n", i);
            break;
        }

        render_frame(frame_buf.data(), (int)src_w, (int)src_h,
                     dst_cols, dst_rows, render_buf);
        fwrite(render_buf.data(), 1, render_buf.size(), stdout);
        fflush(stdout);

        auto deadline = play_start + (i + 1) * frame_duration;
        if (debug) {
            fprintf(stderr, "\r\033[2K幀 %u/%u  幀時間 %.2fms  %.1f fps",
                    i + 1, frame_count, interval_ms, actual_fps);
        }
        prev_frame_start = t0;
        std::this_thread::sleep_until(deadline);
    }

    // 恢復 cursor
    printf("\033[?25h\n");
    fclose(fp);
    return 0;
}
