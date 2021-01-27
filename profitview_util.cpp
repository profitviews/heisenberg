#include "profitview_util.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>

void profitview::util::log_trade(Poco::Logger& logger, Poco::JSON::Object::Ptr& result_object)
{
    logger.information("Price: " + result_object->get("price").toString());
	logger.information("Side: " + result_object->get("side").toString());
	logger.information("Size: " + result_object->get("size").toString());
	logger.information("Source: " + result_object->get("src").toString());
	logger.information("Symbol: " + result_object->get("sym").toString());
}

Poco::JSON::Array::Ptr profitview::util::constructSymbolJSON(int number_of_symbols, char** symbols)
{
	std::string symbols_json_string{"[\""};
	symbols_json_string += symbols[0] + std::string{"\""};
	for (int n = 1; n < number_of_symbols; ++n)
		symbols_json_string += ",\"" + std::string{symbols[n]} + "\"";
	symbols_json_string += "]";

	using namespace Poco::JSON;
	return Parser().parse(symbols_json_string).extract<Array::Ptr>();
}