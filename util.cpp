#include "util.h"

void util::log_trade(Poco::Logger& logger, Poco::JSON::Object::Ptr& result_object)
{
    logger.information("Price: " + result_object->get("price").toString());
	logger.information("Side: " + result_object->get("side").toString());
	logger.information("Size: " + result_object->get("size").toString());
	logger.information("Source: " + result_object->get("src").toString());
	logger.information("Symbol: " + result_object->get("sym").toString());
}