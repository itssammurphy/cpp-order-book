#include "order_book.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>

std::vector<Trade> OrderBook::submitLimitOrder(Order& order) {
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

    std::vector<Trade> trades;

    if (order.side == OrderSide::Buy) {
        trades = matchBuyOrder(order);
    } else {
        trades = matchSellOrder(order);
    }

    if (order.remaining > 0) {
        addRestingOrder(order);
    }

    return trades;
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
        for (const auto& level : bids) {
            count += level.second.size();
        }
    } else {
        for (const auto& level : asks) {
            count += level.second.size();
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

    if (activeOrderIdExists(order.id)) {
        throw std::invalid_argument("Order is already active");
    }

    if (order.side == OrderSide::Buy) {
        return matchBuyOrder(order);
    } 

    return matchSellOrder(order);
}

std::vector<Trade> OrderBook::matchBuyOrder(Order& order) {
    std::vector<Trade> trades;

    while (order.remaining > 0 && !asks.empty()) {
        auto bestLevel = asks.begin();

        const Price tradePrice = bestLevel->first;

        if (!canMatch(order, tradePrice)) {
            break;
        }

        PriceLevel &restingOrders = bestLevel->second;

        while (order.remaining > 0 && !restingOrders.empty()) {
            Order &maker = restingOrders.front();

            const Qty fillQty = std::min(
                order.remaining,
                maker.remaining
            );

            trades.push_back(
                Trade{
                    tradePrice,
                    fillQty,
                    OrderSide::Buy,
                    maker.id,
                    order.id
                }
            );

            order.remaining -= fillQty;
            maker.remaining -= fillQty;

            if (maker.remaining == 0) {
                orderIndex.erase(maker.id);
                restingOrders.pop_front();
            }
        }

        if (restingOrders.empty()) {
            asks.erase(bestLevel);
        }
    }

    return trades;
}

std::vector<Trade> OrderBook::matchSellOrder(Order& order) {
    std::vector<Trade> trades;

    while (order.remaining > 0 && !bids.empty()) {
        auto bestLevel = bids.begin();

        const Price tradePrice = bestLevel->first;

        if (!canMatch(order, tradePrice)) {
            break;
        }

        PriceLevel& restingOrders = bestLevel->second;

        while (order.remaining > 0 && !restingOrders.empty()) {
            Order &maker = restingOrders.front();

            const Qty fillQty = std::min(
                order.remaining,
                maker.remaining);

            trades.push_back(
                Trade{
                    tradePrice,
                    fillQty,
                    OrderSide::Sell,
                    maker.id,
                    order.id
                }
            );

            order.remaining -= fillQty;
            maker.remaining -= fillQty;

            if (maker.remaining == 0) {
                orderIndex.erase(maker.id);
                restingOrders.pop_front();
            }
        }

        if (restingOrders.empty()) {
            bids.erase(bestLevel);
        }
    }

    return trades;
}

void OrderBook::addRestingOrder(const Order& order) {
    if (!order.price.has_value()) {
        throw std::invalid_argument("A resting order must have a price");
    }

    const Price orderPrice = order.price.value();

    if (order.side == OrderSide::Buy) {
        PriceLevel &level = bids[orderPrice];

        level.push_back(order);

        auto orderIterator = std::prev(level.end());

        orderIndex.emplace(
            order.id,
            OrderLocation{
                OrderSide::Buy,
                orderPrice,
                orderIterator
            }
        );
    }
    else
    {
        PriceLevel &level = asks[orderPrice];

        level.push_back(order);

        auto orderIterator = std::prev(level.end());

        orderIndex.emplace(
            order.id,
            OrderLocation{
                OrderSide::Sell,
                orderPrice,
                orderIterator
            }
        );
    }
}

bool OrderBook::canMatch(const Order& order, Price oppositePrice) const {
    if (order.type == OrderType::Market) {
        return true;
    }

    if (!order.price.has_value()) {
        return false;
    }

    const Price limitPrice = order.price.value();

    if (order.side == OrderSide::Buy) {
        return oppositePrice <= limitPrice;
    }

    return oppositePrice >= limitPrice;
}

bool OrderBook::cancelOrder(OrderId orderId) {
    auto locationIterator = orderIndex.find(orderId);

    if (locationIterator == orderIndex.end()) {
        return false;
    }

    const OrderLocation loc = locationIterator->second;

    if (loc.side == OrderSide::Buy) {
        auto levelIterator = bids.find(loc.price);

        if (levelIterator == bids.end()) {
            throw std::logic_error("Order index refers to missing bid level");
        }

        PriceLevel &level = levelIterator->second;

        level.erase(loc.order);

        if (level.empty()) {
            bids.erase(levelIterator);
        }
    } else {
        auto levelIterator = asks.find(loc.price);

        if (levelIterator == asks.end()) {
            throw std::logic_error("Order index refers to a missing ask level");
        }

        PriceLevel &level = levelIterator->second;

        level.erase(loc.order);

        if (level.empty()) {
            asks.erase(levelIterator);
        }
    }

    orderIndex.erase(locationIterator);

    return true;
}

bool OrderBook::activeOrderIdExists(OrderId orderId) const {
    return orderIndex.find(orderId) != orderIndex.end();
}

bool OrderBook::contains(OrderId orderId) const {
    return orderIndex.find(orderId) != orderIndex.end();
}

Qty OrderBook::totalVolume(OrderSide side) const {
    Qty total = 0;

    if (side == OrderSide::Buy) {
        for (const auto& level : bids) {
            for (const Order& order : level.second) {
                total += order.remaining;
            }
        }
    } else {
        for (const auto& level : asks) {
            for (const Order& order : level.second) {
                total += order.remaining;
            }
        }
    }

    return total;
}