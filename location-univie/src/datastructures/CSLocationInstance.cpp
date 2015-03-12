/*
 * CSLocationInstance.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "CSLocationInstance.h"

namespace e4share
{

CSLocationInstance::CSLocationInstance(StreetNetwork network_, std::vector<Trip> trips_, int carCount_) :
		network(network_),
		trips(trips_),
		carCount(carCount_),
		maxTime(0)
{
	// calculate maximum time
	for(Trip trip : trips)
	{
		if(trip.getEndTime() > maxTime)
		{
			maxTime = trip.getEndTime();
		}
	}

}

} /* namespace e4share */
