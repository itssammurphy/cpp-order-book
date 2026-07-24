#ifndef CSVLOGGERHPP
#define CSVLOGGERHPP

#include "order.hpp"
#include "order_book.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class CsvLogger {
    public:
        CsvLogger(
            const std::string &snapshotFile,
            const std::string &tradeFile);

        void logSnapshot(
            std::uint64_t step,
            double fairValue,
            const OrderBook &book);

        void logTrades(
            std::uint64_t step,
            const std::vector<Trade> &trades);

    private:
        static double toDecimalPrice(Price price);
        static const char *sideToString(OrderSide side);

        std::ofstream snapshotOutput;
        std::ofstream tradeOutput;
};

#endif