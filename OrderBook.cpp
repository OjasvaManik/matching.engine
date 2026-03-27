#include "OrderBook.h"
#include <algorithm>
#include <numeric>

OrderBook::OrderBook() = default;

OrderBook::~OrderBook() = default;

Trades OrderBook::add_order(const OrderPointer &order) {
    if (_orders.contains(order->get_id())) {
        return {};
    }
    if (order->get_type() == OrderType::FillAndKill && !can_match(order->get_side(), order->get_price())) {
        return {};
    }

    if (order->get_type() == OrderType::Market) {
        if (order->get_side() == OrderSide::BUY && !_asks.empty()) {
            const auto &[worst_ask, _] = *_asks.rbegin();
            order->to_good_till_cancelled(worst_ask);
        } else if (order->get_side() == OrderSide::SELL && !_bids.empty()) {
            const auto &[worst_bid, _] = *_bids.rbegin();
            order->to_good_till_cancelled(worst_bid);
        } else {
            return {};
        }
    }

    // Orders::iterator iterator;
    // if (order->get_side() == OrderSide::BUY) {
    //     auto &orders = _bids[order->get_price()];
    //     orders.emplace_back(order);
    //     iterator = std::next(orders.begin(), orders.size() - 1);
    // } else {
    //     auto &orders = _asks[order->get_price()];
    //     orders.emplace_back(order);
    //     iterator = std::next(orders.begin(), orders.size() - 1);
    // }

    Orders::iterator iterator;
    if (order->get_side() == OrderSide::BUY) {
        auto &orders = _bids[order->get_price()];
        orders.emplace_back(order);
        iterator = std::prev(orders.end());
    } else {
        auto &orders = _asks[order->get_price()];
        orders.emplace_back(order);
        iterator = std::prev(orders.end());
    }
    _orders.insert({order->get_id(), OrderEntry{order, iterator}});
    return match_orders();
}

void OrderBook::cancel_order(const OrderId id) {
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

Trades OrderBook::modify_order(const OrderModify &modify) {
    if (!_orders.contains(modify.get_id())) return {};
    cancel_order(modify.get_id());
    return add_order(modify.to_order());
}

std::size_t OrderBook::get_order_count() const {
    return _orders.size();
}

OrderBookLevel OrderBook::get_order_infos() const {
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

bool OrderBook::can_match(const OrderSide side, const Price price) {
    if (side == OrderSide::BUY) {
        if (_asks.empty()) return false;
        const auto &[best_ask, _] = *_asks.begin();
        return price >= best_ask;
    }

    if (_bids.empty()) return false;
    const auto &[bestBid, _] = *_bids.begin();
    return price <= bestBid;
}

Trades OrderBook::match_orders() {
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
        if (const auto &bid = bids.front(); bid->get_type() == OrderType::FillAndKill) {
        }
    }

    if (!_asks.empty()) {
        auto &[_, asks] = *_asks.begin();
        if (const auto &ask = asks.front(); ask->get_type() == OrderType::FillAndKill) {
        }
    }

    return trades;
}
