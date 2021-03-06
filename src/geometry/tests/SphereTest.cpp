// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "SphereTest.hpp"
#include "geometry/Sphere.hpp"
#include "io/ExportVTK.hpp"
#include "mesh/Mesh.hpp"
#include "utils/Parallel.hpp"
#include <sstream>

#include "tarch/tests/TestCaseFactory.h"
registerTest(precice::geometry::tests::SphereTest)

namespace precice {
namespace geometry {
namespace tests {

tarch::logging::Log SphereTest:: _log ( "precice::geometry::tests::SphereTest" );

SphereTest:: SphereTest ()
:
  TestCase ("SphereTest")
{}

void SphereTest:: run ()
{
  PRECICE_MASTER_ONLY {
    preciceTrace ( "run" );
    for ( int dim=2; dim <= 3; dim++ ){
      bool flipNormals = false;
      mesh::Mesh mesh ( "test-sphere", dim, flipNormals );
      utils::DynVector offset ( dim, 0.0 );
      double discretizationWidth = 0.1;
      double radius = 1.0;
      Sphere sphere ( offset, discretizationWidth, radius );
      sphere.create ( mesh );
      io::ExportVTK exportVTK(true);
      std::ostringstream filename;
      filename << "geometry-SphereTest-" << dim;
      exportVTK.doExport ( filename.str(), mesh );
    }
  }
}

}}} // namespace precice, geometry, tests
