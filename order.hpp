#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>
#include <optional>

enum class OrderSide
{
    Buy,
    Sell
};

enum class OrderType
{
    Limit,
    Market
};

using OrderId = std::uint64_t;
using Qty = std::uint64_t;
using Price = std::int64_t; // cents

struct Order {
    OrderId id;
    OrderSide side;
    OrderType type;
    Qty qty;
    Qty remaining;
    std::optional<Price> price;
};

struct Trade {
    Price price;
    Qty qty;
    OrderSide aggressor;
    OrderId makerId;
    OrderId takerId;
};

#endif