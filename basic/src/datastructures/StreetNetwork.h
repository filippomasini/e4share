/*
 * StreetNetwork.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef STREETNETWORK_H_
#define STREETNETWORK_H_

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/optional.hpp>
#include <boost/property_map/property_map.hpp>

//class ChargingStation;
#include "ChargingStation.h"

namespace e4share
{

class StreetNetwork
{


public:
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::optional<ChargingStation>, int> Graph;
	typedef Graph::vertex_descriptor Vertex;
	typedef Graph::edge_descriptor Edge;

	StreetNetwork();
	Vertex addVertex();
	void addArc(Vertex source, Vertex target, int distance);
	void addArcPair(Vertex source, Vertex target, int distance);

	void addChargingStation(Vertex& vertex, int cost, int costPerSlot, int capacity);

	std::vector<ChargingStation> findNearbyStations(Vertex vertex);

private:
	Graph network;
};

} /* namespace e4share */

#endif /* STREETNETWORK_H_ */
