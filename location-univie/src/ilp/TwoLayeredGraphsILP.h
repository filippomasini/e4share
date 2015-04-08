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

namespace e4share
{

class TwoLayeredGraphsILP
{
public:
	TwoLayeredGraphsILP(CSLocationInstance instance_, int budget_);
	void solve();

private:
	CSLocationInstance instance;
	BatteryGraph batteryGraph;
	LocationGraph locationGraph;

	int budget;

	IloEnv env;
	IloModel model;
	IloCplex cplex;


	IloBoolVarArray y;
	IloIntVarArray s;
	IloBoolVarArray lambda;
	IloBoolVarArray x;
	IloBoolVarArray z;
	IloBoolVarArray a;
	IloBoolVarArray xc;
	IloBoolVarArray zc;
	IloBoolVarArray f;
	IloBoolVarArray g;

	void addModel();
};

} /* namespace e4share */

#endif /* TWOLAYEREDGRAPHSILP_H_ */
