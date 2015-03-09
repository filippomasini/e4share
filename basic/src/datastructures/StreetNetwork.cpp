/*
 * StreetNetwork.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "StreetNetwork.h"

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
	boost::add_edge(source, target, network);
}

void StreetNetwork::addArcPair(Vertex source, Vertex target, int distance)
{
	addArc(source, target, distance);
	addArc(target, source, distance);
}

} /* namespace e4share */
