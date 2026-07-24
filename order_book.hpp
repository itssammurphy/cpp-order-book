#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP
#include "order.hpp"
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <vector>
#include <list>
#include <unordered_map>

class OrderBook{
    public:
        bool empty(OrderSide side) const;
        std::size_t orderCount(OrderSide side) const;

        std::optional<Price> bestBid() const;
        std::optional<Price> bestAsk() const;

        void print() const;

        std::vector<Trade> submitLimitOrder(Order& order);
        std::vector<Trade> executeMarketOrder(Order& order);

        bool cancelOrder(OrderId orderId);

    private:
        using PriceLevel = std::list<Order>;

        std::map<Price, PriceLevel, std::greater<Price>> bids;
        std::map<Price, PriceLevel> asks;

        std::vector<Trade> matchBuyOrder(Order &order);
        std::vector<Trade> matchSellOrder(Order &order);

        void addRestingOrder(const Order& order);

        bool canMatch(const Order& order, Price oppositePrice) const;

        struct OrderLocation {
            OrderSide side;
            Price price;
            PriceLevel::iterator order;
        };

        bool activeOrderIdExists(OrderId orderId) const;

        std::unordered_map<OrderId, OrderLocation> orderIndex;
};

#endif