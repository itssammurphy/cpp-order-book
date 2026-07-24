set -e

mkdir -p build

g++ \
    -std=c++20 \
    -Wall \
    -Wextra \
    -pedantic \
    main.cpp \
    order_book.cpp \
    marketsim.cpp \
    csvlogger.cpp \
    -o build/orderbook

echo "Build finished: ./build/orderbook"