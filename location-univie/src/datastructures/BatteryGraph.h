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

private:
	struct VertexAttr
	{
		int chargestate;
		int time;
	};

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::optional<VertexAttr>> Graph;
	typedef Graph::vertex_descriptor Vertex;

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
