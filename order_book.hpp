#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP
#include "order.hpp"
#include <deque>
#include <functional>
#include <map>

class OrderBook{
    public:
        void addLimitOrder(const Order &order);

        bool empty(OrderSide side) const;
        std::size_t orderCount(OrderSide side) const;

        void print() const;

    private:
        using PriceLevel = std::deque<Order>;

        std::map<Price, PriceLevel, std::greater<Price>> bids;
        std::map<Price, PriceLevel> asks;
};

#endif