/*
 * TwoLayeredGraphsILP.cpp
 *
 *      Author: Georg Brandstätter
 */

#include "ilp/TwoLayeredGraphsILP.h"

#include <map>

namespace e4share
{

TwoLayeredGraphsILP::TwoLayeredGraphsILP(CSLocationInstance instance_, int budget_) :
		instance(instance_),
		batteryGraph(instance),
		locationGraph(instance),
		budget(budget_),
		env(),
		model(env)
{
	// create variables
	int stationCount = instance.getNetwork().getCandidateStations().size();
	int tripCount = instance.getTrips().size();
	int carCount = instance.getCarCount();

	// stations
	y = IloBoolVarArray(env, stationCount);
	s = IloIntVarArray(env, stationCount);
	for(int i = 0; i < stationCount; i++)
	{
		y[i] = IloBoolVar(env);
		s[i] = IloIntVar(env);
	}

	// trips and trip assignment
	lambda = IloBoolVarArray (env, tripCount);
	x = IloBoolVarArray(env, stationCount * tripCount);
	z = IloBoolVarArray(env, stationCount * tripCount);
	a = IloBoolVarArray(env, carCount * tripCount);
	xc = IloBoolVarArray(env, carCount * stationCount * tripCount);
	zc = IloBoolVarArray(env, carCount * stationCount * tripCount);
	for(int k = 0; k < tripCount; k++)
	{
		lambda[k] = IloBoolVar(env);

		for(int i = 0; i < stationCount; i++)
		{
			int index = k * stationCount + i;
			x[index] = IloBoolVar(env);
			z[index] = IloBoolVar(env);
		}

		for(int c = 0; c < carCount; c++)
		{
			int index = k * carCount + c;
			a[index] = IloBoolVar(env);

			for(int i = 0; i < stationCount; i++)
			{
				int index2 = k * stationCount * carCount + c * stationCount + i;
				xc[index2] = IloBoolVar(env);
				zc[index2] = IloBoolVar(env);
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
	std::map<BatteryGraph::Edge, int> batEdgeIndex;

	// flow in location graph
	int locationEdgeCount = locationGraph.edgeCount();
	f = IloNumVarArray(env, carCount * locationEdgeCount);

	// flow in battery graph
	int batteryEdgeCount = batteryGraph.edgeCount();
	g = IloNumVarArray(env, carCount * batteryEdgeCount);
	for(int c = 0; c < carCount; c++)
	{
		for(int e = 0; e < locationEdgeCount; e++)
		{
			f[c * locationEdgeCount + e] = IloNumVar(env);
		}

		for(int e = 0; e < batteryEdgeCount; e++)
		{
			g[c * batteryEdgeCount + e] = IloNumVar(env);
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
		IloExpr capacity(env);
		capacity += s[i];
		model.add(capacity <= instance.getNetwork().getCandidateStations()[i].getCapacity() * y[i]);
		capacity.end();
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
		for(int tripIndex : startCandidates)
		{
			startStations += x[k * instance.getNetwork().getCandidateStations().size() + tripIndex];
		}
		for(int tripIndex : endCandidates)
		{
			endStations += z[k * instance.getNetwork().getCandidateStations().size() + tripIndex];
		}
		model.add(startStations == lambda[k]);
		model.add(endStations == lambda[k]);
		startStations.end();
		endStations.end();
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
		model.add(rootFlow == carTrips);
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

	cplex.getValues(lambdavals, lambda);
	cplex.getValues(xvals, x);
	cplex.getValues(zvals, z);
	cplex.getValues(avals, a);
	cplex.getValues(xcvals, xc);
	cplex.getValues(zcvals, zc);

	for(int i = 0; i < stationCount; i++)
	{
		std::cout << "y[" << i << "]: " << yvals[i] << ", " << "s[" << i << "]: " << svals[i] << std::endl;
	}

	for(int k = 0; k < tripCount; k++)
	{
		std::cout << "lambda[" << k << "]: " << lambdavals[k] << std::endl;

		if(lambdavals[k] == 1)
		{
			for(int i = 0; i < stationCount; i++)
			{
				int index = k * stationCount + i;
				if(xvals[index] == 1)
				{
					std::cout << " startstation " << i << std::endl;
				}
				if(zvals[index] == 1)
				{
					std::cout << " endstation " << i << std::endl;
				}
			}

			for(int c = 0; c < carCount; c++)
			{
				int index = k * carCount + c;
				if(avals[index] == 1)
				{
					std::cout << " car " << c << std::endl;
				}
	//			std::cout << "a[" << index << "]: " << avals[index] << std::endl;

				for(int i = 0; i < stationCount; i++)
				{
					int index2 = k * stationCount * carCount + c * stationCount + i;
					//std::cout << "xc[" << index2 << "]: " << xcvals[index2] << std::endl;
					//std::cout << "zc[" << index2 << "]: " << zcvals[index2] << std::endl;
				}
			}
		}
	}
}

} /* namespace e4share */
