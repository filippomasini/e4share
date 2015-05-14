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
	Trip(int index_, StreetNetwork& network_, StreetNetwork::Vertex origin_, StreetNetwork::Vertex destination_, int beginTime_, int endTime_,
			int batteryConsumption_, int profit_);

	int getBatteryConsumption() const
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

	StreetNetwork::Vertex getOrigin() const
	{
		return origin;
	}

	StreetNetwork::Vertex getDestination() const
	{
		return destination;
	}

	bool operator < (const Trip& that) const
	{
		return index < that.index;
	}

	bool operator == (const Trip& that) const
	{
		return index == that.index;
	}

private:
	int index;
	StreetNetwork& network;
	StreetNetwork::Vertex origin;
	StreetNetwork::Vertex destination;
	int beginTime;
	int endTime;
	int batteryConsumption;
	int profit;
};

} /* namespace e4share */

#endif /* TRIP_H_ */
