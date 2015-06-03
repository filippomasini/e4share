/*
 * DummyFormatReader.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef DUMMYFORMATREADER_H_
#define DUMMYFORMATREADER_H_

#include <string>

#include "datastructures/CSLocationInstance.h"

namespace e4share
{

class DummyFormatReader
{
public:
	DummyFormatReader(int walkingDistance_);
	CSLocationInstance readInstance(std::string filename, int carCount, bool uniformProfit, int maxTrips, int maxStations);

private:
	int walkingDistance;
};

} /* namespace e4share */

#endif /* DUMMYFORMATREADER_H_ */
