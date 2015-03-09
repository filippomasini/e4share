/*
 * ChargingStation.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef CHARGINGSTATION_H_
#define CHARGINGSTATION_H_

namespace e4share
{

class ChargingStation
{
public:
	ChargingStation(int cost_, int costPerSlot_, int capacity_);

	int getCapacity() const
	{
		return capacity;
	}

	int getCost() const
	{
		return cost;
	}

	int getCostPerSlot() const
	{
		return costPerSlot;
	}

private:
	int cost;
	int costPerSlot;
	int capacity;
};

} /* namespace e4share */

#endif /* CHARGINGSTATION_H_ */
