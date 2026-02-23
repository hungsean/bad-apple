cmake_minimum_required(VERSION 3.15)
project(bad_apple CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# converter: 圖片 → 二元資料
add_executable(converter
    src/converter.cpp
    src/utils/utils.cpp
    src/utils/convert_image.cpp
)

# player: 二元資料 → 動畫顯示
add_executable(player
    src/player.cpp
    src/utils/player_utils.cpp
)
