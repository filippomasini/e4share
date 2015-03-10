/*
 * StreetNetwork.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "StreetNetwork.h"

#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace e4share
{

StreetNetwork::StreetNetwork() :
		network()
{

}

StreetNetwork::Vertex StreetNetwork::addVertex()
{
	return boost::add_vertex(network);
}

void StreetNetwork::addArc(Vertex source, Vertex target, int distance)
{
	Edge edge = boost::add_edge(source, target, network).first;
	network[edge] = distance;
}

void StreetNetwork::addArcPair(Vertex source, Vertex target, int distance)
{
	addArc(source, target, distance);
	addArc(target, source, distance);
}

void StreetNetwork::addChargingStation(Vertex& vertex, int cost, int costPerSlot, int capacity)
{
	network[vertex].reset();
	ChargingStation cs(cost, costPerSlot, capacity);
	network[vertex] = cs;
	candidateStations.push_back(cs);
}

std::vector<ChargingStation> StreetNetwork::findNearbyStations(Vertex vertex)
{
	//TODO use depth-limited Dijkstra
	std::vector<ChargingStation> stations;

	std::vector<int> distance;
	boost::dijkstra_shortest_paths(network, vertex, distance_map(boost::make_iterator_property_map(distance.begin(),
			get(boost::vertex_index, network))) .weight_map(get(boost::edge_bundle, network)));

	auto vp = vertices(network);
	for(auto v = vp.first; v != vp.second; v++)
	{
		if(distance[*v] < 3)
		{
			if(network[*v].is_initialized())// != boost::none)
			{
				stations.push_back(*network[*v]);
			}
		}
	}

	return stations;
}

} /* namespace e4share */
