/*
 * BendersCallback.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef BENDERSCALLBACK_H_
#define BENDERSCALLBACK_H_

#include <ilcplex/ilocplex.h>

#include "SimpleTwoLayeredGraphsILP.h"

namespace e4share
{

class LazyConsI: public IloCplex::LazyConstraintCallbackI
{

private:

	virtual void main()
	{
		mainLazy();
	}

	virtual IloCplex::CallbackI* duplicateCallback() const
	{
		return duplicateCallbackLazy();
	}

public:

	LazyConsI( IloEnv e ) :
			IloCplex::LazyConstraintCallbackI( e )
	{
	}

	/*virtual ~LazyConsI()
	{
	}*/

	virtual void mainLazy() = 0;
	virtual IloCplex::CallbackI* duplicateCallbackLazy() const = 0;

};

class BendersCallback : LazyConsI
{
public:
	BendersCallback(IloEnv& env_, CSLocationInstance& instance_, LocationGraph& locationGraph_, BatteryGraph& batteryGraph_,
			IloNumVarArray& lambda_, IloNumVarArray& y_, IloNumVarArray& s_, IloNumVarArray& a_, IloNumArray& fvals_, IloCplex& master);

	// entry for lazy constraint callback (called for integer solutions)
	virtual void mainLazy();

	virtual IloCplex::CallbackI* duplicateCallbackLazy() const
	{
		return (LazyConsI *) (this);
	}

private:
	IloEnv& env;
	CSLocationInstance& instance;
	LocationGraph& locationGraph;
	BatteryGraph& batteryGraph;

	IloNumVarArray& lambda;
	IloNumVarArray& y;
	IloNumVarArray& s;
	IloNumVarArray& a;

	IloNumArray& fvals;

	IloCplex& master;
	int counter = 0;
};

} /* namespace e4share */

#endif /* BENDERSCALLBACK_H_ */
