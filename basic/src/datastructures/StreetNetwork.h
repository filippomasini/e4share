/*
 * StreetNetwork.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef STREETNETWORK_H_
#define STREETNETWORK_H_

#include <vector>

#include <boost/graph/adjacency_list.hpp>

namespace e4share
{

class StreetNetwork
{


public:
	typedef boost::adjacency_list<> Graph;
	typedef Graph::vertex_descriptor Vertex;

	StreetNetwork();
	Vertex addVertex();
	void addArc(Vertex source, Vertex target, int distance);
	void addArcPair(Vertex source, Vertex target, int distance);

	std::vector<Vertex> getNeighbourhood(Vertex vertex);

private:
	Graph network;
};

} /* namespace e4share */

#endif /* STREETNETWORK_H_ */
