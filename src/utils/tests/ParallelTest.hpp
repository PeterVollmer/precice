// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#ifndef PRECICE_UTILS_PARALLELTEST_HPP_
#define PRECICE_UTILS_PARALLELTEST_HPP_

#include "tarch/tests/TestCase.h"
#include "tarch/logging/Log.h"

namespace precice {
namespace utils {
namespace tests {

class ParallelTest : public tarch::tests::TestCase
{
public:

  ParallelTest ();

  virtual ~ParallelTest () {}

  virtual void setUp () {}

  virtual void run ();

private:

  static tarch::logging::Log _log;
};

}}} // namespace precice, utils, tests

#endif /* PARALLELTEST_HPP_ */
