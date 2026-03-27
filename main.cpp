#include "OrderBook.h"
#include <print>

int main() {
    OrderBook book;
    constexpr OrderId id = 1;
    book.add_order(std::make_shared<Order>(id, OrderSide::BUY, OrderType::GoodTillCancelled, 100, 10));
    std::println("Size of OrderBook: {}", book.get_order_count());
    book.cancel_order(id);
    std::println("Size of OrderBook: {}", book.get_order_count());
    return 0;
}
