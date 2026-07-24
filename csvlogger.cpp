#include "csvlogger.hpp"

#include <iomanip>
#include <stdexcept>

CsvLogger::CsvLogger(const std::string& snapshotFile, const std::string& tradeFile): snapshotOutput(snapshotFile), tradeOutput(tradeFile) {
    if (!snapshotOutput.is_open()) {
        throw std::runtime_error("Could not open snapshot output file.");
    }

    if (!tradeOutput.is_open()) {
        throw std::runtime_error("Could not open trade output file");
    }

    snapshotOutput << "step,"
                   << "fair_value,"
                   << "best_bid,"
                   << "best_ask,"
                   << "spread,"
                   << "bid_volume,"
                   << "ask_volume\n";

    tradeOutput << "step,"
                << "price,"
                << "qty,"
                << "aggressor,"
                << "maker_id,"
                << "taker_id\n";
}

void CsvLogger::logSnapshot(
    std::uint64_t step,
    double fairValue,
    const OrderBook& book
) {
    const std::optional<Price> bid = book.bestBid();
    const std::optional<Price> ask = book.bestAsk();

    snapshotOutput << step
                   << ","
                   << std::fixed
                   << std::setprecision(4)
                   << fairValue
                   << ",";

    if (bid.has_value()) {
        snapshotOutput << std::setprecision(2)
                       << toDecimalPrice(bid.value());
    }

    snapshotOutput << ",";

    if (ask.has_value())
    {
        snapshotOutput
            << std::setprecision(2)
            << toDecimalPrice(ask.value());
    }

    snapshotOutput << ',';

    if (bid.has_value() && ask.has_value())
    {
        const double spread =
            toDecimalPrice(ask.value() - bid.value());

        snapshotOutput
            << std::setprecision(2)
            << spread;
    }

    snapshotOutput
        << ','
        << book.totalVolume(OrderSide::Buy)
        << ','
        << book.totalVolume(OrderSide::Sell)
        << '\n';

    snapshotOutput.flush();
}

void CsvLogger::logTrades(std::uint64_t step, const std::vector<Trade>& trades) {
    for (const Trade& trade : trades) {
        tradeOutput << step
                    << ","
                    << std::fixed
                    << std::setprecision(2)
                    << toDecimalPrice(trade.price)
                    << ","
                    << trade.qty
                    << ","
                    << sideToString(trade.aggressor)
                    << ","
                    << trade.makerId
                    << ","
                    << trade.takerId
                    << "\n";
    }

    tradeOutput.flush();
}

double CsvLogger::toDecimalPrice(Price price) {
    return static_cast<double>(price) / 100.0;
}

const char* CsvLogger::sideToString(OrderSide side) {
    return side == OrderSide::Buy ? "BUY" : "SELL";
}