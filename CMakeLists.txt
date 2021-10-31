set(CMAKE_INCLUDE_CURRENT_DIR ON)
file(GLOB BINANCE_SRC ../binacpp/src/*)

add_library(binance ${BINANCE_SRC})
target_link_libraries(binance)