#pragma once
#include "Types.h"
#include <vector>

struct TradeInfo {
    OrderId order_id;
    Price price;
    Quantity quantity;
};

class Trade {
public:
    Trade(const TradeInfo &bids_trade, const TradeInfo &asks_trade) : _bids_trade(bids_trade), _asks_trade(asks_trade) {
    }

    [[nodiscard]] const TradeInfo &get_bids_trade() const { return _bids_trade; }
    [[nodiscard]] const TradeInfo &get_asks_trade() const { return _asks_trade; }

private:
    TradeInfo _bids_trade;
    TradeInfo _asks_trade;
};

using Trades = std::vector<Trade>;
