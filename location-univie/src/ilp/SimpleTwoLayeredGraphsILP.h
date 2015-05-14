/*
 * SimpleTwoLayeredGraphsILP.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef SIMPLETWOLAYEREDGRAPHSILP_H_
#define SIMPLETWOLAYEREDGRAPHSILP_H_

#include "datastructures/CSLocationInstance.h"
#include "datastructures/BatteryGraph.h"
#include "datastructures/LocationGraph.h"

#include <ilcplex/ilocplex.h>

#include <string>

namespace e4share
{

class SimpleTwoLayeredGraphsILP
{
public:
	SimpleTwoLayeredGraphsILP(CSLocationInstance instance_, int budget_, bool useBatteryGraph_, bool benders_);
	void solve();

private:
	CSLocationInstance instance;
	BatteryGraph batteryGraph;
	LocationGraph locationGraph;

	int budget;
	bool useBatteryGraph;
	bool benders;

	IloEnv env;
	IloModel model;
	IloCplex cplex;

	IloNumVarArray y;
	IloNumVarArray s;
	IloNumVarArray lambda;
	IloNumVarArray a;
	IloNumVarArray f;
	IloNumVarArray g;
	IloNumVarArray g1;

	void addModel();
	void drawSolutionTikz(std::string filename, IloNumArray fvals);
};

} /* namespace e4share */

#endif /* SIMPLETWOLAYEREDGRAPHSILP_H_ */
