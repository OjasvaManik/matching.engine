#include "OrderBook.h"
#include <print>
#include <chrono>
#include <vector>

int main() {
    constexpr int num_orders = 1000000;
    constexpr int num_iterations = 25;
    double total_duration = 0.0;

    std::println("Starting benchmark with {} iterations of {} orders each...\n", num_iterations, num_orders);

    for (int iter = 0; iter < num_iterations; ++iter) {
        OrderBook book;
        std::vector<OrderPointer> orders;
        orders.reserve(num_orders);

        for (int i = 0; i < num_orders; ++i) {
            OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
            Price price = 100 + (i % 5);
            orders.push_back(std::make_shared<Order>(i + 1, side, OrderType::GoodTillCancelled, price, 10));
        }

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto &order: orders) {
            book.add_order(order);
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> duration = end - start;
        total_duration += duration.count();
        double current_throughput = num_orders / duration.count();

        std::println("Iteration {}: Processed in {:.6f} seconds ({:.2f} orders/second).",
                     iter + 1, duration.count(), current_throughput);
    }

    double average_duration = total_duration / num_iterations;
    double average_throughput = num_orders / average_duration;

    std::println("\n--- Benchmark Results ---");
    std::println("Average Duration: {:.6f} seconds.", average_duration);
    std::println("Average Throughput: {:.2f} orders/second.", average_throughput);

    return 0;
}
