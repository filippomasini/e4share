/*
 * BatteryGraph.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "datastructures/BatteryGraph.h"

namespace e4share
{

BatteryGraph::BatteryGraph(CSLocationInstance instance_) :
		instance(instance_),
		graph()
{
	// FIXME must currently be a multiple of stateskip
	int recharge = 10;

	Vertex root = boost::add_vertex(graph);
	graph[root] = {100, 0};

	// create regular vertices
	for(int t = 1; t <= instance.getMaxTime(); t++)
	{
		for(int i = 0; i <= 100; i += stateskip)
		{
			Vertex v = boost::add_vertex(graph);
			graph[v] = {i, t};
		}
	}

	// charge arcs
	boost::add_edge(root, getVertex(100, 1), graph);
	for(int t = 1; t < instance.getMaxTime(); t++)
	{
		for(int i = 0; i <= 100; i += stateskip)
		{
			// full charge arcs
			if(i >= 100 - recharge)
			{
				boost::add_edge(getVertex(i, t), getVertex(100, t+1), graph);
			}
			// recharge arcs
			else
			{
				boost::add_edge(getVertex(i, t), getVertex(i + recharge, t+1), graph);
			}
		}
	}

	// trip arcs
	auto trips = instance.getTrips();
	for(Trip trip : trips)
	{
		auto origin = trip.getOrigin();
		auto destination = trip.getDestination();
		//std::cout << "trip: " << origin << ", " << destination << std::endl;
		int consumptionInPercent = (int)(trip.getBatteryConsumption() * 100);
		// round up to nearest stateskip
		int roundedConsumption = consumptionInPercent;
		if(roundedConsumption % stateskip != 0)
		{
			roundedConsumption += stateskip - roundedConsumption % stateskip;
		}
		std::cout << consumptionInPercent << ", " << roundedConsumption << std::endl;

		for(int i = 100; i >= roundedConsumption; i -= stateskip)
		{
			if(trip.getBeginTime() == 0  &&  i < 100)
			{
				continue;
			}
			if(boost::edge(getVertex(i, trip.getBeginTime()), getVertex(i - roundedConsumption, trip.getEndTime()), graph).second)
			{
				continue;
			}
			boost::add_edge(getVertex(i, trip.getBeginTime()), getVertex(i - roundedConsumption, trip.getEndTime()), graph);
		}
	}

	// TODO debug output
	auto foo = edges(graph);
	for(foo.first; foo.first != foo.second; foo.first++)
	{
		auto from = boost::source(*foo.first, graph);
		auto to = boost::target(*foo.first, graph);
		//std::cout << "from t" << graph[from]->time << "@" << graph[from]->chargestate << " to t" << graph[to]->time << "@" << graph[to]->chargestate << std::endl;
	}
}

} /* namespace e4share */
