#ifndef MARKETSIMHPP
#define MARKETSIMHPP

#include "order.hpp"
#include "order_book.hpp"

#include <cstdint>
#include <random>
#include <vector>

struct SimConfig {
    double startingFairValue = 100.00; // normal decimal dollars

    // random walk parameters
    double drift = 0.25;
    double vol = 0.7;

    // order rates
    double limitOrderRate = 10.0;
    double marketOrderRate = 2;
    double cancelRate = 1.0;

    // order distance in cents
    double meanOffsetCents = 10.0;

    double aggroLimitProb = 0.01;

    Qty minOrderQty = 1;
    Qty maxOrderQty = 12;
};

class MarketSim {
    public:
        MarketSim(OrderBook &book, const SimConfig &config = SimConfig{});

        std::vector<Trade> step();

        double fairValue() const;
        std::uint64_t stepCount() const;

    private:
        void seedBook();

        std::vector<Trade> generateLimitOrders();
        std::vector<Trade> generateMarketOrders();
        void generateCancellations();

        Order createLimitOrder(OrderSide side, Price price, Qty qty);
        Order createMarketOrder(OrderSide size, Qty qty);

        OrderSide randomSide();
        Qty randomQty();

        Price decimalToPrice(double decimalPrice) const;
        double priceToDecimal(Price price) const;

        void rememberIfResting(const Order &order);
        void removeInactiveOrderIds();

        OrderBook &book;
        SimConfig config;

        std::mt19937 randomEngine;

        double currentFairValue;
        OrderId nextOrderId;
        std::uint64_t steps;

        std::vector<OrderId> restingOrderIds;
};

#endif