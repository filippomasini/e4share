/*
 * TempFormatReader.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "TempFormatReader.h"

#include <fstream>

namespace e4share
{

TempFormatReader::TempFormatReader()
{
	// TODO Auto-generated constructor stub

}

CSLocationInstance TempFormatReader::readInstance(std::string filename)
{
	std::ifstream instancefile;
	instancefile.open(filename);

	// first line:
	// vertexCount edgeCount stationCount carCount tripCount

	int vertexCount;
	instancefile >> vertexCount;

	int edgeCount;
	instancefile >> edgeCount;

	int stationCount;
	instancefile >> stationCount;

	int carCount;
	instancefile >> carCount;

	int tripCount;
	instancefile >> tripCount;


	// create vertices
	StreetNetwork network;
	for(int i = 0; i < vertexCount; i++)
	{
		network.addVertex();
	}

	// next edgeCount lines:
	// source target distance
	for(int i = 0; i < edgeCount; i++)
	{
		StreetNetwork::Vertex source, target;
		int distance;
		instancefile >> source;
		instancefile >> target;
		instancefile >> distance;
		network.addArcPair(source, target, distance);
	}

	// next stationCount lines:
	// location cost costPerSlot capacity
	for(int i = 0; i < stationCount; i++)
	{
		StreetNetwork::Vertex location;
		int cost, costPerSlot, capacity;
		instancefile >> location;
		instancefile >> cost;
		instancefile >> costPerSlot;
		instancefile >> capacity;
		network.addChargingStation(location, cost, costPerSlot, capacity);
	}

	// next tripCount lines:
	// origin destination beginTime endTime batteryConsumption profit
	std::vector<Trip> trips;
	for(int i = 0; i < tripCount; i++)
	{
		StreetNetwork::Vertex origin, destination;
		int beginTime, endTime, profit;
		double batteryConsumption;
		instancefile >> origin;
		instancefile >> destination;
		instancefile >> beginTime;
		instancefile >> endTime;
		instancefile >> batteryConsumption;
		instancefile >> profit;
		Trip trip(network, origin, destination, beginTime, endTime, batteryConsumption, profit);
		trips.push_back(trip);
	}

	instancefile.close();

	CSLocationInstance instance(network, trips, carCount);
	return instance;
}

} /* namespace e4share */
