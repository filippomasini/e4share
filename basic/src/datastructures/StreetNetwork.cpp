/*
 * StreetNetwork.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "StreetNetwork.h"

#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace e4share
{

StreetNetwork::StreetNetwork(int walkingDistance_) :
		network(),
		candidateStations(),
		walkingDistance(walkingDistance_)
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

void StreetNetwork::addChargingStation(Vertex vertex, int cost, int costPerSlot, int capacity)
{
	network[vertex].reset();
	ChargingStation cs(cost, costPerSlot, capacity);
	network[vertex] = cs;
	candidateStations.push_back(cs);
}

std::vector<int> StreetNetwork::findNearbyStations(Vertex vertex) const
{
	//TODO use depth-limited Dijkstra
	std::vector<int> stations;

	std::vector<int> distance(boost::num_vertices(network));
	std::vector<Vertex> pred(boost::num_vertices(network));
	boost::dijkstra_shortest_paths(network, vertex,
			distance_map(boost::make_iterator_property_map(distance.begin(), get(boost::vertex_index, network)))
			.weight_map(get(boost::edge_bundle, network))
			);

	auto vp = vertices(network);
	int i = 0;
	for(auto v = vp.first; v != vp.second; v++)
	{

		//if(distance[*v] < 3)
		if(network[*v].is_initialized())
		{
			//if(network[*v].is_initialized())// != boost::none)
			if(distance[*v] <= walkingDistance)
			{
				//stations.push_back(*network[*v]);
				stations.push_back(i);
			}
			i++;
		}
	}

	return stations;
}

} /* namespace e4share */
