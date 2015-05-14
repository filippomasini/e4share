/*
 * LocationGraph.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef LOCATIONGRAPH_H_
#define LOCATIONGRAPH_H_

#include "CSLocationInstance.h"

#include <vector>
#include <map>
#include <unordered_map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/optional.hpp>

namespace e4share
{

class LocationGraph
{
public:
	LocationGraph(CSLocationInstance& instance_);

	struct VertexAttr
	{
		unsigned long station;
		int time;
	};

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::optional<VertexAttr>> Graph;
	typedef Graph::vertex_descriptor Vertex;
	typedef Graph::edge_descriptor Edge;

	int edgeCount() const
	{
		return boost::num_edges(graph);
	}

	std::vector<Edge> allEdges()
	{
		std::vector<Edge> allEdges;
		auto iters = boost::edges(graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			allEdges.push_back(*iter);
		}
		return allEdges;
	}

	Edge getWaitingArc(unsigned long station, int time)
	{
		Vertex v = getVertex(station, time);
		Vertex v2 = getVertex(station, time + 1);
		return boost::edge(v, v2, graph).first;
	}

	std::vector<Edge> incomingEdges(unsigned long station, int time)
	{
		std::vector<Edge> incomingEdges;
		auto iters = boost::in_edges(getVertex(station, time), graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			incomingEdges.push_back(*iter);
		}
		return incomingEdges;
	}

	std::vector<Edge> outgoingEdges(unsigned long station, int time)
	{
		std::vector<Edge> outgoingEdges;
		auto iters = boost::out_edges(getVertex(station, time), graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			outgoingEdges.push_back(*iter);
		}
		return outgoingEdges;
	}

	std::vector<Edge> outgoingEdges()
	{
		std::vector<Edge> outgoingEdges;
		auto iters = boost::out_edges(0, graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			outgoingEdges.push_back(*iter);
		}
		return outgoingEdges;
	}

	Edge rootEdge(unsigned long station)
	{
		Vertex root = boost::vertex(0, graph);
		Vertex target = getVertex(station, 0);
		return boost::edge(root, target, graph).first;
	}

	std::vector<Edge> tripArcsFrom(Trip trip, unsigned long station)
	{
		std::vector<Edge> arcs;
		auto dStations = instance.getNetwork().findNearbyStations(trip.getDestination());
		for(int cs : dStations)
		{
			arcs.push_back(boost::edge(getVertex(station, trip.getBeginTime()), getVertex(cs, trip.getEndTime()), graph).first);
		}
		return arcs;
	}

	std::vector<Edge> tripArcsTo(Trip trip, unsigned long station)
	{
		std::vector<Edge> arcs;
		auto oStations = instance.getNetwork().findNearbyStations(trip.getOrigin());
		for(int cs : oStations)
		{
			arcs.push_back(boost::edge(getVertex(cs, trip.getBeginTime()), getVertex(station, trip.getEndTime()), graph).first);
		}
		return arcs;
	}

	std::vector<Edge> tripArcsOf(Trip trip)
	{
		return tripArcs[trip];
	}

private:



	CSLocationInstance& instance;
	int stationCount;
	Graph graph;
	std::map<Trip, std::vector<Edge>> tripArcs;

	Vertex getVertex(unsigned long station, int time)
	{
		return boost::vertex(time * stationCount + station + 1, graph);
	}


};

} /* namespace e4share */

#endif /* LOCATIONGRAPH_H_ */
