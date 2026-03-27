#pragma once
#include "Types.h"
#include <utility>
#include <vector>

struct LevelInfo {
    Price price;
    Quantity quantity;
};

using Levels = std::vector<LevelInfo>;

class OrderBookLevel {
public:
    OrderBookLevel(Levels &&bids, Levels &&asks) : _bids(std::move(bids)), _asks(std::move(asks)) {
    }

    [[nodiscard]] const Levels &get_bids() const { return _bids; }
    [[nodiscard]] const Levels &get_asks() const { return _asks; }

private:
    Levels _bids;
    Levels _asks;
};
