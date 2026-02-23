#pragma once

// 取得終端機大小
// debug=true 時多保留一行給 debug 資訊
void get_term_size(int& cols, int& rows, bool debug = false);
