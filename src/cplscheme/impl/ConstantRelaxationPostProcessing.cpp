// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "ConstantRelaxationPostProcessing.hpp"
#include "../CouplingData.hpp"
#include "mesh/Data.hpp"
#include "mesh/Mesh.hpp"
#include "tarch/la/DynamicVector.h"
#include "utils/Globals.hpp"
#include "utils/Dimensions.hpp"
//#include "utils/NumericalCompare.hpp"

namespace precice {
namespace cplscheme {
namespace impl {

tarch::logging::Log ConstantRelaxationPostProcessing::
  _log("precice::cplscheme::ConstantRelaxationPostProcessing");

ConstantRelaxationPostProcessing:: ConstantRelaxationPostProcessing
(
  double relaxation,
  std::vector<int>    dataIDs )
:
  PostProcessing(),
  _relaxation(relaxation),
  _dataIDs(dataIDs),
  _designSpecification()
{
  preciceCheck((relaxation > 0.0) && (relaxation <= 1.0),
               "ConstantRelaxationPostProcessing()",
               "Relaxation factor for constant relaxation post processing "
               << "has to be larger than zero and smaller or equal than one!");
}

void ConstantRelaxationPostProcessing:: initialize
(
  DataMap& cplData )
{
  preciceCheck(utils::contained(*_dataIDs.begin(), cplData), "initialize()",
               "Data with ID " << *_dataIDs.begin()
               << " is not contained in data given at initialization!");

  // Append column for old values if not done by coupling scheme yet
  for (DataMap::value_type& pair : cplData) {
    int cols = pair.second->oldValues.cols();
    if (cols < 1){
      assertion1(pair.second->values->size() > 0, pair.first);
      pair.second->oldValues.append(CouplingData::DataMatrix(
        pair.second->values->size(), 1, 0.0));
    }
  }
}

void ConstantRelaxationPostProcessing:: performPostProcessing
(
  DataMap& cplData )
{
  preciceTrace("performPostProcessing()");
  double omega = _relaxation;
  double oneMinusOmega = 1.0 - omega;
  for (DataMap::value_type & pair : cplData) {
    utils::DynVector& values = * pair.second->values;
    utils::DynVector& oldValues = pair.second->oldValues.column(0);
    values *= omega;
    values += oldValues * oneMinusOmega;
    preciceDebug("pp values" << values);
  }
}


/** ---------------------------------------------------------------------------------------------
 *         getDesignSpecification()
 *
 * @brief: Returns the design specification corresponding to the given coupling data.
 *         This information is needed for convergence measurements in the coupling scheme.
 *  ---------------------------------------------------------------------------------------------
 */        // TODO: change to call by ref when Eigen is used.
std::map<int, utils::DynVector> ConstantRelaxationPostProcessing::getDesignSpecification
(
  DataMap& cplData)
{
  preciceError(__func__, "design specification for constant relaxation is not supported yet.");

  std::map<int, utils::DynVector> designSpecifications;
  int off = 0;
  for (int id : _dataIDs) {
      int size = cplData[id]->values->size();
      utils::DynVector q(size, 0.0);
      for (int i = 0; i < size; i++) {
        q(i) = _designSpecification(i+off);
      }
      off += size;
      std::map<int, utils::DynVector>::value_type pair = std::make_pair(id, q);
      designSpecifications.insert(pair);
    }
  return designSpecifications;
}

void ConstantRelaxationPostProcessing::setDesignSpecification(
     Eigen::VectorXd& q)
 {
   _designSpecification = q;
   preciceError(__func__, "design specification for constant relaxation is not supported yet.");
 }


}}} // namespace precice, cplscheme, impl
