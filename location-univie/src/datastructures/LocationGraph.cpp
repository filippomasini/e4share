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
		graph()
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
	for(Trip trip : trips)
	{
		auto origin = trip.getOrigin();
		auto destination = trip.getDestination();
		std::cout << "trip: " << origin << ", " << destination << std::endl;
		std::vector<int> originStations = instance.getNetwork().findNearbyStations(origin);
		std::vector<int> destinationStations = instance.getNetwork().findNearbyStations(destination);

		for(int originstation : originStations)
		{
			for(int destinationstation : destinationStations)
			{
				std::cout << originstation << ", " << destinationstation << std::endl;
				boost::add_edge(getVertex(originstation, trip.getBeginTime()), getVertex(destinationstation, trip.getEndTime()), graph);
			}
		}
	}

	std::cout << boost::num_vertices(graph) << ", " << boost::num_edges(graph) << std::endl;

}

} /* namespace e4share */
