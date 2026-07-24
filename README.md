# c++ orderbook simulator

a command line order book simulation written in C++ by Sam Murphy

Features this will include:
- limit and market orders
- partial fills
- order cancellation
- book depth
- price-time-priority
- simulated order flow

To compile:
```
g++ -std=c++20 -Wall -Wextra -pedantic main.cpp -o orderbook
```

# current status

at the moment it supports resting limit orders and querying the top of the book. i have implemented basic market order matching

- bids sorted from highest to lowest price
- asks sorted from lowest to highest price
- best bid and ask can be safely queried on empty books
- orders at same price retain arrival order
- market orders match against resting orders at best price
- partial fills for resting and incoming orders
- market orders continue through multiple price levels if necessary
- unfilled market order qty is discarded rather than added to book
- resting orders can be cancelled by their id
- id-wise cancellation doesn't scan the whole book during cancellation
- fully-filled orders are removed from the cancellation index