#include "player_utils.h"

#include <sys/ioctl.h>
#include <unistd.h>

// 取得終端機大小
void get_term_size(int& cols, int& rows, bool debug) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        cols = ws.ws_col;
        rows = ws.ws_row - 1 - (debug ? 1 : 0); // 留一行給 cursor，debug 時再多留一行
    } else {
        cols = 120;
        rows = 45;
    }
}
