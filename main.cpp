#include "marketsim.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>
#include "csvlogger.hpp"

void clearScreen() {
    std::cout << "\x1B[2J\x1B[H";
}

void printTrades(const std::vector<Trade>& trades) {
    if (trades.empty())
    {
        std::cout << "No trades this step\n";
        return;
    }

    std::cout << "TRADES\n";
    std::cout << "----------------------------------------\n";

    for (const Trade& trade : trades) {
        const char *aggressor = trade.aggressor == OrderSide::Buy ? "BUY" : "SELL";

        std::cout << "Trade "
                  << std::fixed
                  << std::setprecision(2)
                  << static_cast<double>(trade.price) / 100.0
                  << " x "
                  << trade.qty
                  << " | aggressor: "
                  << aggressor << "\n";
    }
}

int main() {
    std::filesystem::create_directories("output");

    OrderBook book;

    SimConfig config;
    config.limitOrderRate = 5.0;
    config.marketOrderRate = 1.0;
    config.cancelRate = 2.0;

    MarketSim simulator(book, config);

    CsvLogger logger(
        "output/booksnapshots.csv",
        "output/trades.csv");

    constexpr int numberOfSteps = 10;

    for (int i = 0; i < numberOfSteps; ++i) {
        std::vector<Trade> trades = simulator.step();

        logger.logSnapshot(
            simulator.stepCount(),
            simulator.fairValue(),
            book
        );

        logger.logTrades(
            simulator.stepCount(),
            trades
        );

        // clearScreen();

        // std::cout << "\nSIM STEP "
        //           << simulator.stepCount() << "\n";

        // std::cout << "Fair value: "
        //           << std::fixed << std::setprecision(2)
        //           << simulator.fairValue() << "\n";

        // printTrades(trades);
        // book.print();

        // std::cout.flush();

        // std::this_thread::sleep_for(
        //     std::chrono::milliseconds(100));
    }

    return 0;
}