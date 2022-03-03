#include "profitview_poco.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>

void profitview::poco::Logger::log_trade(Poco::JSON::Object::Ptr& result_object)
{
    logger_.information("Price: " + result_object->get("price").toString());
	logger_.information("Side: " + result_object->get("side").toString());
	logger_.information("Size: " + result_object->get("size").toString());
	logger_.information("Source: " + result_object->get("src").toString());
	logger_.information("Symbol: " + result_object->get("sym").toString());
}

Poco::JSON::Array::Ptr profitview::poco::constructSymbolJSON(int number_of_symbols, char** symbols)
{
	std::string symbols_json_string{"[\""};
	symbols_json_string += symbols[0] + std::string{"\""};
	for (int n = 1; n < number_of_symbols; ++n)
		symbols_json_string += ",\"" + std::string{symbols[n]} + "\"";
	symbols_json_string += "]";

	using namespace Poco::JSON;
	return Parser().parse(symbols_json_string).extract<Array::Ptr>();
}