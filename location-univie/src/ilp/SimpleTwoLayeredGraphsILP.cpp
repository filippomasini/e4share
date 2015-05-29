/*
 * SimpleTwoLayeredGraphsILP.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "ilp/SimpleTwoLayeredGraphsILP.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>

#include <fstream>
#include <iostream>
#include <sstream>

#include "BendersCallback.h"

namespace e4share
{

SimpleTwoLayeredGraphsILP::SimpleTwoLayeredGraphsILP(CSLocationInstance instance_, int budget_, bool useBatteryGraph_, bool benders_) :
		instance(instance_),
		batteryGraph(instance),
		locationGraph(instance),
		budget(budget_),
		useBatteryGraph(useBatteryGraph_),
		benders(benders_),
		env(),
		model(env)
{
	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	// stations
	y = IloNumVarArray(env, stationCount);
	s = IloNumVarArray(env, stationCount);
	for(int i = 0; i < stationCount; i++)
	{
		std::stringstream yname;
		yname << "y" << i;
		std::stringstream sname;
		sname << "s" << i;
		y[i] = IloBoolVar(env, yname.str().c_str());
		s[i] = IloIntVar(env, sname.str().c_str());
		s[i].setBounds(0, instance.getNetwork().getCandidateStations()[i].getCapacity());
	}

	lambda = IloNumVarArray(env, tripCount);
	a = IloNumVarArray(env, carCount * tripCount);

	for(int k = 0; k < tripCount; k++)
	{
		std::stringstream lname;
		lname << "lambda" << k;
		lambda[k] = IloBoolVar(env, lname.str().c_str());

		for(int c = 0; c < carCount; c++)
		{
			int index = k * carCount + c;
			std::stringstream aname;
			aname << "a" << c << "," << k;
			a[index] = IloBoolVar(env, aname.str().c_str());
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
	f = IloNumVarArray(env, carCount * locationEdgeCount);

	// flow in battery graph
	int batteryEdgeCount = batteryGraph.edgeCount();
	int timeslots = instance.getMaxTime() + 1;
	if(useBatteryGraph)
	{
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
			std::stringstream name;
			name << "f" << c << "," << e;
			f[c * locationEdgeCount + e] = IloNumVar(env);
			f[c * locationEdgeCount + e].setName(name.str().c_str());
			f[c * locationEdgeCount + e].setBounds(0, 1);
			//f[c * locationEdgeCount + e] = IloBoolVar(env);
		}

		if(useBatteryGraph)
		{
			for(int e = 0; e < batteryEdgeCount; e++)
			{
				std::stringstream name;
				name << "g" << c << "," << e;
				g[c * batteryEdgeCount + e] = IloNumVar(env);
				g[c * batteryEdgeCount + e].setName(name.str().c_str());
				g[c * batteryEdgeCount + e].setBounds(0, 1);
			}
		}
		else
		{
			for(int e = 0; e < timeslots; e++)
			{
				std::stringstream name;
				name << "g1" << c << "," << e;
				g1[c * timeslots + e] = IloNumVar(env);
				g1[c * timeslots + e].setName(name.str().c_str());
				g1[c * timeslots + e].setBounds(0, 100);
			}
		}

	}

	bendersFvals = IloNumArray(env, carCount * locationEdgeCount);


	// objective function
	IloExpr obj(env);
	for(int k = 0; k < instance.getTrips().size(); k++)
	{
		obj += instance.getTrips()[k].getProfit() * lambda[k];
		//std::cout << instance.getTrips()[k].getProfit() << std::endl;
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
	}

	if(!benders)
	{

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
			auto tripArcs = locationGraph.tripArcsOf(instance.getTrips()[k]);
			for(int c = 0; c < carCount; c++)
			{
				IloExpr selectedTripFlow(env);
				for(auto arc : tripArcs)
				{
					selectedTripFlow += f[c * locationEdgeCount + locEdgeIndex[arc]];
				}
				selectedTripFlow -= a[k * instance.getCarCount() + c];
				IloRange temp(selectedTripFlow == 0);
				temp.setName("selectedTrip");
				//model.add(selectedTripFlow == a[k * instance.getCarCount() + c]);
				model.add(temp);
				selectedTripFlow.end();
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
				//std::cout << tripArcs.size() << " trip arcs for trip " << k << std::endl;

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
					//model.add(g1[c * timeslots + t] >= 0);
					//model.add(g1[c * timeslots + t] <= 100);
				}
				//model.add(g1[c * timeslots] == 1);
			}

			// recharging
			for(int c = 0; c < carCount; c++)
			{
				for(int t = 0; t < timeslots - 1; t++)
				{
					model.add(g1[c * timeslots + (t+1)] <= g1[c * timeslots + t] + 10);
				}
			}

			// depleting for trip
			for(int c = 0; c < carCount; c++)
			{
				for(int k = 0; k < tripCount; k++)
				{
					auto trip = instance.getTrips()[k];
					double roundedConsumption = (ceil(trip.getBatteryConsumption() * 20)) / (double)20;
					//std::cout << roundedConsumption << ", " << trip.getBatteryConsumption() << std::endl;
					IloExpr rhs = g1[c * timeslots + trip.getBeginTime()] -
							trip.getBatteryConsumption() * a[k * carCount + c] +
							(1 - a[k * carCount + c]) * (trip.getEndTime() - trip.getBeginTime()) * 10;
					model.add(g1[c * timeslots + trip.getEndTime()] <= rhs);
					rhs.end();
				}
			}
		}
	}
	// else: Benders


	//addModel();
}

void SimpleTwoLayeredGraphsILP::addModel()
{

}

void SimpleTwoLayeredGraphsILP::solve()
{
	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	cplex = IloCplex(model);
	cplex.setParam(IloCplex::NumericalEmphasis, true);
	if(!benders)
	{
		cplex.exportModel("nobenders.lp");
	}

	BendersCallback callback(env, instance, locationGraph, batteryGraph, lambda, y, s, a, bendersFvals, cplex);

	if(benders)
	{
		cplex.use((LazyConsI*) &callback);
		cplex.setParam(IloCplex::Threads, 1);
	}
	cplex.solve();
	std::cout << "obj: " << cplex.getObjValue() << std::endl;
	std::cout << "CPLEX status: " << cplex.getStatus() << std::endl;

	auto yvals = IloNumArray(env);
	auto svals = IloNumArray(env);

	cplex.getValues(yvals, y);
	cplex.getValues(svals, s);

	// trips and trip assignment
	auto lambdavals = IloNumArray(env);
	auto avals = IloNumArray(env);
	auto fvals = IloNumArray(env);
	auto gvals = IloNumArray(env);
	auto g1vals = IloNumArray(env);

	cplex.getValues(lambdavals, lambda);
	cplex.getValues(avals, a);

	if(!benders)
	{
		cplex.getValues(fvals, f);
		if(useBatteryGraph)
		{
			cplex.getValues(gvals, g);
		}
		else
		{
			cplex.getValues(g1vals, g1);
		}
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

			/*for(int i = 0; i < stationCount; i++)
			{
				int index = k * stationCount + i;
				if(xvals[index] > 0.50)
				{
					std::cout << " startstation " << i << std::endl;
				}
				else if(xvals[index] != 0)
				{
					std::cout << " startstation " << i << ": " << xvals[index] << std::endl;
				}
				if(zvals[index] > 0)
				{
					std::cout << " endstation " << i << std::endl;
				}
				else if(zvals[index] != 0)
				{
					std::cout << " endstation " << i << ": " << zvals[index] << std::endl;
				}
			}*/

			for(int c = 0; c < carCount; c++)
			{
				int index = k * carCount + c;
				if(avals[index] > 0)
				{
					std::cout << " car " << c << std::endl;
				}
	//			std::cout << "a[" << index << "]: " << avals[index] << std::endl;

				/*for(int i = 0; i < stationCount; i++)
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
				}*/
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

			if(!benders)
			{
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
					//std::cout << " " << edge << ", " << station << ", " << time << std::endl;
				}
				//std::cout << "--------" << std::endl;

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
						//std::cout << " " << edge << ", " << charge << ", " << time << std::endl;
					}
					//std::cout << std::endl;
				}
				// alternative tracking
				else
				{
					int timeslots = instance.getMaxTime() + 1;
					for(int t = 0; t < timeslots; t++)
					{
						//std::cout << " t" << t << ": " << g1vals[c * timeslots + t] << std::endl;
					}
				}
			}

		}
	}

	if(!benders)
	{
		drawSolutionTikz("solution.tex", fvals);
	}
	else
	{
		IloEnv env2;
		IloModel model2(env2);
		int locationEdgeCount = locationGraph.edgeCount();
		f = IloNumVarArray(env2, carCount * locationEdgeCount);
		for(int c = 0; c < carCount; c++)
		{
			for(int e = 0; e < locationEdgeCount; e++)
			{
				std::stringstream name;
				name << "f" << c << "," << e;
				f[c * locationEdgeCount + e] = IloNumVar(env2);
				f[c * locationEdgeCount + e].setName(name.str().c_str());
				f[c * locationEdgeCount + e].setBounds(0, 1);
				//f[c * locationEdgeCount + e] = IloBoolVar(env);
			}
		}

		std::map<LocationGraph::Edge, int> locEdgeIndex;
		int edgeIndex = 0;
		for(LocationGraph::Edge e : locationGraph.allEdges())
		{
			locEdgeIndex[e] = edgeIndex;
			edgeIndex++;
		}

		// LOCATION GRAPH FLOW
		// capacity constraint on flow
		for(int i = 0; i < stationCount; i++)
		{
			for(int t = 0; t < instance.getMaxTime(); t++)
			{
				LocationGraph::Edge waitingEdge = locationGraph.getWaitingArc(i, t);
				int waitingEdgeIndex = locEdgeIndex[waitingEdge];
				IloExpr waitingCars(env2);
				for(int c = 0; c < carCount; c++)
				{
					waitingCars += f[c * locationEdgeCount + waitingEdgeIndex];
				}
				IloRange capacityConstraint(waitingCars <= svals[i]);
				std::stringstream name;
				name << "capacity " << i << "," << t;
				capacityConstraint.setName(name.str().c_str());
				model2.add(capacityConstraint);
				waitingCars.end();
			}
		}

		// only use opened stations
		for(int i = 0; i < stationCount; i++)
		{
			for(int t = 0; t <= instance.getMaxTime(); t++)
			{
				auto inEdges = locationGraph.incomingEdges(i, t);
				for(int c = 0; c < carCount; c++)
				{
					IloExpr incomingArcs(env2);
					for(auto edge : inEdges)
					{
						incomingArcs += f[c * locationEdgeCount + locEdgeIndex[edge]];
					}

					IloRange openedStationConstraint(incomingArcs <= yvals[i]);
					std::stringstream name;
					name << "openedStation " << i << "," << t << "," << c;
					openedStationConstraint.setName(name.str().c_str());
					model2.add(openedStationConstraint);
				}
			}
		}

		// only use selected cars
		for(int c = 0; c < carCount; c++)
		{
			IloExpr carTrips(env2);
			IloExpr rootFlow(env2);
			for(int k = 0; k < tripCount; k++)
			{
				carTrips += avals[k * carCount + c];
			}
			auto rootEdges = locationGraph.outgoingEdges();
			for(auto edge: rootEdges)
			{
				rootFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
			}
			IloExpr temp(env2);
			temp += rootFlow;
			temp -= carTrips;
			IloRange selectedCarConstraint1(temp <= 0);
			std::stringstream name1;
			name1 << "selectedCar " << c;
			selectedCarConstraint1.setName(name1.str().c_str());
			//temp.end();
			IloRange selectedCarConstraint2(rootFlow <= 1);
			std::stringstream name2;
			name2 << "selectedCar2 " << c;
			selectedCarConstraint2.setName(name2.str().c_str());
			IloExpr temp2(env);
			temp2 += 1;
			model2.add(selectedCarConstraint1);
			model2.add(selectedCarConstraint2);
			carTrips.end();
			//carTrips2.end();
			rootFlow.end();
			temp.end();
			temp2.end();
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
					IloExpr inFlow(env2);
					IloExpr outFlow(env2);
					IloExpr temp(env2);
					for(auto edge : inEdges)
					{
						//inFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
						temp += f[c * locationEdgeCount + locEdgeIndex[edge]];
					}
					for(auto edge : outEdges)
					{
						//outFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
						temp -= f[c * locationEdgeCount + locEdgeIndex[edge]];
					}

					//temp += inFlow;
					//temp -= outFlow;
					IloRange flowConservationConstraint(temp == 0);
					std::stringstream name;
					name << "flowConservation " << c << "," << i << "," << t;
					flowConservationConstraint.setName(name.str().c_str());
					model2.add(flowConservationConstraint);
					inFlow.end();
					outFlow.end();
					temp.end();
					//flowConservationConstraint.end();
				}
			}
		}


		// flow for covered trips
		for(int k = 0; k < tripCount; k++)
		{
			auto tripArcs = locationGraph.tripArcsOf(instance.getTrips()[k]);
			for(int c = 0; c < carCount; c++)
			{
				IloExpr selectedTripFlow(env2);
				for(auto arc : tripArcs)
				{
					selectedTripFlow += f[c * locationEdgeCount + locEdgeIndex[arc]];
				}
				//std::cout << "foo: " << selectedTripFlow << std::endl;
				IloRange selectedTripFlowConstraint(selectedTripFlow == avals[k * instance.getCarCount() + c]);
				std::stringstream name;
				name << "tripSelection " << k << "," << c;
				selectedTripFlowConstraint.setName(name.str().c_str());
				model2.add(selectedTripFlowConstraint);
				selectedTripFlow.end();
				//selectedTripFlowConstraint.end();
			}
		}

		IloCplex cplex2(model2);
		cplex2.exportModel("additionalFlow.lp");
		cplex2.solve();
		std::cout << cplex2.getStatus() << std::endl;
	}
}

void SimpleTwoLayeredGraphsILP::drawSolutionTikz(std::string filename, IloNumArray fvals)
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
