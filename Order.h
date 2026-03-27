#pragma once
#include "Types.h"
#include <format>
#include <list>
#include <memory>
#include <stdexcept>

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
