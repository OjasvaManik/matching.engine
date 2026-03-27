#pragma once
#include <cstdint>
#include <vector>

enum class OrderType {
    GoodTillCancelled,
    FillAndKill,
    FillOrKill,
    GoodForDay,
    Market
};

enum class OrderSide {
    BUY,
    SELL
};

using Price = int32_t;
using Quantity = uint32_t;
using OrderId = uint64_t;
using OrderIds = std::vector<OrderId>;
