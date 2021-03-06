// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "ScaleByDtAction.hpp"
#include "mesh/Data.hpp"
#include "mesh/Edge.hpp"
#include "mesh/Triangle.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/PropertyContainer.hpp"
#include "utils/Dimensions.hpp"
#include "utils/Globals.hpp"

namespace precice {
namespace action {

tarch::logging::Log ScaleByDtAction:: _log ( "precice::action::ScaleByDtAction" );

ScaleByDtAction:: ScaleByDtAction
(
  Timing               timing,
  int                  sourceDataID,
  int                  targetDataID,
  const mesh::PtrMesh& mesh,
  Scaling              scaling )
:
  Action(timing, mesh),
  _sourceData(mesh->data(sourceDataID)),
  _targetData(mesh->data (targetDataID)),
  _scaling(scaling)
{
  assertion2(_sourceData->getDimensions() == _targetData->getDimensions(),
             _sourceData->getDimensions(), _targetData->getDimensions());
}

void ScaleByDtAction:: performAction
(
  double time,
  double dt,
  double computedPartFullDt,
  double fullDt )
{
  preciceTrace3("performAction()", dt, computedPartFullDt, fullDt);
  utils::DynVector& sourceValues = _sourceData->values();
  utils::DynVector& targetValues = _targetData->values();
  assertion2(sourceValues.size() == targetValues.size(),
             sourceValues.size(), targetValues.size());
  if (_scaling == SCALING_BY_COMPUTED_DT_RATIO){
    double scaling = dt / fullDt;
    preciceDebug("Scale by computed dt ratio " << scaling);
    for (int i=0; i < targetValues.size(); i++){
      targetValues[i] = sourceValues[i] * scaling;
    }
  }
  else if (_scaling == SCALING_BY_DT){
    preciceDebug("Scale by dt " << fullDt);
    for ( int i=0; i < targetValues.size(); i++ ){
      targetValues[i] = sourceValues[i] * fullDt;
    }
  }
  else {
    assertion1(_scaling == SCALING_BY_COMPUTED_DT_PART_RATIO, _scaling);
    double scaling = computedPartFullDt / fullDt;
    preciceDebug("Scale by computed dt part ratio " << scaling);
    for (int i=0; i < targetValues.size(); i++){
      targetValues[i] = sourceValues[i] * scaling;
    }
  }
}

}} // namspace precice, action
