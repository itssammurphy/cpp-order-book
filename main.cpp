#include <iostream>
#include "order.hpp"
#include "order_book.hpp"
#include <iomanip>
#include <optional>
#include <vector>

void printPrice(const std::optional<Price>& price) {
    if (!price.has_value()) {
        std::cout << "none";
        return;
    }

    std::cout << std::fixed << std::setprecision(2) << static_cast<double>(price.value()) / 100.0;
}

void printTrades(const std::vector<Trade>& trades) {
    if (trades.empty()) {
        std::cout << "no trades\n";
        return;
    }

    std::cout << "TRADES\n";
    std::cout << "--------------\n";

    for (const Trade& trade : trades) {
        std::cout << "Price: "
                  << std::fixed << std::setprecision(2)
                  << static_cast<double>(trade.price) / 100.0
                  << ", Qty: "
                  << trade.qty
                  << ", Maker: "
                  << trade.makerId
                  << ", Taker: "
                  << trade.takerId
                  << "\n";
    }
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

    book.submitLimitOrder(buyOne);
    book.submitLimitOrder(buyTwo);
    book.submitLimitOrder(sellOne);
    book.submitLimitOrder(sellTwo);

    std::cout << "Before market order:\n";
    book.print();

    Order aggroBuy{
        5,
        OrderSide::Buy,
        OrderType::Limit,
        20,
        20,
        9900};

    std::vector<Trade> trades = book.submitLimitOrder(aggroBuy);

    printTrades(trades);

    std::cout << "\nMarket order remaining: "
              << aggroBuy.remaining
              << "\n";

    std::cout << "After market order:\n";
    book.print();

    std::cout << "Best bid: ";
    printPrice(book.bestBid());
    std::cout << "\n";
    std::cout << "Best ask: ";
    printPrice(book.bestAsk());
    std::cout << "\n";

    return 0;
}