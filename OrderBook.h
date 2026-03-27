#pragma once
#include "LevelInfo.h"
#include "Order.h"
#include "OrderModify.h"
#include "Trade.h"
#include <map>
#include <unordered_map>

class OrderBook {
public:
    Trades add_order(const OrderPointer &order);

    void cancel_order(const OrderId id);

    Trades modify_order(const OrderModify &modify);

    [[nodiscard]] std::size_t get_order_count() const;

    [[nodiscard]] OrderBookLevel get_order_infos() const;

private:
    struct OrderEntry {
        OrderPointer _order{nullptr};
        Orders::iterator _iterator;
    };

    std::map<Price, Orders, std::greater<> > _bids;
    std::map<Price, Orders, std::less<> > _asks;
    std::unordered_map<OrderId, OrderEntry> _orders;

    bool can_match(const OrderSide side, const Price price);

    Trades match_orders();
};
