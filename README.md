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

at the moment it supports resting limit orders and querying the top of the book.
- bids sorted from highest to lowest price
- asks sorted from lowest to highest price
- best bid and ask can be safely queried on empty books
- no matching or cancellation yet