/*
 * LocationGraph.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "datastructures/LocationGraph.h"

#include <fstream>
#include <iostream>
#include <sstream>

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


	// draw location graph
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();
	int locationEdgeCount = edgeCount();
	std::map<LocationGraph::Edge, int> locEdgeIndex;
	int edgeIndex = 0;
	for(LocationGraph::Edge e : allEdges())
	{
		locEdgeIndex[e] = edgeIndex;
		edgeIndex++;
	}

	std::ofstream tikzfile;
	tikzfile.open("layeredGraph.tex");
	tikzfile << "\\documentclass{article}" << std::endl;
	tikzfile << "\\usepackage[landscape]{geometry}" << std::endl;
	tikzfile << "\\usepackage{tikz}" << std::endl;
	tikzfile << "\\begin{document}" << std::endl;
	tikzfile << "\\pagenumbering{gobble}" << std::endl << std::endl;

	for(int c = 0; c < 1; c++)
	{
		// check which stations are visited by car c
		int stationsVisited = 0;
		std::vector<bool> stationUsed(stationCount);
		for(int i = 0; i < stationCount; i++)
		{
			stationUsed[i] = false;
			for(int t = 0; t <= instance.getMaxTime(); t++)
			{
				auto inEdges = incomingEdges(i, t);
				for(auto edge : inEdges)
				{
					stationUsed[i] = true;
				}
			}
		}

		for(int i = 0; i < stationCount; i++)
		{
			if(stationUsed[i])
			{
				stationsVisited++;
			}
		}

//		auto target = edge.m_target;
//						int station = (target - 1) % stationCount;
//						int time = (target - 1) / stationCount;

		tikzfile << "\\begin{figure}" << std::endl;
		tikzfile << "\\centering" << std::endl;
		tikzfile << "\\begin{tikzpicture}" << std::endl;

		//tikzfile << (stationsVisited - 1) / 2 << std::endl;
		//tikzfile << ((stationsVisited - 1) / 2) << std::endl;
		tikzfile << "\\node (r) at (" << (stationsVisited - 1) / 2.0 << ", 1) {$r$};" << std::endl;

		// draw vertices
		int xcounter = 0;
		for(int i = 0; i < stationCount; i++)
		{
			if(stationUsed[i])
			{
				for(int t = 0; t <= instance.getMaxTime(); t++)
				{
					tikzfile << "\\node (" << i << "-" << t << ") at (" << xcounter << ", -" << t << ") {" << i << "};" << std::endl;
				}
				xcounter++;
			}
		}

		// draw arcs
		tikzfile << "\\draw" << std::endl;
		for(int i = 0; i < stationCount; i++)
		{
			if(stationUsed[i])
			{
				for(int t = 0; t <= instance.getMaxTime(); t++)
				{
					auto inEdges = incomingEdges(i, t);
					for(auto edge : inEdges)
					{
						std::string source = "r";
						if(edge.m_source > 0)
						{
							int stat = (edge.m_source - 1) % stationCount;
							int time = (edge.m_source - 1) / stationCount;
							std::stringstream temp;
							temp << stat << "-" << time;
							source = temp.str();
						}
						tikzfile << "(" << source << ") -- (" << i << "-" << t << ") node [midway, above, font = \\tiny] {}" << std::endl;
					}
				}
			}
		}
		tikzfile << ";" << std::endl;

		tikzfile << "\\end{tikzpicture}" << std::endl;
		tikzfile << "\\end{figure}" << std::endl;
		tikzfile << "\\clearpage" << std::endl << std::endl;
	}
	tikzfile << "\\end{document}" << std::endl;
	tikzfile.close();

}

} /* namespace e4share */
