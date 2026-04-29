#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <future>
#include <mutex>
#include <thread>

namespace
{

constexpr auto k_short_timeout = std::chrono::milliseconds(50);
constexpr auto k_wait_budget = std::chrono::seconds(5);

}    // namespace

// Mirrors the completion handoff in ccex_order_executor.hpp (CcexOrderHandler + new_order): shared
// promise, call_once for duplicate events, waiter uses wait_for.

TEST_CASE("wait_for returns timeout when promise is never completed", "[order_sync]")
{
    std::promise<void> completion;
    std::future<void> const done = completion.get_future();
    REQUIRE(done.wait_for(k_short_timeout) == std::future_status::timeout);
}

TEST_CASE("call_once allows only one set_value on shared promise", "[order_sync]")
{
    auto completion = std::make_shared<std::promise<void>>();
    std::future<void> const done = completion->get_future();
    std::once_flag once;

    std::call_once(once, [&]() { completion->set_value(); });
    std::call_once(once, [&]() { completion->set_value(); });

    REQUIRE(done.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
}

TEST_CASE("background thread completes promise; waiter observes ready within budget", "[order_sync]")
{
    auto completion = std::make_shared<std::promise<void>>();
    std::future<void> const done = completion->get_future();
    std::once_flag once;

    std::thread worker([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        std::call_once(once, [&]() { completion->set_value(); });
    });

    REQUIRE(done.wait_for(k_wait_budget) == std::future_status::ready);
    worker.join();
}
