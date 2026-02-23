#include "player_utils.h"

#include <sys/ioctl.h>
#include <unistd.h>

// 取得終端機大小
void get_term_size(int& cols, int& rows) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        cols = ws.ws_col;
        rows = ws.ws_row - 1; // 留一行給 cursor
    } else {
        cols = 120;
        rows = 45;
    }
}
