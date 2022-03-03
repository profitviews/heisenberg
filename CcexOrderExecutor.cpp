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
using ::ccapi::UtilSystem;

const std::map<OrderExecutor::OrderType, std::string> CcexOrderExecutor::order_type_names_{
    {OrderType::limit, "Limit"}, {OrderType::market, "Market"}};

const std::map<OrderExecutor::Side, std::string> CcexOrderExecutor::side_names_{
    {Side::buy, "Buy"}, {Side::sell, "Sell"}};

const std::map<std::string, std::tuple<std::string, std::string, std::string>>
    CcexOrderExecutor::exchange_key_names_ =
        {
            {CCAPI_EXCHANGE_NAME_FTX, {CCAPI_FTX_API_KEY, CCAPI_FTX_API_SECRET, ""}},
            {CCAPI_EXCHANGE_NAME_BITMEX, {CCAPI_BITMEX_API_KEY, CCAPI_FTX_API_SECRET, ""}},
            {CCAPI_EXCHANGE_NAME_COINBASE, 
              {CCAPI_COINBASE_API_KEY, CCAPI_COINBASE_API_SECRET, CCAPI_COINBASE_API_PASSPHRASE}},
        };

CcexOrderExecutor::CcexOrderExecutor(
    const std::string &exchange, int expiry,
    const std::string &api_key,
    const std::string &api_secret,
    const std::string &pass_phrase)
    : exchange_{exchange}, api_key_{api_key}, api_secret_{api_secret}, expiry_{expiry}, pass_phrase_{pass_phrase}
{
}
using namespace boost::mp11;
using namespace boost::typeindex;
namespace ct = boost::callable_traits;

void CcexOrderExecutor::new_order(const std::string &symbol, Side side, double orderQty, OrderType type, double price)
{
  SessionOptions session_options;
  SessionConfigs session_configs;
  CcexOrderHandler event_handler;

  using setCredential_p1 = mp_at_c<ct::args_t<decltype(&SessionConfigs::setCredential)>, 1>;

  setCredential_p1 cred = {
      {std::get<0>(exchange_key_names_.at(exchange_)), api_key_},
      {std::get<1>(exchange_key_names_.at(exchange_)), api_secret_},
      {std::get<2>(exchange_key_names_.at(exchange_)), pass_phrase_},
  };

  session_configs.setCredential(cred);

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