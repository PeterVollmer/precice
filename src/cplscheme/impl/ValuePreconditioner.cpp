// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "ValuePreconditioner.hpp"
#include "utils/MasterSlave.hpp"

namespace precice {
namespace cplscheme {
namespace impl {

tarch::logging::Log ValuePreconditioner::
   _log ( "precice::cplscheme::ValuePreconditioner" );

ValuePreconditioner:: ValuePreconditioner
(
   std::vector<int> dimensions)
:
   Preconditioner (dimensions),
   _firstTimestep(true)
{}

void ValuePreconditioner::update(bool timestepComplete, const DataValues& oldValues, const DataValues& res)
{
  preciceTrace("update()");

  if(timestepComplete || _firstTimestep){

    std::vector<double> norms(_dimensions.size(),0.0);

    int offset = 0;
    for(size_t k=0; k<_dimensions.size(); k++){
      DataValues part;
      for(int i=0; i<_dimensions[k]*_sizeOfSubVector; i++){
        part.append(oldValues[i+offset]);
      }
      norms[k] = utils::MasterSlave::l2norm(part);
      offset += _dimensions[k]*_sizeOfSubVector;
      assertion(norms[k]>0.0);
    }

    offset = 0;
    for(size_t k=0; k<_dimensions.size(); k++){
      for(int i=0; i<_dimensions[k]*_sizeOfSubVector; i++){
        _weights[i+offset] = 1.0 / norms[k];
        _invWeights[i+offset] = norms[k];
      }
      offset += _dimensions[k]*_sizeOfSubVector;
    }

    _requireNewQR = true;
    _firstTimestep = false;
  }
}

void ValuePreconditioner::update(bool timestepComplete, const Eigen::VectorXd& oldValues, const Eigen::VectorXd& res)
{
  preciceTrace("update()");

  if(timestepComplete || _firstTimestep){

    std::vector<double> norms(_dimensions.size(),0.0);

    int offset = 0;
    for(size_t k=0; k<_dimensions.size(); k++){
      Eigen::VectorXd part = Eigen::VectorXd::Zero(_dimensions[k]*_sizeOfSubVector);
      for(int i=0; i<_dimensions[k]*_sizeOfSubVector; i++){
        part(i) = oldValues(i+offset);
      }
      norms[k] = utils::MasterSlave::l2norm(part);
      offset += _dimensions[k]*_sizeOfSubVector;
      assertion(norms[k]>0.0);
    }

    offset = 0;
    for(size_t k=0; k<_dimensions.size(); k++){
      for(int i=0; i<_dimensions[k]*_sizeOfSubVector; i++){
        _weights[i+offset] = 1.0 / norms[k];
        _invWeights[i+offset] = norms[k];
      }
      offset += _dimensions[k]*_sizeOfSubVector;
    }

    _requireNewQR = true;
    _firstTimestep = false;
    if(_needsGlobalWeights){
      communicateGlobalWeights();
    }
  }
}

}}} // namespace precice, cplscheme
