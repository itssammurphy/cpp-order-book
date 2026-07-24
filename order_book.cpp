#include "order_book.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>

void OrderBook::addLimitOrder(const Order& order) {
    if (order.type != OrderType::Limit) {
        throw std::invalid_argument(
            "Only limit orders can rest in the order book");
    }

    if (!order.price.has_value()) {
        throw std::invalid_argument("A limit order must have a price");
    }

    if (order.qty == 0) {
        // qty of 0 possible with uint64_t so handle it
        throw std::invalid_argument("Order qty must be > 0");
    }

    if (order.remaining == 0 || order.remaining > order.qty) {
        throw std::invalid_argument(
            "Order qty is invalid");
    }

    const Price orderPrice = order.price.value();

    if (order.side == OrderSide::Buy) {
        bids[orderPrice].push_back(order);
    } else {
        asks[orderPrice].push_back(order);
    }
}

bool OrderBook::empty(OrderSide side) const {
    if (side == OrderSide::Buy) {
        return bids.empty();
    }

    return asks.empty();
}

std::size_t OrderBook::orderCount(OrderSide side) const {
    std::size_t count = 0;

    if (side == OrderSide::Buy) {
        for (const auto&[price, orders] : bids) {
            count += orders.size();
        }
    } else {
        for (const auto&[price, orders] : asks) {
            count += orders.size();
        }
    }

    return count;
}

void OrderBook::print() const {
    std::cout << "\nORDER BOOK\n";
    std::cout << "--------------------------\n";
    std::cout << "ASKS\n";

    for (auto level = asks.rbegin(); level != asks.rend(); ++level) {
        const Price price = level->first;
        const PriceLevel &orders = level->second;

        Qty totalQty = 0;

        for (const Order& order : orders) {
            totalQty += order.remaining;
        }

        std::cout << std::fixed << std::setprecision(2) << std::setw(8)
                  << static_cast<double>(price) / 100.0
                  << std::setw(8) << totalQty << "\n";
    }

    std::cout <<  "--------------------------\n";

    std::cout << "BIDS\n";

    for (const auto& [price, orders] : bids) {
        Qty totalQty = 0;

        for (const Order& order : orders) {
            totalQty += order.remaining;
        }

        std::cout << std::fixed << std::setprecision(2) << std::setw(8)
                  << static_cast<double>(price) / 100.0
                  << std::setw(8) << totalQty << "\n";
    }

    std::cout << "\n";
}

std::optional<Price> OrderBook::bestBid() const {
    if (bids.empty()) {
        return std::nullopt;
    }

    return bids.begin()->first;
}

std::optional<Price> OrderBook::bestAsk() const {
    if (asks.empty()) {
        return std::nullopt;
    }

    return asks.begin()->first;
}

std::vector<Trade> OrderBook::executeMarketOrder(Order& order) {
    if (order.type != OrderType::Market) {
        throw std::invalid_argument("executeMarketOrder needs a market order to execute");
    }

    if (order.qty == 0) {
        // qty of 0 possible with uint64_t so handle it
        throw std::invalid_argument("Order qty must be > 0");
    }

    if (order.remaining == 0 || order.remaining > order.qty) {
        throw std::invalid_argument(
            "Order qty is invalid");
    }

    std::vector<Trade> trades;

    if (order.side == OrderSide::Buy) {
        if (asks.empty()) {
            return trades;
        }

        auto bestLevel = asks.begin();
        Price tradePrice = bestLevel->first;
        PriceLevel &orders = bestLevel->second;

        while (order.remaining > 0 && !orders.empty()) {
            Order &maker = orders.front();

            Qty fillQty = std::min(
                order.remaining,
                maker.remaining
            );

            Trade trade{
                tradePrice,
                fillQty,
                OrderSide::Buy,
                maker.id,
                order.id
            };

            trades.push_back(trade);

            order.remaining -= fillQty;
            maker.remaining -= fillQty;

            if (maker.remaining == 0) {
                orders.pop_front();
            }
        }

        if (orders.empty()) {
            asks.erase(bestLevel);
        }
    } else {
        if (bids.empty()) {
            return trades;
        }

        auto bestLevel = bids.begin();
        Price tradePrice = bestLevel->first;
        PriceLevel &orders = bestLevel->second;

        while (order.remaining > 0 && !orders.empty()) {
            Order &maker = orders.front();

            Qty fillQty = std::min(
                order.remaining,
                maker.remaining
            );

            Trade trade{
                tradePrice,
                fillQty,
                OrderSide::Sell,
                maker.id,
                order.id
            };

            trades.push_back(trade);

            order.remaining -= fillQty;
            maker.remaining -= fillQty;

            if (maker.remaining == 0) {
                orders.pop_front();
            }
        }

        if (orders.empty()) {
            bids.erase(bestLevel);
        }
    }

    return trades;
}