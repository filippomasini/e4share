/*
 * Trip.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef TRIP_H_
#define TRIP_H_

#include "StreetNetwork.h"

namespace e4share
{

class Trip
{

public:
	Trip(StreetNetwork& network_, StreetNetwork::Vertex& origin_, StreetNetwork::Vertex& destination_, int beginTime_, int endTime_,
			double batteryConsumption_, int profit_);

	double getBatteryConsumption() const
	{
		return batteryConsumption;
	}

	int getBeginTime() const
	{
		return beginTime;
	}

	int getEndTime() const
	{
		return endTime;
	}

	int getProfit() const
	{
		return profit;
	}

private:
	StreetNetwork& network;
	StreetNetwork::Vertex& origin;
	StreetNetwork::Vertex& destination;
	int beginTime;
	int endTime;
	double batteryConsumption;
	int profit;
};

} /* namespace e4share */

#endif /* TRIP_H_ */
