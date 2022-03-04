#include "CcexOrderExecutor.h"

#include <ccapi_cpp/ccapi_session.h>

#include <boost/mp11.hpp>
#include <boost/callable_traits.hpp>
#include <boost/type_index.hpp>

#include <tuple>
#include <map>

namespace ccapi
{
  Logger *Logger::logger = nullptr; // This line is needed.
}

using ::ccapi::Request;
using ::ccapi::Session;
using ::ccapi::SessionConfigs;
using ::ccapi::SessionOptions;

CcexOrderExecutor::CcexOrderExecutor(
    const std::string &exchange, int expiry,
    const std::string &api_key,
    const std::string &api_secret,
    const std::string &pass_phrase)
    : exchange_{exchange}, api_key_{api_key}, api_secret_{api_secret}, expiry_{expiry}, pass_phrase_{pass_phrase}
{
}

void CcexOrderExecutor::new_order(const std::string &symbol, Side side, double orderQty, OrderType type, double price)
{
  SessionOptions session_options;
  SessionConfigs session_configs;
  CcexOrderHandler event_handler;

  enum { api_key, api_secret, pass_phrase };
  session_configs.setCredential({
      {std::get<api_key    >(exchange_key_names_.at(exchange_)), api_key_    },
      {std::get<api_secret >(exchange_key_names_.at(exchange_)), api_secret_ },
      {std::get<pass_phrase>(exchange_key_names_.at(exchange_)), pass_phrase_},
  });

  Session session(session_options, session_configs, &event_handler);

  Request request(Request::Operation::CREATE_ORDER, exchange_, symbol);
  request.appendParam({{"type", type == OrderType::market ? "market" : "limit"},
                       {"side", side == Side::buy ? "BUY" : "SELL"},
                       {"size", std::to_string(orderQty)},
                       {"price", type == OrderType::limit ? std::to_string(price) : "0.0001"}});
  session.sendRequest(request);
  event_handler.wait();
  session.stop();
}

CcexOrderExecutor::~CcexOrderExecutor()
{
}