/*
 * BatteryGraph.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef BATTERYGRAPH_H_
#define BATTERYGRAPH_H_

#include "CSLocationInstance.h"

namespace e4share
{

class BatteryGraph
{
public:
	BatteryGraph(CSLocationInstance instance_);

	struct VertexAttr
	{
		int chargestate;
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

	std::vector<Edge> incomingEdges(int chargestate, int time)
	{
		std::vector<Edge> incomingEdges;
		auto iters = boost::in_edges(getVertex(chargestate, time), graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			incomingEdges.push_back(*iter);
		}
		return incomingEdges;
	}

	std::vector<Edge> outgoingEdges(int chargestate, int time)
	{
		std::vector<Edge> outgoingEdges;
		auto iters = boost::out_edges(getVertex(chargestate, time), graph);
		for(auto iter = iters.first; iter != iters.second; iter++)
		{
			outgoingEdges.push_back(*iter);
		}
		return outgoingEdges;
	}

	std::vector<Edge> tripArcs(Trip trip)
	{
		std::vector<Edge> arcs;
		//int consumptionInPercent = (int)(trip.getBatteryConsumption() * 100);
		int consumptionInPercent =trip.getBatteryConsumption();
		// round up to nearest stateskip
		int roundedConsumption = consumptionInPercent;
		if(roundedConsumption % stateskip != 0)
		{
			roundedConsumption += stateskip - roundedConsumption % stateskip;
		}

		for(int i = 100; i >= roundedConsumption; i -= stateskip)
		{
			if(trip.getBeginTime() == 0  &&  i < 100)
			{
				continue;
			}
			arcs.push_back(boost::edge(getVertex(i, trip.getBeginTime()), getVertex(i - roundedConsumption, trip.getEndTime()), graph).first);
		}

		return arcs;
	}

private:


	CSLocationInstance instance;
	Graph graph;
	int stateskip = 5;

	Vertex getVertex(int chargestate, int time)
	{
		return boost::vertex(((time - 1) * (100 / stateskip + 1)) + (chargestate / stateskip) + 1, graph);
	}


};

} /* namespace e4share */

#endif /* BATTERYGRAPH_H_ */
