/*
 * TwoLayeredGraphsILP.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef TWOLAYEREDGRAPHSILP_H_
#define TWOLAYEREDGRAPHSILP_H_

#include "datastructures/CSLocationInstance.h"
#include "datastructures/BatteryGraph.h"
#include "datastructures/LocationGraph.h"

#include <ilcplex/ilocplex.h>

#include <string>

namespace e4share
{

class TwoLayeredGraphsILP
{
public:
	TwoLayeredGraphsILP(CSLocationInstance instance_, int budget_, bool useBatteryGraph_);
	void solve();

private:
	CSLocationInstance instance;
	BatteryGraph batteryGraph;
	LocationGraph locationGraph;

	int budget;
	bool useBatteryGraph;

	IloEnv env;
	IloModel model;
	IloCplex cplex;


//	IloBoolVarArray y;
//	IloIntVarArray s;
//	IloBoolVarArray lambda;
//	IloBoolVarArray x;
//	IloBoolVarArray z;
//	IloBoolVarArray a;
//	IloBoolVarArray xc;
//	IloBoolVarArray zc;
//	IloBoolVarArray f;
//	IloBoolVarArray g;
//	IloNumVarArray g1;

	IloNumVarArray y;
	IloNumVarArray s;
	IloNumVarArray lambda;
	IloNumVarArray x;
	IloNumVarArray z;
	IloNumVarArray a;
	IloNumVarArray xc;
	IloNumVarArray zc;
	IloNumVarArray f;
	IloNumVarArray g;
	IloNumVarArray g1;

	void addModel();
	void drawSolutionTikz(std::string filename, IloNumArray fvals);
};

} /* namespace e4share */

#endif /* TWOLAYEREDGRAPHSILP_H_ */
