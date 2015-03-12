/*
 * TempFormatReader.h
 *
 *      Author: Georg Brandstätter
 */

#ifndef TEMPFORMATREADER_H_
#define TEMPFORMATREADER_H_

#include <string>

#include "datastructures/CSLocationInstance.h"

namespace e4share
{

class TempFormatReader
{
public:
	TempFormatReader();
	CSLocationInstance readInstance(std::string filename);
};

} /* namespace e4share */

#endif /* TEMPFORMATREADER_H_ */
