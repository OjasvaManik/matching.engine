# C++ Matching Engine

## Overview

A single-threaded Limit Order Book (LOB) matching engine written in C++, implementing core exchange functionality
with price-time priority matching. Benchmarked at **~12.5M orders/second** on the core matching path — comparable
to the throughput of production exchange matching cores before network I/O, persistence, and risk check overhead
are factored in.

The current implementation utilizes `std::list` and `std::map` to manage order queues and price levels, serving
as the baseline for an ongoing multi-implementation benchmark study.

## Features

* **Price-Time Priority Matching:** Standard FIFO execution for resting orders at the same price level.
* **O(1) Order Insertion:** Optimized list insertion to place new orders at the back of the queue in constant time,
  preventing exponential performance degradation under load.
* **Supported Order Types:**
    * Market
    * Good Till Cancelled (GTC)
    * Fill Or Kill (FOK)

## Architecture

The order book maintains bids and asks using `std::map` sorted by price (descending for bids, ascending for asks).
Each price level contains a queue of orders managed by a `std::list` of `std::shared_ptr<Order>`. An internal
`std::unordered_map` tracks order IDs for O(1) lookups during cancellations or modifications.

## Performance Benchmarks

Benchmarks measure the **core matching logic in isolation** — order ingestion, price-time priority matching, and
book maintenance. Production engines operate in the 1–10M orders/second range when the full pipeline is included
(network parsing, pre-trade risk checks, regulatory logging, and market data dissemination). This benchmark
isolates the matching core specifically to evaluate data structure performance, which is the primary variable
across the planned implementation variants.

The engine was benchmarked by submitting 1,000,000 sequential orders alternating between Buy and Sell across
varying price levels to force continuous matching and order book updates.

**Test Parameters:**

* Iterations: 25
* Orders per iteration: 1,000,000
* Order Type: Good Till Cancelled
* Build: Release (`-O3`, optimized)

**Results:**

* **Average Duration:** 0.079719 seconds
* **Average Throughput:** 12,544,051.86 orders/second
* **Peak Throughput:** 14,018,759.06 orders/second

> ⚠️ Note: Debug builds (`-O0`) result in significantly lower throughput (~0.8M orders/sec).  
> All benchmarks above are measured using optimized Release builds.  
> Iterations 1–3 excluded from analysis to account for CPU branch predictor and cache warmup effects.

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

Development is currently paused on feature additions. The next phase focuses on memory layout optimizations
to reduce heap allocations and CPU cache misses, with each variant benchmarked against this `std::list` baseline
to quantify the performance impact of each structural change:

* **`std::vector` backend** — contiguous memory for order queues, trading insertion cost for improved cache
  locality on iteration.
* **Price-bucketed array** — O(1) price level lookup via direct indexing, eliminating tree traversal entirely.
  Expected to significantly outperform both list and vector variants.
* **Automated benchmark suite** — identical workloads across all three implementations with statistical
  aggregation (median, P99 latency, throughput).
* **Additional order types** — Fill And Kill (FAK) and Good For Day (GFD).