#include <iostream>
#include "order.hpp"
#include "order_book.hpp"
#include <iomanip>
#include <optional>

void printPrice(const std::optional<Price>& price) {
    if (!price.has_value()) {
        std::cout << "none";
        return;
    }

    std::cout << std::fixed << std::setprecision(2) << static_cast<double>(price.value()) / 100.0;
}

int main() {

    OrderBook book;

    Order buyOne{
        1,
        OrderSide::Buy,
        OrderType::Limit,
        10,
        10,
        10000 // $100.00
    };

    Order buyTwo{
        2,
        OrderSide::Buy,
        OrderType::Limit,
        5,
        5,
        9995 // $99.95
    };

    Order sellOne{
        3,
        OrderSide::Sell,
        OrderType::Limit,
        8,
        8,
        10005
    };

    Order sellTwo{
        4,
        OrderSide::Sell,
        OrderType::Limit,
        10,
        10,
        10010
    };

    book.addLimitOrder(buyOne);
    book.addLimitOrder(buyTwo);
    book.addLimitOrder(sellOne);
    book.addLimitOrder(sellTwo);

    book.print();

    std::cout << "Best bid: ";
    printPrice(book.bestBid());
    std::cout << "\n";
    std::cout << "Best ask: ";
    printPrice(book.bestAsk());
    std::cout << "\n";

    return 0;
}