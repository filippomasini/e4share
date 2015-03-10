/*
 * CSLocationInstance.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef CSLOCATIONINSTANCE_H_
#define CSLOCATIONINSTANCE_H_

#include <vector>

#include "datastructures/StreetNetwork.h"
#include "datastructures/Trip.h"

namespace e4share
{

class CSLocationInstance
{
public:
	CSLocationInstance(StreetNetwork network_, std::vector<Trip> trips_, int carCount_);

	int getCarCount() const
	{
		return carCount;
	}

	const StreetNetwork& getNetwork() const
	{
		return network;
	}

	const std::vector<Trip>& getTrips() const
	{
		return trips;
	}

private:
	StreetNetwork network;
	std::vector<Trip> trips;
	int carCount;
};

} /* namespace e4share */

#endif /* CSLOCATIONINSTANCE_H_ */
