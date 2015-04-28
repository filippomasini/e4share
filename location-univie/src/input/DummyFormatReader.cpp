/*
 * DummyFormatReader.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "input/DummyFormatReader.h"

#include <fstream>

namespace e4share
{

DummyFormatReader::DummyFormatReader(int walkingDistance_) :
		walkingDistance(walkingDistance_)
{
	// TODO Auto-generated constructor stub

}

CSLocationInstance DummyFormatReader::readInstance(std::string filename)
{
	std::string toignore;
	std::ifstream instancefile(filename);
	//instancefile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	// ignore comment line
	//std::getline(instancefile, toignore);
	instancefile.ignore(1024, '\n');
	std::cout << instancefile << std::endl;

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

	std::cout << vertexCount << "," << edgeCount << std::endl;
	std::cout << instancefile << std::endl;
	// create vertices
	StreetNetwork network(walkingDistance);
	for(int i = 0; i < vertexCount; i++)
	{
		network.addVertex();
	}

	// ignore comment lines
	for(int i = 0; i < 6; i++)
	{
		instancefile >> toignore;
		std::cout << toignore << std::endl;
	}

	// next edgeCount lines:
	// source target distance
	for(int i = 0; i < edgeCount; i++)
	{
		//std::cout << "blah" << std::endl;
		StreetNetwork::Vertex source, target;
		int distance;
		instancefile >> source;
		instancefile >> target;
		instancefile >> distance;
		network.addArcPair(source, target, distance);
		//std::cout << source << "," << target << "," << distance << std::endl;
	}

	// ignore comment lines
	for(int i = 0; i < 7; i++)
	{
		instancefile >> toignore;
		std::cout << toignore << std::endl;
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

	// ignore comment lines
	for(int i = 0; i < 9; i++)
	{
		instancefile >> toignore;
		std::cout << toignore << std::endl;
	}

	// next tripCount lines:
	// origin destination beginTime endTime batteryConsumption profit
	std::vector<Trip> trips;
	for(int i = 0; i < tripCount; i++)
	{
		StreetNetwork::Vertex origin, destination;
		int beginTime, endTime, profit;
		int batteryConsumption;
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
