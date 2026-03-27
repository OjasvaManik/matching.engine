#pragma once
#include "Order.h"
#include "Types.h"

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
