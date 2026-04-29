# `CcexOrderExecutor` (live ccapi orders)

[`src/ccex_order_executor.hpp`](../src/ccex_order_executor.hpp) implements [`OrderExecutor`](../src/order_executor.hpp) for **real exchange connectivity** using **cryptochronic-api (ccapi)**. Algos and CLI use the same synchronous API: **`new_order(...)`** blocks until the exchange acknowledges the create (or a timeout).

For tests and offline smoke without keys, see [`SandboxOrderExecutor`](../src/sandbox_order_executor.hpp) instead.

## Why not one Session per order?

ccapi’s **`Session`** owns networking and dispatch threads. Creating and tearing down a **`Session`** for every order adds latency and churn. **`CcexOrderExecutor`** keeps **one long-lived `Session`**, created lazily on the first **`new_order`** via **`ensure_session()`**, and stopped only in the destructor.

## End-to-end flow

1. **Caller** invokes **`new_order(symbol, side, qty, type, price)`** (same signature as the base interface).
2. **Correlation id:** A unique string is allocated (**`pv-<n>`**, monotonic **`std::atomic<uint64_t>`**) so each request can be matched to exactly one waiter.
3. **Pending completion:** Under **`pending_mu_`**, a **`std::shared_ptr<std::promise<void>>`** is stored in **`pending_[cid]`**. The caller holds the matching **`std::future`** from **`get_future()`**.
4. **Request:** **`Request(Operation::CREATE_ORDER, exchange_, symbol, cid)`** attaches **`cid`** so ccapi echoes it on the response (**`Message::getCorrelationIdList()[0]`**).
5. **`session_->sendRequest(request)`** — ccapi documents **`sendRequest`** as **thread-safe**, so multiple callers may submit concurrently.
6. **Blocking wait:** The caller **`wait_for(order_response_timeout_)`** (currently **120s**) on the future.
7. **Dispatch:** ccapi delivers the async result on its **event dispatcher** thread(s). The nested **`CcexOrderHandler::processEvent`** forwards to **`on_ccapi_order_event`**.
8. **Completion:** **`on_ccapi_order_event`** parses logging/status, optionally **`add_open_order`** when **`LIMIT_PRICE`** is present (limit-order acknowledgement path), then under **`pending_mu_`** **moves** the promise out of **`pending_`**, erases the map entry, and **`set_value()`** on the promise **outside** the lock so the waiter unblocks.
9. **Timeout:** If **`wait_for`** returns **`timeout`**, **`cid`** is **erased** from **`pending_`** so a **late** duplicate response does not satisfy a **different** future — those events are logged and ignored.

## Threading

- **Caller thread(s):** **`new_order`**, **`ensure_session`** (guarded by **`session_start_mu_`**), **`pending_`** insert/erase under **`pending_mu_`**.
- **Dispatcher thread(s):** **`processEvent`** / **`on_ccapi_order_event`** — must not assume it runs on the caller thread.

Cross-thread **`promise::set_value`** / **`future::wait`** is standard and matches ccapi’s model (see upstream ccapi README **“Thread safety”**).

## Destruction order

**`handler_`** is declared **before** **`session_`**. The destructor locks **`session_start_mu_`**, calls **`session_->stop()`**, then **`session_.reset()`**, so the **`Session`** does not outlive the **`EventHandler`** pointer passed at construction. **`CcexOrderExecutor`** is non-copyable and non-movable.

## Credentials and venues

The constructor maps **`exchange_`** to ccapi credential field names (**`exchange_key_names_`**) for supported venues (e.g. Coinbase passphrase, FTX subaccount). **`SessionConfigs`** is filled before any **`Session`** exists.

## Limitations / caveats

- **`open_orders_`** is keyed by correlation id and populated when the first response includes **`LIMIT_PRICE`**. **Market** acks without limit price do not go through that branch — **`open_orders_`** may stay empty for those paths unless extended later.
- Empty message lists or missing correlation ids are logged; duplicate completions catch **`std::future_error`** if **`set_value`** runs twice.
