// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "BroydenPostProcessing.hpp"
#include "cplscheme/CouplingData.hpp"
#include "utils/Globals.hpp"
#include "tarch/la/GramSchmidt.h"
#include "tarch/la/LUDecomposition.h"
#include "tarch/la/MatrixVectorOperations.h"
#include "tarch/la/TransposedMatrix.h"
#include "mesh/Mesh.hpp"
#include "mesh/Vertex.hpp"
#include "utils/Dimensions.hpp"
#include "tarch/la/Scalar.h"
#include "io/TXTWriter.hpp"
#include "io/TXTReader.hpp"

#include <time.h>
#include <sstream>
#include <fstream>
//#include "utils/NumericalCompare.hpp"

namespace precice {
namespace cplscheme {
namespace impl {

// tarch::logging::Log BroydenPostProcessing::
 //      _log("precice::cplscheme::impl::BroydenPostProcessing");

      
BroydenPostProcessing:: BroydenPostProcessing
(
  double initialRelaxation,
  bool forceInitialRelaxation,
  int    maxIterationsUsed,
  int    timestepsReused,
  int    filter,
  double singularityLimit,
  std::vector<int> dataIDs,
  PtrPreconditioner preconditioner)
:
  BaseQNPostProcessing(initialRelaxation, forceInitialRelaxation, maxIterationsUsed, timestepsReused,
		       filter, singularityLimit, dataIDs, preconditioner),
  _invJacobian(),
  _oldInvJacobian(),
  _maxColumns(maxIterationsUsed),
  _currentColumns(0)
{}



void BroydenPostProcessing:: initialize
(
  DataMap& cplData )
{
  // do common QN post processing initialization
  BaseQNPostProcessing::initialize(cplData);
  
  double init = 0.0;
  size_t entries= _residuals.size();
  
  _invJacobian = Matrix(entries, entries, init);
  _oldInvJacobian = Matrix(entries, entries, init);
}



void BroydenPostProcessing::computeUnderrelaxationSecondaryData
(
  DataMap& cplData)
{
    // Perform underrelaxation with initial relaxation factor for secondary data
    for (int id: _secondaryDataIDs){
      PtrCouplingData data = cplData[id];
      DataValues& values = *(data->values);
      values *= _initialRelaxation;                   // new * omg
      DataValues& secResiduals = _secondaryResiduals[id];
      secResiduals = data->oldValues.column(0);    // old
      secResiduals *= 1.0 - _initialRelaxation;       // (1-omg) * old
      values += secResiduals;                      // (1-omg) * old + new * omg
    }
}




void BroydenPostProcessing::updateDifferenceMatrices
(
  DataMap& cplData)
{
  using namespace tarch::la;
  if (_firstIteration && _firstTimeStep) {
  }
  else {
    if (not _firstIteration){
      _currentColumns++;
    }
  }
  
  // call the base method for common update of V, W matrices
  BaseQNPostProcessing::updateDifferenceMatrices(cplData);
}

void BroydenPostProcessing::computeQNUpdate
    (PostProcessing::DataMap& cplData, DataValues& xUpdate)
{
  preciceTrace("computeQNUpdate()");
  using namespace tarch::la;
  
  preciceDebug("currentColumns="<<_currentColumns);  
  if(_currentColumns > 1)
  {
     preciceDebug("compute update with QR-dec");
     computeNewtonFactorsQRDecomposition(cplData, xUpdate);
  }else
  {
    preciceDebug("compute update with Broyden");
    // ------------- update inverse Jacobian -----------
    // ------------- Broyden Update
    //
    // J_inv = J_inv_n + (w- J_inv_n*v)*v^T/|v|_l2
    // ----------------------------------------- -------
  
    DataValues v = _matrixV.column(0);
    DataValues w = _matrixW.column(0);
    Matrix JUpdate(_invJacobian.rows(),_invJacobian.cols(), 0.);

    preciceDebug("took latest column of V,W");

    double dotproductV = v*v;
    DataValues tmp = _oldInvJacobian * v;    // J_inv*v
    tmp = w - tmp;                        // (w-J_inv*v)
    tmp = tmp/dotproductV;                // (w-J_inv*v)/|v|_l2
    preciceDebug("did step (W-J_inv*v)/|v|");
    Matrix tmp_mat(tmp.size(),1);
    Matrix vT_mat(1,v.size());
    for(int i = 0; i < v.size(); i++)      // transform vectors in matrices
    {
      tmp_mat(i,0) = tmp(i);
      vT_mat(0,i) = v(i);
    }
    preciceDebug("converted vectors into matrices");
    assertion2(tmp_mat.cols() == vT_mat.rows(), tmp_mat.cols(), vT_mat.rows());
    multiply(tmp_mat, vT_mat, JUpdate);   // (w-J_inv*v)/|v| * v^T
    preciceDebug("multiplied (w-J_inv*v)/|v| * v^T");
    assertion2(_invJacobian.rows() == JUpdate.rows(), _invJacobian.rows(), JUpdate.rows());
    _invJacobian = _oldInvJacobian + JUpdate;
    
    DataValues res_tilde(_residuals.size());
    for(int i = 0; i < res_tilde.size(); i++)
      res_tilde(i) = _residuals(i);

    res_tilde *= -1.;
  
    // solve delta_x = - J_inv*residuals
    multiply(_invJacobian, res_tilde, xUpdate);
  }
  
  if(_currentColumns >= _maxColumns)
  {
    _currentColumns = 0; 
    _oldInvJacobian = _invJacobian;
  }
}


void BroydenPostProcessing::computeNewtonFactorsQRDecomposition
(PostProcessing::DataMap& cplData, DataValues& xUpdate)
{
  preciceTrace("computeNewtonFactorsQRDecomposition()");
  using namespace tarch::la;
 
  // ------------- update inverse Jacobian -----------
  // J_inv = J_inv_n + (W - J_inv_n*V)*(V^T*V)^-1*V^T
  // ----------------------------------------- -------

  assertion2(_currentColumns <= _matrixV.cols(), _currentColumns, _matrixV.cols());
  DataMatrix v;
  DataMatrix Vcopy, _matV, _matW;
  DataMatrix Q(_matrixV.rows(), _currentColumns, 0.0);
  DataMatrix R(_currentColumns, _currentColumns, 0.0);
  
  for(int i = 0; i < _currentColumns; i++)
  {
    Vcopy.append(_matrixV.column(i));
    _matV.append(_matrixV.column(i));
    _matW.append(_matrixW.column(i));
  }

    modifiedGramSchmidt(Vcopy, Q, R);
  
    DataValues ytmpVec(_currentColumns, 0.0);
    DataValues _matrixQRow;
    for(int i = 0; i < Q.rows(); i++)
    {
      for(int j=0; j < Q.cols(); j++){
      _matrixQRow.append(Q(i,j));
      }
      backSubstitution(R, _matrixQRow, ytmpVec);	
      v.append(ytmpVec);  
    _matrixQRow.clear();
    }
 
  // tmpMatrix = J_inv_n*V
  Matrix tmpMatrix(_matrixV.rows(), _currentColumns, 0.0);
  assertion2(_oldInvJacobian.cols() == _matrixV.rows(), _oldInvJacobian.cols(), _matrixV.rows());
  multiply(_oldInvJacobian, _matV, tmpMatrix);
  
  // tmpMatrix = (W-J_inv_n*V)
  tmpMatrix *= -1.;
  tmpMatrix = tmpMatrix + _matW;

  // invJacobian = (W - J_inv_n*V)*(V^T*V)^-1*V^T
  assertion2(tmpMatrix.cols() == v.rows(), tmpMatrix.cols(), v.rows());
  
  multiply(tmpMatrix, v, _invJacobian);
  _invJacobian = _invJacobian + _oldInvJacobian;
  
  DataValues res_tilde(_residuals.size());
  for(int i = 0; i < res_tilde.size(); i++)
   res_tilde(i) = _residuals(i);

  res_tilde *= -1.;
  
  // solve delta_x = - J_inv*residuals
  multiply(_invJacobian, res_tilde, xUpdate);
}



void BroydenPostProcessing:: specializedIterationsConverged
(
   DataMap & cplData)
{
  _currentColumns = 0;
  // store old Jacobian
  _oldInvJacobian = _invJacobian;
}

}}} // namespace precice, cplscheme, impl
