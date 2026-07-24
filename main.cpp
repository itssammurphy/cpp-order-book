#include <iostream>
#include "order.hpp"

const char* sideToString(OrderSide side) {
    return side == OrderSide::Buy ? "BUY" : "SELL";
}

int main() {
    Order order{
        1,
        OrderSide::Buy,
        OrderType::Limit,
        10,
        10,
        10005 // $100.05
    };

    std::cout << "Order book simulation\n";
    std::cout << "Created order "
              << order.id << ": " << sideToString(order.side)
              << " " << order.qty << " @ "
              << *order.price << "\n";
    return 0;
}