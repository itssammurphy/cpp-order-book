#include "marketsim.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>

MarketSim::MarketSim(OrderBook &book, const SimConfig &config)
    : book(book),
      config(config),
      randomEngine(),
      currentFairValue(config.startingFairValue),
      nextOrderId(1),
      steps(0) 
{
    if (config.minOrderQty == 0) {
        throw std::invalid_argument("Minimum qty must be positive");
    }

    if (config.maxOrderQty < config.minOrderQty) {
        throw std::invalid_argument("Maximum order size must be >= minimum");

        if (config.meanOffsetCents <= 0.0) {
            throw std::invalid_argument("Mean limit-order offset must be > 0");
        }
    }
}

std::vector<Trade> MarketSim::step() {
    ++steps;

    if (steps == 1 && book.empty(OrderSide::Buy) && book.empty(OrderSide::Sell)) {
        seedBook();
    }

    std::normal_distribution<double> fairValueMove(config.drift, config.vol);

    currentFairValue += fairValueMove(randomEngine);

    std::vector<Trade> trades;

    generateCancellations();

    std::vector<Trade> limitTrades = generateLimitOrders();
    trades.insert(trades.end(), limitTrades.begin(), limitTrades.end());

    std::vector<Trade> marketTrades = generateMarketOrders();
    trades.insert(trades.end(), marketTrades.begin(), marketTrades.end());

    removeInactiveOrderIds();

    return trades;
}

void MarketSim::seedBook() {
    for (int level = 1; level <= 10; ++level)
    {
        const double dist = static_cast<double>(level) * 0.02;

        const Price bidPrice = decimalToPrice(currentFairValue - dist);

        const Price askPrice = decimalToPrice(currentFairValue + dist);

        Order bid = createLimitOrder(OrderSide::Buy, bidPrice, randomQty());

        Order ask = createLimitOrder(OrderSide::Sell, askPrice, randomQty());

        book.submitLimitOrder(bid);
        book.submitLimitOrder(ask);

        rememberIfResting(bid);
        rememberIfResting(ask);
    }
}

std::vector<Trade> MarketSim::generateLimitOrders() {
    std::vector<Trade> trades;

    std::poisson_distribution<int> orderCount(config.limitOrderRate);

    const int numberOfOrders = orderCount(randomEngine);

    const double lambda = 1.0 / config.meanOffsetCents;

    std::exponential_distribution<double> offsetDistro(lambda);

    std::bernoulli_distribution aggroDistro(config.aggroLimitProb);

    for (int i = 0; i < numberOfOrders; ++i) {
        const OrderSide side = randomSide();
        const double offsetCents = offsetDistro(randomEngine);
        const double offsetDollars = offsetCents / 100.0;

        const bool aggro = aggroDistro(randomEngine);

        double decimalPrice = currentFairValue;

        if (side == OrderSide::Buy) {
            decimalPrice += aggro ? offsetDollars : -offsetDollars;
        } else {
            decimalPrice += aggro ? -offsetDollars : offsetDollars;
        }

        decimalPrice = std::max(0.01, decimalPrice);

        Order order = createLimitOrder(
            side,
            decimalToPrice(decimalPrice),
            randomQty()
        );

        std::vector<Trade> newTrades = book.submitLimitOrder(order);

        trades.insert(trades.end(), newTrades.begin(), newTrades.end());

        rememberIfResting(order);
    }

    return trades;
}

std::vector<Trade> MarketSim::generateMarketOrders() {
    std::vector<Trade> trades;

    std::poisson_distribution<int> orderCount(config.marketOrderRate);

    const int numberOfOrders = orderCount(randomEngine);

    for (int i = 0; i < numberOfOrders; ++i) {
        const OrderSide side = randomSide();

        if (side == OrderSide::Buy && book.empty(OrderSide::Sell)) {
            continue;
        }

        if (side == OrderSide::Sell && book.empty(OrderSide::Buy)) {
            continue;
        }

        Order order = createMarketOrder(side, randomQty());

        std::vector<Trade> newTrades = book.executeMarketOrder(order);

        trades.insert(trades.end(), newTrades.begin(), newTrades.end());
    }

    return trades;
}

void MarketSim::generateCancellations() {
    std::poisson_distribution<int> cancellationCount(
        config.cancelRate);

    const int numberOfCancellations = cancellationCount(randomEngine);

    for (int i = 0; i < numberOfCancellations; ++i) {
        if (restingOrderIds.empty()) {
            return;
        }

        std::uniform_int_distribution<std::size_t> indexDistro(0, restingOrderIds.size() - 1);

        const std::size_t index = indexDistro(randomEngine);

        const OrderId orderId = restingOrderIds[index];

        book.cancelOrder(orderId);

        restingOrderIds[index] = restingOrderIds.back();
        restingOrderIds.pop_back();
    }
}

Order MarketSim::createLimitOrder(OrderSide side, Price price, Qty qty) {
    return Order{
        nextOrderId++,
        side,
        OrderType::Limit,
        qty,
        qty,
        price
    };
}

Order MarketSim::createMarketOrder(OrderSide side, Qty qty) {
    return Order{
        nextOrderId++,
        side,
        OrderType::Market,
        qty,
        qty,
        std::nullopt
    };
}

OrderSide MarketSim::randomSide() {
    std::bernoulli_distribution sideDistro(0.5);

    return sideDistro(randomEngine) ? OrderSide::Buy : OrderSide::Sell;
}

Qty MarketSim::randomQty() {
    std::uniform_int_distribution<Qty> qtyDistro(config.minOrderQty, config.maxOrderQty);

    return qtyDistro(randomEngine);
}

Price MarketSim::decimalToPrice(double decimalPrice) const {
    return static_cast<Price>(
        std::llround(decimalPrice * 100.0)
    );
}

void MarketSim::rememberIfResting(const Order& order) {
    if (order.remaining > 0 && book.contains(order.id)) {
        restingOrderIds.push_back(order.id);
    }
}

void MarketSim::removeInactiveOrderIds() {
    restingOrderIds.erase(
        std::remove_if(
            restingOrderIds.begin(),
            restingOrderIds.end(),
            [this](OrderId orderId)
            {
                return !book.contains(orderId);
            }
        ),
        restingOrderIds.end()
    );
}

double MarketSim::fairValue() const {
    return currentFairValue;
}

std::uint64_t MarketSim::stepCount() const {
    return steps;
}