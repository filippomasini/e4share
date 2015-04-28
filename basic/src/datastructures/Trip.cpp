/*
 * Trip.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "Trip.h"

namespace e4share
{

Trip::Trip(StreetNetwork& network_, StreetNetwork::Vertex origin_, StreetNetwork::Vertex destination_, int beginTime_, int endTime_,
		int batteryConsumption_, int profit_) :
				network(network_),
				origin(origin_),
				destination(destination_),
				beginTime(beginTime_),
				endTime(endTime_),
				batteryConsumption(batteryConsumption_),
				profit(profit_)
{
	// TODO Auto-generated constructor stub

}

} /* namespace e4share */
