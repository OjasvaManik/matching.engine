#pragma once
#include <condition_variable>

#include "LevelInfo.h"
#include "Order.h"
#include "OrderModify.h"
#include "Trade.h"
#include <map>
#include <thread>
#include <unordered_map>

class OrderBook {
public:
    OrderBook();

    OrderBook(const OrderBook &) = delete;

    void operator=(const OrderBook &) = delete;

    OrderBook(OrderBook &&) = delete;

    void operator=(OrderBook &&) = delete;

    ~OrderBook();

    Trades add_order(const OrderPointer &order);

    void cancel_order(OrderId id);

    Trades modify_order(const OrderModify &modify);

    [[nodiscard]] std::size_t get_order_count() const;

    [[nodiscard]] OrderBookLevel get_order_infos() const;

private:
    struct OrderEntry {
        OrderPointer _order{nullptr};
        Orders::iterator _iterator;
    };

    struct LevelData {
        Quantity quantity{};
        Quantity count{};

        enum class Action {
            ADD,
            REMOVE,
            MATCH
        };
    };

    std::map<Price, Orders, std::greater<> > _bids;
    std::map<Price, Orders, std::less<> > _asks;
    std::unordered_map<OrderId, OrderEntry> _orders;

    std::unordered_map<Price, LevelData> _level_data;
    mutable std::mutex _orders_mutex;
    std::thread _order_prune_thread;
    std::condition_variable _shutdown_condition_variable;
    std::atomic_bool _shutdown{false};

    bool can_match(OrderSide side, Price price);

    void prune_good_for_day_orders();

    void cancel_orders(OrderIds ids);

    void cancel_order_internal(OrderId id);

    void on_order_cancelled(OrderPointer order);

    void on_order_added(OrderPointer order);

    void on_order_matched(Price price, Quantity quantity, bool isFullyFilled);

    void update_level_data(Price price, Quantity quantity, LevelData::Action action);

    bool can_fully_fill(OrderSide side, Price price, Quantity quantity) const;

    Trades match_orders();
};
