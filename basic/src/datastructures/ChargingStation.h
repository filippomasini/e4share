/*
 * ChargingStation.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef CHARGINGSTATION_H_
#define CHARGINGSTATION_H_

#include "StreetNetwork.h"

namespace e4share
{

class ChargingStation
{
public:
	ChargingStation(StreetNetwork& network_, StreetNetwork::Vertex& location_, int cost_, int costPerSlot_, int capacity_);

private:
	StreetNetwork& network;
	StreetNetwork::Vertex& location;
	int cost;
	int costPerSlot;
	int capacity;
};

} /* namespace e4share */

#endif /* CHARGINGSTATION_H_ */
