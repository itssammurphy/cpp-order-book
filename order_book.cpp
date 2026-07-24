#include "order_book.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>

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