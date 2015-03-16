/*
 * loc-uv.cpp
 *
 *      Author: Georg Brandstätter
 */

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "datastructures/CSLocationInstance.h"
#include "datastructures/LocationGraph.h"
#include "input/TempFormatReader.h"
#include "stacktrace.h"

using namespace e4share;

int main(int argc, const char* argv[])
{
	register_handler();
	std::string filename;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("file,f", po::value<std::string>(&filename)->required(), "input file to be processed")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	// notify is done after we handled the 'help'-option
	// this way, we can call the program with -h without having to provide all required options
	po::notify(vm);

	TempFormatReader reader;
	auto instance = reader.readInstance(filename);

	// check

	std::cout << "stations: " << std::endl;
	for(auto cs : instance.getNetwork().getCandidateStations())
	{
		std::cout << cs.getCost() << ", " << cs.getCostPerSlot() << ", " << cs.getCapacity() << std::endl;
	}

	std::cout << "carCount: " << instance.getCarCount() << std::endl;

	std::cout << "trips: " << std::endl;
	for(auto trip : instance.getTrips())
	{
		std::cout << trip.getOrigin() << ", " << trip.getDestination() << ", " << trip.getBeginTime() << ", " <<
				trip.getEndTime() << ", " << trip.getBatteryConsumption() << ", " << trip.getProfit() << std::endl;
	}

	std::cout << "maxTime: " << instance.getMaxTime() << std::endl;

	LocationGraph locationGraph(instance);

	return 0;
}
