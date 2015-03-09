/*
 * ChargingStation.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "ChargingStation.h"

namespace e4share
{

ChargingStation::ChargingStation(StreetNetwork& network_, StreetNetwork::Vertex& location_, int cost_, int costPerSlot_, int capacity_) :
	network(network_),
	location(location_),
	cost(cost_),
	costPerSlot(costPerSlot_),
	capacity(capacity_)
{
	// TODO Auto-generated constructor stub

}

} /* namespace e4share */
