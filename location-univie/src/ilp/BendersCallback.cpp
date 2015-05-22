/*
 * BendersCallback.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "ilp/BendersCallback.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <string>

namespace e4share
{

BendersCallback::BendersCallback(IloEnv& env_, CSLocationInstance& instance_, LocationGraph& locationGraph_, BatteryGraph& batteryGraph_,
		IloNumVarArray& lambda_, IloNumVarArray& y_, IloNumVarArray& s_, IloNumVarArray& a_) :
		LazyConsI(env_),
		env(env_),
		instance(instance_),
		locationGraph(locationGraph_),
		batteryGraph(batteryGraph_),
		lambda(lambda_),
		y(y_),
		s(s_),
		a(a_)
{
	// TODO Auto-generated constructor stub

}

void BendersCallback::mainLazy()
{
	std::cout << "starting callback" << std::endl;

	env = LazyConsI::getEnv();
	IloNumArray lambdavals(env);
	IloNumArray yvals(env);
	IloNumArray svals(env);
	IloNumArray avals(env);
	LazyConsI::getValues(lambdavals, lambda);
	LazyConsI::getValues(yvals, y);
	LazyConsI::getValues(svals, s);
	LazyConsI::getValues(avals, a);



	IloEnv subEnv;//(LazyConsI::getEnv());
	IloModel model(subEnv);

	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	for(int i = 0; i < stationCount; i++)
	{
		std::cout << "y[" << i << "]: " << yvals[i] << ", " << "s[" << i << "]: " << svals[i] << std::endl;
	}

	for(int k = 0; k < tripCount; k++)
	{
		std::cout << "lambda[" << k << "]: " << lambdavals[k] << std::endl;

		if(lambdavals[k] > 0)
		{
			for(int c = 0; c < carCount; c++)
			{
				int index = k * carCount + c;
				if(avals[index] > 0)
				{
					std::cout << " car " << c << std::endl;
				}
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
	IloNumVarArray f = IloNumVarArray(subEnv, carCount * locationEdgeCount);

	// flow in battery graph
	int batteryEdgeCount = batteryGraph.edgeCount();
	int timeslots = instance.getMaxTime() + 1;
//	if(useBatteryGraph)
//	{
//		g = IloNumVarArray(env, carCount * batteryEdgeCount);
//	}
//	else
//	{
//		g1 = IloNumVarArray(env, carCount * timeslots);
//	}

	for(int c = 0; c < carCount; c++)
	{
		for(int e = 0; e < locationEdgeCount; e++)
		{
			std::stringstream name;
			name << "f" << c << "," << e;
			f[c * locationEdgeCount + e] = IloNumVar(subEnv, name.str().c_str());
			f[c * locationEdgeCount + e].setBounds(0, 1);
		}

//		if(useBatteryGraph)
//		{
//			for(int e = 0; e < batteryEdgeCount; e++)
//			{
//				g[c * batteryEdgeCount + e] = IloNumVar(env);
//				g[c * batteryEdgeCount + e].setBounds(0, 1);
//			}
//		}
//		else
//		{
//			for(int e = 0; e < timeslots; e++)
//			{
//				g1[c * timeslots + e] = IloNumVar(env);
//				g1[c * timeslots + e].setBounds(0, 100);
//			}
//		}

	}

	// LOCATION GRAPH FLOW
	// capacity constraint on flow

	std::map<IloInt, IloExpr> constraintMap;
	std::vector<IloRange> capacityConstraints;
	for(int i = 0; i < stationCount; i++)
	{
		for(int t = 0; t < instance.getMaxTime(); t++)
		{
			LocationGraph::Edge waitingEdge = locationGraph.getWaitingArc(i, t);
			int waitingEdgeIndex = locEdgeIndex[waitingEdge];
			IloExpr waitingCars(subEnv);
			for(int c = 0; c < carCount; c++)
			{
				waitingCars += f[c * locationEdgeCount + waitingEdgeIndex];
			}
			IloRange capacityConstraint(waitingCars <= svals[i]);
			std::stringstream name;
			name << "capacity " << i << "," << t;
			capacityConstraint.setName(name.str().c_str());
			capacityConstraints.push_back(capacityConstraint);
			constraintMap[capacityConstraint.getId()] = s[i];
			model.add(capacityConstraint);
			waitingCars.end();
		}
	}

	// only use opened stations
	std::vector<IloRange> openedStationConstraints;
	for(int i = 0; i < stationCount; i++)
	{
		for(int t = 0; t <= instance.getMaxTime(); t++)
		{
			auto inEdges = locationGraph.incomingEdges(i, t);
			for(auto edge : inEdges)
			{
				for(int c = 0; c < carCount; c++)
				{
					IloRange openedStationConstraint(f[c * locationEdgeCount + locEdgeIndex[edge]] <= yvals[i]);
					std::stringstream name;
					name << "openedStation " << i << "," << t << "," << c;
					openedStationConstraint.setName(name.str().c_str());
					openedStationConstraints.push_back(openedStationConstraint);
					constraintMap[openedStationConstraint.getId()] = y[i];
					model.add(openedStationConstraint);
				}
			}
		}
	}

	// only use selected cars
	std::vector<IloRange> selectedCarConstraints1;
	std::vector<IloRange> selectedCarConstraints2;
	for(int c = 0; c < carCount; c++)
	{
		IloExpr carTrips(subEnv);
		IloExpr carTrips2(env);
		IloExpr rootFlow(subEnv);
		for(int k = 0; k < tripCount; k++)
		{
			carTrips += avals[k * carCount + c];
			carTrips2 += a[k * carCount + c];
		}
		auto rootEdges = locationGraph.outgoingEdges();
		for(auto edge: rootEdges)
		{
			rootFlow += f[c * locationEdgeCount + locEdgeIndex[edge]];
		}
		IloExpr temp(subEnv);
		temp += rootFlow;
		temp -= carTrips;
		IloRange selectedCarConstraint1(temp <= 0);
		std::stringstream name1;
		name1 << "selectedCar " << c;
		selectedCarConstraint1.setName(name1.str().c_str());
		//temp.end();
		selectedCarConstraints1.push_back(selectedCarConstraint1);
		constraintMap[selectedCarConstraint1.getId()] = carTrips2;
		IloRange selectedCarConstraint2(rootFlow <= 1);
		std::stringstream name2;
		name2 << "selectedCar2 " << c;
		selectedCarConstraint2.setName(name2.str().c_str());
		selectedCarConstraints2.push_back(selectedCarConstraint2);
		IloExpr temp2(env);
		temp2 += 1;
		constraintMap[selectedCarConstraint2.getId()] = temp2;
		model.add(selectedCarConstraint1);
		model.add(selectedCarConstraint2);
		carTrips.end();
		//carTrips2.end();
		rootFlow.end();
		temp.end();
		temp2.end();
	}

	// flow conservation
	std::vector<IloRange> flowConservationConstraints;
	for(int c = 0; c < carCount; c++)
	{
		for(int i = 0; i < stationCount; i++)
		{
			for(int t = 0; t < instance.getMaxTime(); t++)
			{
				auto inEdges = locationGraph.incomingEdges(i, t);
				auto outEdges = locationGraph.outgoingEdges(i, t);
				IloExpr inFlow(subEnv);
				IloExpr outFlow(subEnv);
				IloExpr temp(subEnv);
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
				flowConservationConstraints.push_back(flowConservationConstraint);
				model.add(flowConservationConstraint);
				inFlow.end();
				outFlow.end();
				temp.end();
				//flowConservationConstraint.end();
			}
		}
	}

	// flow for covered trips
	std::vector<IloRange> selectedTripFlowConstraints;
	for(int k = 0; k < tripCount; k++)
	{
		auto tripArcs = locationGraph.tripArcsOf(instance.getTrips()[k]);
		for(int c = 0; c < carCount; c++)
		{
			IloExpr selectedTripFlow(subEnv);
			for(auto arc : tripArcs)
			{
				selectedTripFlow += f[c * locationEdgeCount + locEdgeIndex[arc]];
			}
			//std::cout << "foo: " << selectedTripFlow << std::endl;
			IloRange selectedTripFlowConstraint(selectedTripFlow == avals[k * instance.getCarCount() + c]);
			std::stringstream name;
			name << "tripSelection " << k << "," << c;
			selectedTripFlowConstraint.setName(name.str().c_str());
			selectedTripFlowConstraints.push_back(selectedTripFlowConstraint);
			constraintMap[selectedTripFlowConstraint.getId()] = a[k * instance.getCarCount() + c];
			model.add(selectedTripFlowConstraint);
			selectedTripFlow.end();
			//selectedTripFlowConstraint.end();
		}
	}


	// count constraints
	int constraintCount = capacityConstraints.size() + openedStationConstraints.size() + selectedCarConstraints1.size() +
			selectedCarConstraints2.size() + flowConservationConstraints.size() + selectedTripFlowConstraints.size();
	IloConstraintArray allConstraints(subEnv, constraintCount);
	//IloConstraintArray allConstraints2(subEnv, constraintCount);
	IloNumArray dualVars(subEnv, constraintCount);

	// insert constraints into array
	int cc = 0;
//	for(auto constraint : capacityConstraints)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}
//	for(auto constraint : openedStationConstraints)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}
//	for(auto constraint : selectedCarConstraints1)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}
//	for(auto constraint : selectedCarConstraints2)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}
//	for(auto constraint : flowConservationConstraints)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}
//	for(auto constraint : selectedTripFlowConstraints)
//	{
//		allConstraints[cc] = constraint;
//		cc++;
//	}

	for(auto constraint : constraintMap)
	{
		constraint.second.setName("foo");
	}
	IloCplex cplex(model);
	cplex.exportModel("foo.lp");
	cplex.setParam(IloCplex::RootAlg, IloCplex::Dual);
	cplex.setParam(IloCplex::PreInd, false);
	cplex.solve();
	auto status = cplex.getStatus();
	std::cout << status << std::endl;
	//exit(-1);

	if(status == IloAlgorithm::Infeasible)
	{
		IloExpr feasibilityCut(env);
		// add feasibility cut
		try
		{
			cplex.dualFarkas(allConstraints, dualVars);
			//std::cout << constraintCount << ", " << allConstraints.getSize() << std::endl;
			for(int cs = 0; cs < allConstraints.getSize(); cs++)
			{
				if(dualVars[cs] != 0)
				{
					//std::cout << std::string(allConstraints[cs].getName());
					std::cout << allConstraints[cs] << ", " << dualVars[cs];
					std::cout << " (" << constraintMap[allConstraints[cs].getId()] << ")" << std::endl;
				}
				if(constraintMap.count(allConstraints[cs].getId()) > 0)
				{
					//std::cout << "hooray" << std::endl;
					//std::cout << constraintMap[allConstraints[cs].getId()].getName() << std::endl;
					feasibilityCut += constraintMap[allConstraints[cs].getId()] * dualVars[cs];
				}
			}
			//exit(-1);
			std::cout << feasibilityCut << std::endl;
			LazyConsI::add(feasibilityCut >= 0);
			//std::cout << feasibilityCut << std::endl;
			feasibilityCut.end();
			allConstraints.end();
			dualVars.end();
			//std::cout << "cut added" << std::endl;
			//model.end();
			cplex.end();
			model.end();

			for(int cs = 0; cs < constraintCount; cs++)
			{
//				if(capacityConstraints.count(allConstraints[cs].getId()) > 0)
//				{
//					//std::cout << allConstraints[cs] << ", " << dualVars[cs] << std::endl;
//					std::cout << capacityConstraints[allConstraints[cs].getId()].getName() << std::endl;
//				}
				//std::cout << allConstraints[cs].getId() << std::endl;
//				if(allConstraints[cs].getId() == "foobar")
//				{
//
//				}

			}
//			for(int i = 0; i < stationCount; i++)
//			{
//				for(int t = 0; t <= instance.getMaxTime(); t++)
//				{
//
//				}
//			}
		}
		catch (IloException& e)
		{
			std::cout << e.getMessage() << std::endl;
		}
	}

	//std::cout << "ending callback" << std::endl;
}

} /* namespace e4share */

