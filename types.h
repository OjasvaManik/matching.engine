#pragma once
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <print>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

enum class OrderType {
    GoodTillCancelled,
    FillOrKill
};

enum class OrderSide {
    BUY,
    SELL
};

using Price = int32_t;
using Quantity = uint32_t;
using OrderId = uint64_t;

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

class Order {
public:
    Order(const OrderId id, const OrderSide side, const OrderType type, const Price price,
          const Quantity quantity) : _id(id), _side(side), _type(type), _price(price), _initial_quantity(quantity),
                                     _remaining_quantity(quantity) {
    }

    [[nodiscard]] OrderId get_id() const { return _id; }
    [[nodiscard]] OrderSide get_side() const { return _side; }
    [[nodiscard]] OrderType get_type() const { return _type; }
    [[nodiscard]] Price get_price() const { return _price; }
    [[nodiscard]] Quantity get_initial_quantity() const { return _initial_quantity; }
    [[nodiscard]] Quantity get_remaining_quantity() const { return _remaining_quantity; }

    [[nodiscard]] bool is_filled() const { return _remaining_quantity == 0; }

    [[nodiscard]] Quantity get_filled_quantity() const {
        return get_initial_quantity() - get_remaining_quantity();
    }

    void fill(const Quantity quantity) {
        if (quantity > get_remaining_quantity()) {
            throw std::logic_error(std::format("Order: {}, cannot be filled more than remaining quantity.", get_id()));
        }
        _remaining_quantity -= quantity;
    }

private:
    OrderId _id;
    OrderSide _side;
    OrderType _type;
    Price _price;
    Quantity _initial_quantity;
    Quantity _remaining_quantity;
};

using OrderPointer = std::shared_ptr<Order>;
using Orders = std::list<OrderPointer>;

class OrderModify {
public:
    OrderModify(const OrderId id, const OrderSide side, const Price price,
                const Quantity quantity) : _id(id), _side(side), _price(price), _quantity(quantity) {
    }

    [[nodiscard]] OrderId get_id() const { return _id; }
    [[nodiscard]] OrderSide get_side() const { return _side; }
    [[nodiscard]] Price get_price() const { return _price; }
    [[nodiscard]] Quantity get_quantity() const { return _quantity; }

    [[nodiscard]] OrderPointer to_order() const {
        return std::make_shared<Order>(_id, _side, OrderType::GoodTillCancelled, _price, _quantity);
    }

private:
    OrderId _id;
    OrderSide _side;
    Price _price;
    Quantity _quantity;
};

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

class OrderBook {
public:
    Trades add_order(const OrderPointer &order) {
        if (_orders.contains(order->get_id())) {
            return {};
        }
        if (order->get_type() == OrderType::FillOrKill && !can_match(order->get_side(), order->get_price())) {
            return {};
        }

        Orders::iterator iterator;
        if (order->get_side() == OrderSide::BUY) {
            auto &orders = _bids[order->get_price()];
            orders.emplace_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        } else {
            auto &orders = _asks[order->get_price()];
            orders.emplace_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }
        _orders.insert({order->get_id(), OrderEntry{order, iterator}});
        return match_orders();
    }

    void cancel_order(const OrderId id) {
        if (!_orders.contains(id)) return;
        const auto &[order, iterator] = _orders.at(id);
        _orders.erase(id);

        if (order->get_side() == OrderSide::BUY) {
            const auto price = order->get_price();
            auto &orders = _bids.at(price);
            orders.erase(iterator);
            if (orders.empty()) _bids.erase(price);
        } else {
            const auto price = order->get_price();
            auto &orders = _asks.at(price);
            orders.erase(iterator);
            if (orders.empty()) _asks.erase(price);
        }
    }

    Trades modify_order(const OrderModify &modify) {
        if (!_orders.contains(modify.get_id())) return {};
        const auto &[existing_order, _] = _orders.at(modify.get_id());
        cancel_order(modify.get_id());
        return add_order(modify.to_order());
    }

    std::size_t get_order_count() const { return _orders.size(); }

    OrderBookLevel get_order_infos() const {
        Levels bid_infos, ask_infos;
        bid_infos.reserve(_orders.size());
        ask_infos.reserve(_orders.size());

        auto create_level_info = [](Price price, const Orders &orders) {
            return LevelInfo{
                price,
                std::accumulate(orders.begin(), orders.end(), static_cast<Quantity>(0),
                                [](const Quantity running_sum, const OrderPointer &order) {
                                    return running_sum + order->get_remaining_quantity();
                                })
            };
        };

        for (const auto &[price, orders]: _bids) {
            bid_infos.emplace_back(create_level_info(price, orders));
        }

        for (const auto &[price, orders]: _asks) {
            ask_infos.emplace_back(create_level_info(price, orders));
        }

        return {std::move(bid_infos), std::move(ask_infos)};
    }

private:
    struct OrderEntry {
        OrderPointer _order{nullptr};
        Orders::iterator _iterator;
    };

    std::map<Price, Orders, std::greater<> > _bids;
    std::map<Price, Orders, std::less<> > _asks;
    std::unordered_map<OrderId, OrderEntry> _orders;

    bool can_match(const OrderSide side, const Price price) {
        if (side == OrderSide::BUY) {
            if (_asks.empty()) return false;
            const auto &[best_ask, _] = *_asks.begin();
            return price >= best_ask;
        }

        if (_bids.empty()) return false;
        const auto &[bestBid, _] = *_bids.begin();
        return price <= bestBid;
    }

    Trades match_orders() {
        Trades trades;
        trades.reserve(_orders.size());

        while (true) {
            if (_bids.empty() || _asks.empty()) break;

            auto &[bid_price, bids] = *_bids.begin();
            auto &[ask_price, asks] = *_asks.begin();

            if (bid_price < ask_price) break;
            while (!bids.empty() && !asks.empty()) {
                const auto &bid = bids.front();
                const auto &ask = asks.front();

                const Quantity quantity = std::min(bid->get_remaining_quantity(), ask->get_remaining_quantity());
                bid->fill(quantity);
                ask->fill(quantity);

                if (bid->is_filled()) {
                    _orders.erase(bid->get_id());
                    bids.pop_front();
                }
                if (ask->is_filled()) {
                    _orders.erase(ask->get_id());
                    asks.pop_front();
                }

                if (bids.empty()) _bids.erase(bid_price);
                if (asks.empty()) _asks.erase(ask_price);

                trades.emplace_back(
                    TradeInfo{bid->get_id(), bid_price, quantity},
                    TradeInfo{ask->get_id(), ask_price, quantity}
                );
            }
        }

        if (!_bids.empty()) {
            auto &[_, bids] = *_bids.begin();
            if (const auto &bid = bids.front(); bid->get_type() == OrderType::FillOrKill) {
            }
        }

        if (!_asks.empty()) {
            auto &[_, asks] = *_asks.begin();
            if (const auto &ask = asks.front(); ask->get_type() == OrderType::FillOrKill) {
            }
        }

        return trades;
    }
};
