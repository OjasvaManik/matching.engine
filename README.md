# C++ Matching Engine

## Overview

A single-threaded Limit Order Book (LOB) matching engine written in C++. This project implements core exchange
functionality, matching buy and sell orders based on price-time priority. The current implementation utilizes
`std::list` and `std::map` to manage order queues and price levels.

## Features

* **Price-Time Priority Matching:** Standard FIFO execution for resting orders at the same price level.
* **O(1) Order Insertion:** Optimized list insertion to place new orders at the back of the queue in constant time,
  preventing exponential performance degradation under load.
* **Supported Order Types:**
    * Market
    * Good Till Cancelled (GTC)
    * Fill Or Kill (FOK)

## Architecture

The order book maintains bids and asks using `std::map` sorted by price (descending for bids, ascending for asks). Each
price level contains a queue of orders managed by a `std::list` of `std::shared_ptr<Order>`. An internal
`std::unordered_map` tracks order IDs for O(1) lookups during cancellations or modifications.

## Performance Benchmarks

The engine was benchmarked by submitting 1,000,000 sequential orders alternating between Buy and Sell across varying
price levels to force continuous matching and order book updates.

**Test Parameters:**

* Iterations: 25
* Orders per iteration: 1,000,000
* Order Type: Good Till Cancelled

**Results:**

* **Average Duration:** 1.151775 seconds
* **Average Throughput:** 868,224.78 orders/second
* **Peak Throughput:** 881,641.97 orders/second

## Usage

The engine requires a compiler that supports C++20 or later.

```cpp
#include "OrderBook.h"

int main() {
    OrderBook book;
    
    book.add_order(std::make_shared<Order>(1, OrderSide::BUY, OrderType::GoodTillCancelled, 100, 10));
    
    Trades trades = book.add_order(std::make_shared<Order>(2, OrderSide::SELL, OrderType::Market, 10));
    
    return 0;
}
```

## Roadmap

Development is currently paused, but future iterations will focus on memory layout optimizations to reduce heap
allocations and CPU cache misses. Planned updates include:

* Implementing alternative data structures using `std::vector` and price-based arrays.
* Creating an automated benchmark suite to compare speeds between List, Vector, and Array implementations.
* Adding support for Fill And Kill (FAK) and Good For Day (GFD) order types.