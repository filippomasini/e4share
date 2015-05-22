/*
 * LocationGraph.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "datastructures/LocationGraph.h"

namespace e4share
{

LocationGraph::LocationGraph(CSLocationInstance& instance_) :
		instance(instance_),
		stationCount(instance.getNetwork().getCandidateStations().size()),
		graph(),
		tripArcs()
{
	// artificial root vertex, for initial allocation
	Vertex root = boost::add_vertex(graph);

	// create regular vertices
	for(int t = 0; t <= instance.getMaxTime(); t++)
	{
		for(unsigned long i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
		{
			Vertex v = boost::add_vertex(graph);
			graph[v] = {i, t};
		}
	}


	// arcs from root to each t0-vertex
	for(unsigned long i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
	{
		boost::add_edge(root, getVertex(i, 0), graph);
	}

	// waiting arcs
	for(int t = 0; t < instance.getMaxTime(); t++)
	{
		for(unsigned long i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
		{
			boost::add_edge(getVertex(i, t), getVertex(i, t + 1), graph);
		}
	}

	// trip arcs
	auto trips = instance.getTrips();
	auto allStations = instance.getNetwork().getCandidateStations();
	int i = 0;
	for(Trip trip : trips)
	{
		std::vector<Edge> assocEdges;
		auto origin = trip.getOrigin();
		auto destination = trip.getDestination();
		//std::cout << "trip: " << origin << ", " << destination << std::endl;
		std::vector<int> originStations = instance.getNetwork().findNearbyStations(origin);
		std::vector<int> destinationStations = instance.getNetwork().findNearbyStations(destination);

		//std::cout << "stations: " << originStations.size() << ", " << destinationStations.size() << std::endl;

		for(int originstation : originStations)
		{
			for(int destinationstation : destinationStations)
			{
				if(boost::edge(getVertex(originstation, trip.getBeginTime()), getVertex(destinationstation, trip.getEndTime()), graph).second)
				{
					continue;
				}
				//std::cout << originstation << ", " << destinationstation << std::endl;
				auto newEdge = boost::add_edge(getVertex(originstation, trip.getBeginTime()), getVertex(destinationstation, trip.getEndTime()), graph);
				assocEdges.push_back(newEdge.first);
			}
		}
		tripArcs[trip] = assocEdges;
		std::cout << "trip " << i << ": " << assocEdges.size() << " trip arcs" << std::endl;
		std::cout << originStations.size() << " origins (";
		for(auto os : originStations)
		{
			std::cout << os << ", ";
		}
		std::cout << ")" << std::endl;
		std::cout << destinationStations.size() << " destinations (";
		for(auto os : destinationStations)
		{
			std::cout << os << ", ";
		}
		std::cout << ")" << std::endl;
		i++;
	}

	//std::cout << boost::num_vertices(graph) << ", " << boost::num_edges(graph) << std::endl;

}

} /* namespace e4share */
