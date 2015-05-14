/*
 * TwoLayeredGraphsILP.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "ilp/TwoLayeredGraphsILP.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>

#include <fstream>
#include <iostream>

namespace e4share
{

TwoLayeredGraphsILP::TwoLayeredGraphsILP(CSLocationInstance instance_, int budget_, bool useBatteryGraph_) :
		instance(instance_),
		batteryGraph(instance),
		locationGraph(instance),
		budget(budget_),
		useBatteryGraph(useBatteryGraph_),
		env(),
		model(env)
{
	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	// stations
	//y = IloBoolVarArray(env, stationCount);
	y = IloNumVarArray(env, stationCount);
	//s = IloIntVarArray(env, stationCount);
	s = IloNumVarArray(env, stationCount);
	for(int i = 0; i < stationCount; i++)
	{
		y[i] = IloBoolVar(env);
		y[i].setBounds(0, 1);
//		y[i] = IloNumVar(env);
//		model.add(y[i] >= 0);
//		model.add(y[i] <= 1);
		s[i] = IloIntVar(env);
		s[i] = IloNumVar(env);
	}

	// trips and trip assignment
//	lambda = IloBoolVarArray (env, tripCount);
//	x = IloBoolVarArray(env, stationCount * tripCount);
//	z = IloBoolVarArray(env, stationCount * tripCount);
//	a = IloBoolVarArray(env, carCount * tripCount);
//	xc = IloBoolVarArray(env, carCount * stationCount * tripCount);
//	zc = IloBoolVarArray(env, carCount * stationCount * tripCount);

	lambda = IloNumVarArray (env, tripCount);
	x = IloNumVarArray(env, stationCount * tripCount);
	z = IloNumVarArray(env, stationCount * tripCount);
	a = IloNumVarArray(env, carCount * tripCount);
	xc = IloNumVarArray(env, carCount * stationCount * tripCount);
	zc = IloNumVarArray(env, carCount * stationCount * tripCount);

	for(int k = 0; k < tripCount; k++)
	{
		lambda[k] = IloBoolVar(env);
//		lambda[k] = IloNumVar(env);
//		model.add(lambda[k] >= 0);
//		model.add(lambda[k] <= 1);

		for(int i = 0; i < stationCount; i++)
		{
			int index = k * stationCount + i;
			x[index] = IloBoolVar(env);
//			x[index] = IloNumVar(env);
//			model.add(x[index] >= 0);
//			model.add(x[index] <= 1);
			z[index] = IloBoolVar(env);
//			z[index] = IloNumVar(env);
//			model.add(z[index] >= 0);
//			model.add(z[index] <= 1);
		}

		for(int c = 0; c < carCount; c++)
		{
			int index = k * carCount + c;
			a[index] = IloBoolVar(env);
//			a[index] = IloNumVar(env);
//			model.add(a[index] >= 0);
//			model.add(a[index] <= 1);

			for(int i = 0; i < stationCount; i++)
			{
				int index2 = k * stationCount * carCount + c * stationCount + i;
				xc[index2] = IloBoolVar(env);
//				xc[index2] = IloNumVar(env);
//				model.add(xc[index2] >= 0);
//				model.add(xc[index2] <= 1);
				zc[index2] = IloBoolVar(env);
//				zc[index2] = IloNumVar(env);
//				model.add(zc[index2] >= 0);
//				model.add(zc[index2] <= 1);
			}
		}
	}

	// mapping edges to edge-indices
	std::map<LocationGraph::Edge, int> locEdgeIndex;
	int edgeIndex = 0;
	for(LocationGraph::Edge e : locationGraph.allEdges())
	{
		locEdgeIndex[e] = edgeIndex;
		edgeIndex++;
	}

	edgeIndex = 0;
	std::map<BatteryGraph::Edge, int> batEdgeIndex;
	for(BatteryGraph::Edge e : batteryGraph.allEdges())
	{
		batEdgeIndex[e] = edgeIndex;
		edgeIndex++;
	}

	// flow in location graph
	int locationEdgeCount = locationGraph.edgeCount();
	//f = IloBoolVarArray(env, carCount * locationEdgeCount);
	f = IloNumVarArray(env, carCount * locationEdgeCount);

	// flow in battery graph
	int batteryEdgeCount = batteryGraph.edgeCount();
	int timeslots = instance.getMaxTime() + 1;
	if(useBatteryGraph)
	{
		//g = IloBoolVarArray(env, carCount * batteryEdgeCount);
		g = IloNumVarArray(env, carCount * batteryEdgeCount);
	}
	else
	{
		g1 = IloNumVarArray(env, carCount * timeslots);
	}

	for(int c = 0; c < carCount; c++)
	{
		for(int e = 0; e < locationEdgeCount; e++)
		{
			f[c * locationEdgeCount + e] = IloBoolVar(env);
//			f[c * locationEdgeCount + e] = IloNumVar(env);
//			model.add(f[c * locationEdgeCount + e] >= 0);
//			model.add(f[c * locationEdgeCount + e] <= 1);
		}

		if(useBatteryGraph)
		{
			for(int e = 0; e < batteryEdgeCount; e++)
			{
				g[c * batteryEdgeCount + e] = IloBoolVar(env);
//				g[c * batteryEdgeCount + e] = IloNumVar(env);
//				model.add(g[c * batteryEdgeCount + e] >= 0);
//				model.add(g[c * batteryEdgeCount + e] <= 1);
			}
		}
		else
		{
			for(int e = 0; e < timeslots; e++)
			{
				g1[c * timeslots + e] = IloNumVar(env);
			}
		}

	}


	// objective function
	IloExpr obj(env);
	for(int k = 0; k < instance.getTrips().size(); k++)
	{
		obj += instance.getTrips()[k].getProfit() * lambda[k];
		std::cout << instance.getTrips()[k].getProfit() << std::endl;
	}
	model.add(IloMaximize(env, obj));
	obj.end();

	// budget constraint
	IloExpr budgetConstraint(env);
	for(int i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
	{
		budgetConstraint += instance.getNetwork().getCandidateStations()[i].getCost() * y[i];
		budgetConstraint += instance.getNetwork().getCandidateStations()[i].getCostPerSlot() * s[i];
	}
	model.add(budgetConstraint <= budget);
	budgetConstraint.end();

	// station capacity
	for(int i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
	{
		model.add(s[i] <= instance.getNetwork().getCandidateStations()[i].getCapacity() * y[i]);
		model.add(s[i] >= y[i]);
	}


	// assigning car and start/end station to each trip
	for(int k = 0; k < instance.getTrips().size(); k++)
	{
		// assign car
		IloExpr assignedCars(env);
		for(int c = 0; c < instance.getCarCount(); c++)
		{
			assignedCars += a[k * instance.getCarCount() + c];
		}
		model.add(assignedCars == lambda[k]);
		assignedCars.end();

		// assign start station
		IloExpr startStations(env);
		IloExpr endStations(env);
		auto startCandidates = instance.getNetwork().findNearbyStations(instance.getTrips()[k].getOrigin());
		auto endCandidates = instance.getNetwork().findNearbyStations(instance.getTrips()[k].getDestination());
		for(int stationIndex : startCandidates)
		{
			startStations += x[k * instance.getNetwork().getCandidateStations().size() + stationIndex];
		}
		for(int stationIndex : endCandidates)
		{
			endStations += z[k * instance.getNetwork().getCandidateStations().size() + stationIndex];
		}
		model.add(startStations == lambda[k]);
		model.add(endStations == lambda[k]);
		startStations.end();
		endStations.end();

		// disallow stations that are not candidates
		IloExpr nonStartStations(env);
		IloExpr nonEndStations(env);
		for(int i = 0; i < stationCount; i++)
		{
			if(std::count(startCandidates.begin(), startCandidates.end(), i) == 0)
			{
				nonStartStations += x[k * instance.getNetwork().getCandidateStations().size() + i];
			}
			if(std::count(endCandidates.begin(), endCandidates.end(), i) == 0)
			{
				nonStartStations += z[k * instance.getNetwork().getCandidateStations().size() + i];
			}
		}
		model.add(nonStartStations == 0);
		model.add(nonEndStations == 0);
		nonStartStations.end();
		nonEndStations.end();

	}

	// linking a and x/z variables
	for(int k = 0; k < instance.getTrips().size(); k++)
	{
		for(int c = 0; c < instance.getCarCount(); c++)
		{
			for(int i = 0; i < instance.getNetwork().getCandidateStations().size(); i++)
			{
				int index = k * stationCount * carCount + c * stationCount + i;
				model.add(xc[index] <= a[k * carCount + c]);
				model.add(xc[index] <= x[k * stationCount + i]);
				model.add(xc[index] >= a[k * carCount + c] + x[k * stationCount + i] - 1);

				model.add(zc[index] <= a[k * carCount + c]);
				model.add(zc[index] <= z[k * stationCount + i]);
				model.add(zc[index] >= a[k * carCount + c] + z[k * stationCount + i] - 1);
			}
		}
	}


	// LOCATION GRAPH FLOW
	// capacity constraint on flow
	for(int i = 0; i < stationCount; i++)
	{
		for(int t = 0; t < instance.getMaxTime(); t++)
		{
			LocationGraph::Edge waitingEdge = locationGraph.getWaitingArc(i, t);
			int waitingEdgeIndex = locEdgeIndex[waitingEdge];
			IloExpr waitingCars(env);
			for(int c = 0; c < carCount; c++)
			{
				waitingCars += f[c * locationEdgeCount + waitingEdgeIndex];
			}
			model.add(waitingCars <= s[i]);
			waitingCars.end();
		}
	}

	// only use opened stations
	for(int i = 0; i < stationCount; i++)
	{
		for(int t = 0; t <= instance.getMaxTime(); t++)
		{
			auto inEdges = locationGraph.incomingEdges(i, t);
			for(auto edge : inEdges)
			{
				for(int c = 0; c < carCount; c++)
				{
					model.add(f[c * locationEdgeCount + locEdgeIndex[edge]] <= y[i]);
				}
			}
		}
	}

	// only use selected cars
	for(int c = 0; c < carCount; c++)
	{
		IloExpr carTrips(env);
		IloExpr rootFlow(env);
		for(int k = 0; k < tripCount; k++)
		{
			carTrips += a[k * carCount + c];
		}
		auto rootEdges = locationGraph.outgoingEdges();
		for(auto edge: rootEdges)
		{
			rootFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
		}
		model.add(rootFlow <= carTrips);
		model.add(rootFlow <= 1);
		carTrips.end();
		rootFlow.end();
	}

	// flow conservation
	for(int c = 0; c < carCount; c++)
	{
		for(int i = 0; i < stationCount; i++)
		{
			for(int t = 0; t < instance.getMaxTime(); t++)
			{
				auto inEdges = locationGraph.incomingEdges(i, t);
				auto outEdges = locationGraph.outgoingEdges(i, t);
				IloExpr inFlow(env);
				IloExpr outFlow(env);
				for(auto edge : inEdges)
				{
					inFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
				}
				for(auto edge : outEdges)
				{
					outFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
				}
				model.add(inFlow == outFlow);
				inFlow.end();
				outFlow.end();
			}
		}
	}

	// flow for covered trips
	for(int k = 0; k < tripCount; k++)
	{
		for(int i = 0; i < stationCount; i++)
		{
			auto arcsToDestinations = locationGraph.tripArcsFrom(instance.getTrips()[k], i);
			auto arcsFromOrigins = locationGraph.tripArcsTo(instance.getTrips()[k], i);
			for(int c = 0; c < carCount; c++)
			{
				IloExpr flowToDestinations(env);
				for(auto arc : arcsToDestinations)
				{
					flowToDestinations += f[c * locationEdgeCount + locEdgeIndex[arc]];
				}
				//model.add(flowToDestinations == xc[k * stationCount * carCount + c * stationCount + i]);
				flowToDestinations.end();

				IloExpr flowFromOrigins(env);
				for(auto arc : arcsFromOrigins)
				{
					flowFromOrigins += f[c * locationEdgeCount + locEdgeIndex[arc]];
				}
				//model.add(flowFromOrigins == zc[k * stationCount * carCount + c * stationCount + i]);
				flowFromOrigins.end();
			}
		}
	}


	// BATTERY GRAPH FLOW
	// only use selected cars

	if(useBatteryGraph)
	{
		for(int c = 0; c < carCount; c++)
		{
			IloExpr carTrips(env);
			IloExpr rootFlow(env);
			for(int k = 0; k < tripCount; k++)
			{
				carTrips += a[k * carCount + c];
			}
			auto rootEdges = batteryGraph.outgoingEdges();
			for(auto edge: rootEdges)
			{
				rootFlow += g[c * batteryEdgeCount + batEdgeIndex[edge]];
			}
			//model.add(rootFlow <= carTrips);
			model.add(rootFlow == 1);
			carTrips.end();
			rootFlow.end();
		}

		// flow conservation
		for(int c = 0; c < carCount; c++)
		{
			for(int charge = 0; charge <= 100; charge += 5)
			{
				for(int t = 1; t < instance.getMaxTime(); t++)
				{
					auto inEdges = batteryGraph.incomingEdges(charge, t);
					auto outEdges = batteryGraph.outgoingEdges(charge, t);
					IloExpr inFlow(env);
					IloExpr outFlow(env);
					for(auto edge : inEdges)
					{
						inFlow += g[c * batteryEdgeCount + batEdgeIndex[edge]];
					}
					for(auto edge : outEdges)
					{
						outFlow += g[c * batteryEdgeCount + batEdgeIndex[edge]];
					}
					model.add(inFlow == outFlow);
					inFlow.end();
					outFlow.end();
				}
			}
		}

		// flow for covered trips
		for(int k = 0; k < tripCount; k++)
		{
			auto tripArcs = batteryGraph.tripArcs(instance.getTrips()[k]);
			std::cout << tripArcs.size() << " trip arcs for trip " << k << std::endl;

			for(int c = 0; c < carCount; c++)
			{
				IloExpr flow(env);

				for(auto arc : tripArcs)
				{
					flow += g[c * batteryEdgeCount + batEdgeIndex[arc]];
				}
				model.add(flow == a[k * carCount + c]);
				//model.add(flow == 0);
				flow.end();
			}
		}
	}
	// alternative battery state tracking
	else
	{
		// limits
		for(int c = 0; c < carCount; c++)
		{
			for(int t = 0; t < timeslots; t++)
			{
				model.add(g1[c * timeslots + t] >= 0);
				model.add(g1[c * timeslots + t] <= 1);
			}
			//model.add(g1[c * timeslots] == 1);
		}

		// recharging
		for(int c = 0; c < carCount; c++)
		{
			for(int t = 0; t < timeslots - 1; t++)
			{
				model.add(g1[c * timeslots + (t+1)] <= g1[c * timeslots + t] + 0.1);
			}
		}

		// depleting for trip
		for(int c = 0; c < carCount; c++)
		{
			for(int k = 0; k < tripCount; k++)
			{
				auto trip = instance.getTrips()[k];
				double roundedConsumption = (ceil(trip.getBatteryConsumption() * 20)) / (double)20;
				std::cout << roundedConsumption << ", " << trip.getBatteryConsumption() << std::endl;
				IloExpr rhs = g1[c * timeslots + trip.getBeginTime()] -
						roundedConsumption * a[k * carCount + c] +
						(1 - a[k * carCount + c]) * (trip.getEndTime() - trip.getBeginTime()) * 0.1;
				model.add(g1[c * timeslots + trip.getEndTime()] <= rhs);
				rhs.end();
			}
		}
	}


	//addModel();
}

void TwoLayeredGraphsILP::addModel()
{

}

void TwoLayeredGraphsILP::solve()
{
	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	cplex = IloCplex(model);
	cplex.solve();
	std::cout << "obj: " << cplex.getObjValue() << std::endl;
	std::cout << "CPLEX status: " << cplex.getStatus() << std::endl;

	auto yvals = IloNumArray(env);
	auto svals = IloNumArray(env);

	cplex.getValues(yvals, y);
	cplex.getValues(svals, s);

	// trips and trip assignment
	auto lambdavals = IloNumArray(env);
	auto xvals = IloNumArray(env);
	auto zvals = IloNumArray(env);
	auto avals = IloNumArray(env);
	auto xcvals = IloNumArray(env);
	auto zcvals = IloNumArray(env);
	auto fvals = IloNumArray(env);
	auto gvals = IloNumArray(env);
	auto g1vals = IloNumArray(env);

	cplex.getValues(lambdavals, lambda);
	cplex.getValues(xvals, x);
	cplex.getValues(zvals, z);
	cplex.getValues(avals, a);
	cplex.getValues(xcvals, xc);
	cplex.getValues(zcvals, zc);
	cplex.getValues(fvals, f);
	if(useBatteryGraph)
	{
		cplex.getValues(gvals, g);
	}
	else
	{
		cplex.getValues(g1vals, g1);
	}

	for(int i = 0; i < stationCount; i++)
	{
		std::cout << "y[" << i << "]: " << yvals[i] << ", " << "s[" << i << "]: " << svals[i] << std::endl;
	}

	for(int k = 0; k < tripCount; k++)
	{
		std::cout << "lambda[" << k << "]: " << lambdavals[k] << std::endl;

		if(lambdavals[k] > 0)
		{
			/*std::cout << " candidate starts: ";
			for(auto station : instance.getNetwork().findNearbyStations(instance.getTrips()[k].getOrigin()))
			{
				std::cout << station << ", ";
			}
			std::cout << std::endl;

			std::cout << " candidate ends: ";
			for(auto station : instance.getNetwork().findNearbyStations(instance.getTrips()[k].getDestination()))
			{
				std::cout << station << ", ";
			}
			std::cout << std::endl;*/

			for(int i = 0; i < stationCount; i++)
			{
				int index = k * stationCount + i;
				if(xvals[index] > 0.50)
				{
					std::cout << " startstation " << i << std::endl;
				}
				/*else if(xvals[index] != 0)
				{
					std::cout << " startstation " << i << ": " << xvals[index] << std::endl;
				}*/
				if(zvals[index] > 0)
				{
					std::cout << " endstation " << i << std::endl;
				}
				/*else if(zvals[index] != 0)
				{
					std::cout << " endstation " << i << ": " << zvals[index] << std::endl;
				}*/
			}

			for(int c = 0; c < carCount; c++)
			{
				int index = k * carCount + c;
				if(avals[index] > 0)
				{
					std::cout << " car " << c << std::endl;
				}
	//			std::cout << "a[" << index << "]: " << avals[index] << std::endl;

				for(int i = 0; i < stationCount; i++)
				{
					int index2 = k * stationCount * carCount + c * stationCount + i;
					if(xcvals[index2] > 0)
					{
						std::cout << " xc(k=" << k << ",i=" << i << ",c=" << c << ")  == " << xcvals[index2] << std::endl;
					}
					if(zcvals[index2] > 0)
					{
						std::cout << " zc(k=" << k << ",i=" << i << ",c=" << c << ")  == " << zcvals[index2] << std::endl;
					}
					//std::cout << "xc[" << index2 << "]: " << xcvals[index2] << std::endl;
					//std::cout << "zc[" << index2 << "]: " << zcvals[index2] << std::endl;
				}
			}
		}
	}

	std::cout << std::endl;
	auto edges = locationGraph.allEdges();
	auto batteryEdges = batteryGraph.allEdges();
	for(int c = 0; c < carCount; c++)
	{
		int assignedTrips = 0;
		for(int k = 0; k < tripCount; k++)
		{
			assignedTrips += avals[k * carCount + c];
		}

		// if car is used
		if(assignedTrips > 0)
		{
			std::cout << "car " << c << " with " << assignedTrips << " assigned trips: ";
			for(int k = 0; k < tripCount; k++)
			{
				if(avals[k * carCount + c] > 0)
				{
					std::cout << k << ", ";
				}
			}
			std::cout << std::endl;
			std::set<LocationGraph::Edge, std::function<bool (LocationGraph::Edge, LocationGraph::Edge)>> usedEdges
					(
							[](LocationGraph::Edge e1, LocationGraph::Edge e2)
							{
								if(e1.m_source == e2.m_source)
								{
									return e1.m_target < e2.m_target;
								}
								else
								{
									return e1.m_source < e2.m_source;
								}
							}
					);
			int e = 0;
			for(auto edge : edges)
			{
				if(fvals[c * edges.size() + e] > 0)
				{
					usedEdges.insert(edge);
				}
				e++;
			}
			//std::cout << " ";
			for(auto edge : usedEdges)
			{
				auto target = edge.m_target;
				int station = (target - 1) % stationCount;
				int time = (target - 1) / stationCount;
				//std::cout << station << "@" << time << " --> ";
				std::cout << " " << edge << ", " << station << ", " << time << std::endl;
			}
			std::cout << "--------" << std::endl;

			// battery graph
			if(useBatteryGraph)
			{
				std::set<BatteryGraph::Edge, std::function<bool (BatteryGraph::Edge, BatteryGraph::Edge)>> usedBatteryEdges
						(
								[](BatteryGraph::Edge e1, BatteryGraph::Edge e2)
								{
									if(e1.m_source == e2.m_source)
									{
										return e1.m_target < e2.m_target;
									}
									else
									{
										return e1.m_source < e2.m_source;
									}
								}
						);
				e = 0;
				for(auto edge : batteryEdges)
				{
					if(gvals[c * batteryEdges.size() + e] > 0)
					{
						usedBatteryEdges.insert(edge);
					}
					e++;
				}
				//std::cout << " ";
				for(auto edge : usedBatteryEdges)
				{
					auto target = edge.m_target;
					int charge = (target - 1) % 21;
					int time = ((target - 1) / 21) + 1;
					//std::cout << station << "@" << time << " --> ";
					std::cout << " " << edge << ", " << charge << ", " << time << std::endl;
				}
				std::cout << std::endl;
			}
			// alternative tracking
			else
			{
				int timeslots = instance.getMaxTime() + 1;
				for(int t = 0; t < timeslots; t++)
				{
					std::cout << " t" << t << ": " << g1vals[c * timeslots + t] << std::endl;
				}
			}

		}
	}

	drawSolutionTikz("solution.tex", fvals);
}

void TwoLayeredGraphsILP::drawSolutionTikz(std::string filename, IloNumArray fvals)
{
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();
	int locationEdgeCount = locationGraph.edgeCount();
	std::map<LocationGraph::Edge, int> locEdgeIndex;
	int edgeIndex = 0;
	for(LocationGraph::Edge e : locationGraph.allEdges())
	{
		locEdgeIndex[e] = edgeIndex;
		edgeIndex++;
	}

	std::ofstream tikzfile;
	tikzfile.open(filename);
	tikzfile << "\\documentclass{article}" << std::endl;
	tikzfile << "\\usepackage[landscape]{geometry}" << std::endl;
	tikzfile << "\\usepackage{tikz}" << std::endl;
	tikzfile << "\\begin{document}" << std::endl << std::endl;

	for(int c = 0; c < carCount; c++)
	{
		// check which stations are visited by car c
		int stationsVisited = 0;
		std::vector<bool> stationUsed(stationCount);
		for(int i = 0; i < stationCount; i++)
		{
			stationUsed[i] = false;
			for(int t = 0; t <= instance.getMaxTime(); t++)
			{
				auto inEdges = locationGraph.incomingEdges(i, t);
				for(auto edge : inEdges)
				{
					if(fvals[c * locationEdgeCount + locEdgeIndex[edge]] > 0)
					{
						stationUsed[i] = true;
					}
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
		tikzfile << "\\caption{car " << c << "}" << std::endl;
		tikzfile << "\\begin{tikzpicture}" << std::endl;

		//tikzfile << (stationsVisited - 1) / 2 << std::endl;
		//tikzfile << ((stationsVisited - 1) / 2) << std::endl;
		tikzfile << "\\node (r) at (" << (stationsVisited - 1) / 2 << ", 1) {$r$};" << std::endl;

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
					auto inEdges = locationGraph.incomingEdges(i, t);
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
						double fval = fvals[c * locationEdgeCount + locEdgeIndex[edge]] > 0;
						if(fval > 0)
						{
							tikzfile << "(" << source << ") -- (" << i << "-" << t << ") node [midway, above, font = \\tiny] {" << fval << "}" << std::endl;
						}
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
