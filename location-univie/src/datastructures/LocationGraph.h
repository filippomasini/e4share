/*
 * LocationGraph.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef LOCATIONGRAPH_H_
#define LOCATIONGRAPH_H_

#include "CSLocationInstance.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/optional.hpp>

namespace e4share
{

class LocationGraph
{
public:
	LocationGraph(CSLocationInstance& instance_);

private:
	struct VertexAttr
	{
		unsigned long station;
		int time;
	};

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::optional<VertexAttr>> Graph;
	typedef Graph::vertex_descriptor Vertex;


	CSLocationInstance& instance;
	int stationCount;
	Graph graph;

	Vertex getVertex(unsigned long station, int time)
	{
		return boost::vertex(time * stationCount + station + 1, graph);
	}
};

} /* namespace e4share */

#endif /* LOCATIONGRAPH_H_ */
