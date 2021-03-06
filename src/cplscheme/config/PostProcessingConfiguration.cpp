// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#include "PostProcessingConfiguration.hpp"
#include "cplscheme/impl/ConstantRelaxationPostProcessing.hpp"
#include "cplscheme/impl/AitkenPostProcessing.hpp"
#include "cplscheme/impl/HierarchicalAitkenPostProcessing.hpp"
#include "cplscheme/impl/IQNILSPostProcessing.hpp"
#include "cplscheme/impl/MVQNPostProcessing.hpp"
#include "cplscheme/impl/MMPostProcessing.hpp"
#include "cplscheme/impl/BroydenPostProcessing.hpp"
#include "cplscheme/impl/BaseQNPostProcessing.hpp"
#include "cplscheme/impl/PostProcessing.hpp"
#include "cplscheme/impl/BroydenPostProcessing.hpp"
#include "cplscheme/impl/ConstantPreconditioner.hpp"
#include "cplscheme/impl/ValuePreconditioner.hpp"
#include "cplscheme/impl/ResidualPreconditioner.hpp"
#include "cplscheme/impl/ResidualSumPreconditioner.hpp"
#include "mesh/config/MeshConfiguration.hpp"
#include "mesh/Data.hpp"
#include "mesh/Mesh.hpp"
#include "utils/xml/XMLTag.hpp"
#include "utils/xml/XMLAttribute.hpp"
#include "utils/xml/ValidatorEquals.hpp"
#include "utils/xml/ValidatorOr.hpp"
#include "utils/Globals.hpp"

namespace precice {
namespace cplscheme {

tarch::logging::Log PostProcessingConfiguration::
      _log ( "precice::cplscheme::PostProcessingConfiguration" );

//const std::string & PostProcessingConfiguration:: getTag ()
//{
//  static std::string tag ( "post-processing" );
//  return tag;
//}

PostProcessingConfiguration:: PostProcessingConfiguration
(
  const mesh::PtrMeshConfiguration& meshConfig)
:
  TAG("post-processing"),
  TAG_RELAX("relaxation" ),
  TAG_INIT_RELAX("initial-relaxation"),
  TAG_MAX_USED_ITERATIONS("max-used-iterations"),
  TAG_TIMESTEPS_REUSED("timesteps-reused"),
  TAG_DATA("data"),
  TAG_FILTER("filter"),
  TAG_ESTIMATEJACOBIAN("estimate-jacobian"),
  TAG_PRECONDITIONER("preconditioner"),
  ATTR_NAME("name"),
  ATTR_MESH("mesh"),
  ATTR_SCALING("scaling"),
  ATTR_VALUE("value"),
  ATTR_ENFORCE("enforce"),
  ATTR_SINGULARITYLIMIT("limit"),
  ATTR_TYPE("type"),
  VALUE_CONSTANT("constant"),
  VALUE_AITKEN ("aitken"),
  VALUE_HIERARCHICAL_AITKEN("hierarchical-aitken"),
  VALUE_IQNILS ("IQN-ILS"),
  VALUE_MVQN("IQN-IMVJ"),
  VALUE_ManifoldMapping("MM"),
  VALUE_BROYDEN("broyden"),
  VALUE_QR1FILTER("QR1"),
  VALUE_QR1_ABSFILTER("QR1-absolute"),
  VALUE_QR2FILTER("QR2"),
  VALUE_CONSTANT_PRECONDITIONER("constant"),
  VALUE_VALUE_PRECONDITIONER("value"),
  VALUE_RESIDUAL_PRECONDITIONER("residual"),
  VALUE_RESIDUAL_SUM_PRECONDITIONER("residual-sum"),
  //_isValid(false),
  _meshConfig(meshConfig),
  _postProcessing(),
  _coarseModelOptimizationConfig(),
  _neededMeshes(),
  _preconditioner(),
  _config(),
  _isAddManifoldMappingTagAllowed(true)
{
  assertion(meshConfig.get() != nullptr);
}

void PostProcessingConfiguration:: connectTags(
    utils::XMLTag&                    parent){

  using namespace utils;

 // static int recursionCounter = 0;
 // recursionCounter++;

    XMLTag::Occurrence occ = XMLTag::OCCUR_NOT_OR_ONCE;
    std::list<XMLTag> tags;
    {
      XMLTag tag(*this, VALUE_CONSTANT, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }
    {
      XMLTag tag(*this, VALUE_AITKEN, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }
    {
      XMLTag tag(*this, VALUE_HIERARCHICAL_AITKEN, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }
    {
      XMLTag tag(*this, VALUE_IQNILS, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }
    {
      XMLTag tag(*this, VALUE_MVQN, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }
    //if(recursionCounter <= 1){
    if(_isAddManifoldMappingTagAllowed){
      {
        XMLTag tag(*this, VALUE_ManifoldMapping, occ, TAG);
        addTypeSpecificSubtags(tag);
        tags.push_back(tag);
      }
      _isAddManifoldMappingTagAllowed = false;
    }
    {
      XMLTag tag(*this, VALUE_BROYDEN, occ, TAG);
      addTypeSpecificSubtags(tag);
      tags.push_back(tag);
    }


    for (XMLTag& tag : tags) {
      parent.addSubtag(tag);
    }
}

//bool PostProcessingConfiguration:: parseSubtag
//(
//  tarch::irr::io::IrrXMLReader * xmlReader )
//{
//  preciceTrace("parseSubtag()" );
//  using namespace utils;
//
//  XMLTag tag(TAG, XMLTag::OCCUR_ONCE );
//
//  XMLAttribute<std::string> attrType(ATTR_TYPE );
//  ValidatorEquals<std::string> validConstant(VALUE_CONSTANT );
//  ValidatorEquals<std::string> validAitken(VALUE_AITKEN );
//  ValidatorEquals<std::string> validHierarchAitken(VALUE_HIERARCHICAL_AITKEN );
//  ValidatorEquals<std::string> validIQNILS(VALUE_IQNILS );
//  attrType.setValidator (
//    validConstant || validAitken || validHierarchAitken|| validIQNILS );
//  tag.addAttribute(attrType );value
//
//  XMLAttribute<std::string> attrData(ATTR_DATA );
//  tag.addAttribute(attrData );
//
//  XMLAttribute<std::string> attrMesh(ATTR_MESH );
//  tag.addAttribute(attrMesh );
//
//  _isValid = tag.parse(xmlReader, *this );
//  _config = ConfigurationData ();
//  return _isValid;
//}

//bool PostProcessingConfiguration:: isValid() const
//{
//  return _isValid;
//}

impl::PtrPostProcessing PostProcessingConfiguration:: getPostProcessing()
{
  return _postProcessing;
}

PtrPostProcessingConfiguration PostProcessingConfiguration::getCoarseModelOptimizationConfig()
{
  return _coarseModelOptimizationConfig;
}


void PostProcessingConfiguration:: xmlTagCallback
(
  utils::XMLTag& callingTag )
{
  preciceTrace1("xmlTagCallback()", callingTag.getFullName());

  if (callingTag.getNamespace() == TAG){
      _config.type = callingTag.getName();
      //_coarseModelOptimizationConfig->clear();
  }

  if (callingTag.getName() == TAG_RELAX){
    _config.relaxationFactor = callingTag.getDoubleAttributeValue(ATTR_VALUE);
  }
  else if (callingTag.getName() == TAG_DATA){
    std::string dataName = callingTag.getStringAttributeValue(ATTR_NAME);
    _meshName = callingTag.getStringAttributeValue(ATTR_MESH);
    double scaling = 1.0;
    if(_config.type == VALUE_IQNILS || _config.type == VALUE_MVQN ||
       _config.type == VALUE_ManifoldMapping || _config.type == VALUE_BROYDEN){
      scaling = callingTag.getDoubleAttributeValue(ATTR_SCALING);
    }


    for(mesh::PtrMesh mesh : _meshConfig->meshes() ) {
      if(mesh->getName() == _meshName ) {
        for (mesh::PtrData data : mesh->data() ) {
          if (dataName == data->getName()){
            _config.dataIDs.push_back(data->getID());
            _config.scalings.insert(std::make_pair(data->getID(),scaling));
          }
        }
      }
    }

    if (_config.dataIDs.empty()){
      std::ostringstream stream;
      stream << "Data with name \"" << dataName << "\" associated to mesh \""
             << _meshName << "\" not found on configuration of post-processing";
      throw stream.str();
    }
    _neededMeshes.push_back(_meshName);
  }
  else if (callingTag.getName() == TAG_INIT_RELAX){
    _config.relaxationFactor = callingTag.getDoubleAttributeValue(ATTR_VALUE);
    _config.forceInitialRelaxation = callingTag.getBooleanAttributeValue(ATTR_ENFORCE);
  }
  else if (callingTag.getName() == TAG_MAX_USED_ITERATIONS){
    _config.maxIterationsUsed = callingTag.getIntAttributeValue(ATTR_VALUE);
  }
  else if (callingTag.getName() == TAG_TIMESTEPS_REUSED){
    _config.timestepsReused = callingTag.getIntAttributeValue(ATTR_VALUE);
  }
  else if (callingTag.getName() == TAG_FILTER){
	  auto f = callingTag.getStringAttributeValue(ATTR_TYPE);
	  if(f == VALUE_QR1FILTER){
		  _config.filter = impl::PostProcessing::QR1FILTER;
	  }else if (f == VALUE_QR1_ABSFILTER){
	  		  _config.filter = impl::PostProcessing::QR1FILTER_ABS;
	  }else if (f == VALUE_QR2FILTER){
		  _config.filter = impl::PostProcessing::QR2FILTER;
	  }else {
	    assertion(false);
	  }
	  _config.singularityLimit = callingTag.getDoubleAttributeValue(ATTR_SINGULARITYLIMIT);
  }else if (callingTag.getName() == TAG_ESTIMATEJACOBIAN) {
    if(_config.type == VALUE_ManifoldMapping)
         _config.estimateJacobian = callingTag.getBooleanAttributeValue(ATTR_VALUE);
  }else if (callingTag.getName() == TAG_PRECONDITIONER) {
    _config.preconditionerType = callingTag.getStringAttributeValue(ATTR_TYPE);
  }
}

void PostProcessingConfiguration:: xmlEndTagCallback
(
  utils::XMLTag& callingTag )
{
  preciceTrace1("xmlEndTagCallback()", callingTag.getName());
  if (callingTag.getNamespace() == TAG){

    //create preconditioner
    if (callingTag.getName() == VALUE_IQNILS || callingTag.getName() == VALUE_MVQN ||  callingTag.getName() == VALUE_ManifoldMapping){
      std::vector<int> dims;
      for (int id : _config.dataIDs){
        for(mesh::PtrMesh mesh : _meshConfig->meshes() ) {
          for (mesh::PtrData data : mesh->data() ) {
            if(data->getID() == id){
              dims.push_back(data->getDimensions());
            }
          }
        }
      }

      if(_config.preconditionerType == VALUE_CONSTANT_PRECONDITIONER){
        std::vector<double> factors;
        for (int id : _config.dataIDs){
          factors.push_back(_config.scalings[id]);
        }
        _preconditioner = impl::PtrPreconditioner(new impl::ConstantPreconditioner(dims, factors));
      }
      else if(_config.preconditionerType == VALUE_VALUE_PRECONDITIONER){
        _preconditioner = impl::PtrPreconditioner (new impl::ValuePreconditioner(dims));
      }
      else if(_config.preconditionerType == VALUE_RESIDUAL_PRECONDITIONER){
        _preconditioner = impl::PtrPreconditioner (new impl::ResidualPreconditioner(dims));
      }
      else if(_config.preconditionerType == VALUE_RESIDUAL_SUM_PRECONDITIONER){
        _preconditioner = impl::PtrPreconditioner (new impl::ResidualSumPreconditioner(dims));
      }
      else{
        // no preconditioner defined
        std::vector<double> factors;
        for (int id = 0; id < _config.dataIDs.size(); ++id) {
          factors.push_back(1.0);
        }
        _preconditioner = impl::PtrPreconditioner (new impl::ConstantPreconditioner(dims, factors));
      }
    }



    if (callingTag.getName() == VALUE_CONSTANT){
      _postProcessing = impl::PtrPostProcessing (
          new impl::ConstantRelaxationPostProcessing (
          _config.relaxationFactor, _config.dataIDs) );
    }
    else if (callingTag.getName() == VALUE_AITKEN){
      _postProcessing = impl::PtrPostProcessing (
          new impl::AitkenPostProcessing(
          _config.relaxationFactor, _config.dataIDs) );
    }
    else if (callingTag.getName() == VALUE_HIERARCHICAL_AITKEN){
      _postProcessing = impl::PtrPostProcessing (
          new impl::HierarchicalAitkenPostProcessing (
          _config.relaxationFactor, _config.dataIDs) );
    }
    else if (callingTag.getName() == VALUE_IQNILS){
      _postProcessing = impl::PtrPostProcessing (
          new impl::IQNILSPostProcessing(
          _config.relaxationFactor,
          _config.forceInitialRelaxation,
          _config.maxIterationsUsed,
          _config.timestepsReused,
          _config.filter, _config.singularityLimit,
          _config.dataIDs,
          _preconditioner) );
    }
    else if (callingTag.getName() == VALUE_MVQN){
		#ifndef PRECICE_NO_MPI
		  _postProcessing = impl::PtrPostProcessing (
			  new impl::MVQNPostProcessing(
			  _config.relaxationFactor,
			  _config.forceInitialRelaxation,
			  _config.maxIterationsUsed,
			  _config.timestepsReused,
			  _config.filter, _config.singularityLimit,
			  _config.dataIDs,
			  _preconditioner) );
		#else
      	  preciceError("xmlEndTagCallback()", "Post processing IQN-IMVJ only works if preCICE is compiled with MPI");
    #endif
    }
    else if (callingTag.getName() == VALUE_ManifoldMapping){

      // create coarse model optimization method recursive
      assertion((_coarseModelOptimizationConfig.get() != nullptr));
      assertion((_coarseModelOptimizationConfig->getPostProcessing().get() != nullptr));

      // create manifold mapping PP
      _postProcessing = impl::PtrPostProcessing (
        new impl::MMPostProcessing(
        _coarseModelOptimizationConfig->getPostProcessing(),                // coarse model optimization method
        _config.maxIterationsUsed,
        _config.timestepsReused,
        _config.filter, _config.singularityLimit,
        _config.estimateJacobian,
        _config.dataIDs,                                                    // fine data IDs
        _coarseModelOptimizationConfig->getPostProcessing()->getDataIDs(),  // coarse data IDs
        _preconditioner) );
    }
    else if (callingTag.getName() == VALUE_BROYDEN){
      _postProcessing = impl::PtrPostProcessing (
          new impl::BroydenPostProcessing(
          _config.relaxationFactor,
          _config.forceInitialRelaxation,
          _config.maxIterationsUsed,
          _config.timestepsReused,
          _config.filter, _config.singularityLimit,
          _config.dataIDs,
          _preconditioner) );
    }
    else {
      assertion(false );
    }
  }
}

void PostProcessingConfiguration:: clear()
{
  _config = ConfigurationData();
  _postProcessing = impl::PtrPostProcessing();
  _neededMeshes.clear();
}

void PostProcessingConfiguration:: addTypeSpecificSubtags
(
  utils::XMLTag& tag )
{
  using namespace utils;
  if(tag.getName() == VALUE_CONSTANT ) {
    XMLTag tagRelax(*this, TAG_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrValue(ATTR_VALUE );
    tagRelax.addAttribute(attrValue );
    tag.addSubtag(tagRelax );

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);
  }
  else if (tag.getName() == VALUE_AITKEN){
    XMLTag tagInitRelax(*this, TAG_INIT_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrValue(ATTR_VALUE );
    tagInitRelax.addAttribute(attrValue );
    XMLAttribute<bool> attrEnforce(ATTR_ENFORCE);
    attrEnforce.setDefaultValue(false);
    tagInitRelax.addAttribute(attrEnforce);
    tag.addSubtag(tagInitRelax);

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);
  }
  else if (tag.getName() == VALUE_HIERARCHICAL_AITKEN){
    XMLTag tagInitRelax(*this, TAG_INIT_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrValue(ATTR_VALUE );
    tagInitRelax.addAttribute(attrValue );
    XMLAttribute<bool> attrEnforce(ATTR_ENFORCE);
    attrEnforce.setDefaultValue(false);
    tagInitRelax.addAttribute(attrEnforce);
    tag.addSubtag(tagInitRelax);

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);
  }
  else if (tag.getName() == VALUE_IQNILS){
    XMLTag tagInitRelax(*this, TAG_INIT_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrDoubleValue(ATTR_VALUE);
    tagInitRelax.addAttribute(attrDoubleValue);
    XMLAttribute<bool> attrEnforce(ATTR_ENFORCE);
    attrEnforce.setDefaultValue(false);
    tagInitRelax.addAttribute(attrEnforce);
    tag.addSubtag(tagInitRelax);

    XMLTag tagMaxUsedIter(*this, TAG_MAX_USED_ITERATIONS, XMLTag::OCCUR_ONCE );
    XMLAttribute<int> attrIntValue(ATTR_VALUE );
    tagMaxUsedIter.addAttribute(attrIntValue );
    tag.addSubtag(tagMaxUsedIter );

    XMLTag tagTimestepsReused(*this, TAG_TIMESTEPS_REUSED, XMLTag::OCCUR_ONCE );
    tagTimestepsReused.addAttribute(attrIntValue );
    tag.addSubtag(tagTimestepsReused );

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    XMLAttribute<double> attrScaling(ATTR_SCALING);
    attrScaling.setDefaultValue(1.0);
    attrScaling.setDocumentation("If the absolute values of two coupling variables"
         " differ too much, a scaling improves the performance of V-IQN-ILS");
    tagData.addAttribute(attrScaling);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);

    XMLTag tagFilter(*this, TAG_FILTER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<std::string> attrFilterName(ATTR_TYPE );
    ValidatorEquals<std::string> validQR1(VALUE_QR1FILTER );
    ValidatorEquals<std::string> validQR1abs(VALUE_QR1_ABSFILTER );
    ValidatorEquals<std::string> validQR2(VALUE_QR2FILTER );
    attrFilterName.setValidator (validQR1 || validQR1abs || validQR2);
   	tagFilter.addAttribute(attrFilterName);
   	XMLAttribute<double> attrSingularityLimit(ATTR_SINGULARITYLIMIT);
    attrSingularityLimit.setDefaultValue(1e-16);
    tagFilter.addAttribute(attrSingularityLimit);
    tagFilter.setDocumentation("Type of filtering technique that is used to "
              "maintain good conditioning in the least-squares system. Possible filters:\n"
              "  QR1-filter: updateQR-dec with (relative) test R(i,i) < eps *||R||\n"
              "  QR1_absolute-filter: updateQR-dec with (absolute) test R(i,i) < eps|\n"
              "  QR2-filter: en-block QR-dec with test |v_orth| < eps * |v|\n"
              "Please note that a QR1 is based on Given's rotations whereas QR2 uses "
              "modified Gram-Schmidt. This can give different results even when no columns "
              "are filtered out.");
    tag.addSubtag(tagFilter);

   	XMLTag tagPreconditioner(*this, TAG_PRECONDITIONER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<std::string> attrPreconditionerType(ATTR_TYPE);
    ValidatorEquals<std::string> valid1 ( VALUE_CONSTANT_PRECONDITIONER);
    ValidatorEquals<std::string> valid2 ( VALUE_VALUE_PRECONDITIONER);
    ValidatorEquals<std::string> valid3 ( VALUE_RESIDUAL_PRECONDITIONER);
    ValidatorEquals<std::string> valid4 ( VALUE_RESIDUAL_SUM_PRECONDITIONER);
    attrPreconditionerType.setValidator ( valid1 || valid2 || valid3 || valid4 );
    attrPreconditionerType.setDocumentation("To improve the performance of a parallel or a multi coupling schemes a preconditioner"
        " can be applied. A constant preconditioner scales every post-processing data by a constant value, which you can define as"
        " an attribute of data. "
        " A value preconditioner scales every post-processing data by the norm of the data in the previous timestep."
        " A residual preconditioner scales every post-processing data by the current residual."
        " A residual-sum preconditioner scales every post-processing data by the sum of the residuals from the current timestep.");
    tagPreconditioner.addAttribute(attrPreconditionerType);
    tag.addSubtag(tagPreconditioner);

  }
  else if (tag.getName() == VALUE_MVQN){
    XMLTag tagInitRelax(*this, TAG_INIT_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrDoubleValue(ATTR_VALUE);
    tagInitRelax.addAttribute(attrDoubleValue);
    XMLAttribute<bool> attrEnforce(ATTR_ENFORCE);
    attrEnforce.setDefaultValue(false);
    tagInitRelax.addAttribute(attrEnforce);
    tag.addSubtag(tagInitRelax);

    XMLTag tagMaxUsedIter(*this, TAG_MAX_USED_ITERATIONS, XMLTag::OCCUR_ONCE );
    XMLAttribute<int> attrIntValue(ATTR_VALUE );
    tagMaxUsedIter.addAttribute(attrIntValue );
    tag.addSubtag(tagMaxUsedIter );

    XMLTag tagTimestepsReused(*this, TAG_TIMESTEPS_REUSED, XMLTag::OCCUR_ONCE );
    tagTimestepsReused.addAttribute(attrIntValue );
    tag.addSubtag(tagTimestepsReused );

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    XMLAttribute<double> attrScaling(ATTR_SCALING);
    attrScaling.setDefaultValue(1.0);
    attrScaling.setDocumentation("If the absolute values of two coupling variables"
         " differ too much, a scaling improves the performance of V-IQN-IMVJ");
    tagData.addAttribute(attrScaling);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);

    XMLTag tagFilter(*this, TAG_FILTER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<double> attrSingularityLimit(ATTR_SINGULARITYLIMIT);
    attrSingularityLimit.setDefaultValue(1e-16);
    tagFilter.addAttribute(attrSingularityLimit);
    XMLAttribute<std::string> attrFilterName(ATTR_TYPE );
    ValidatorEquals<std::string> validQR1(VALUE_QR1FILTER );
    ValidatorEquals<std::string> validQR1abs(VALUE_QR1_ABSFILTER );
    ValidatorEquals<std::string> validQR2(VALUE_QR2FILTER );
    attrFilterName.setValidator (validQR1 || validQR1abs || validQR2);
    tagFilter.addAttribute(attrFilterName);
    tagFilter.setDocumentation("Type of filtering technique that is used to "
	   			"maintain good conditioning in the least-squares system. Possible filters:\n"
	   			"  QR1-filter: updateQR-dec with (relative) test R(i,i) < eps *||R||\n"
	   			"  QR1_absolute-filter: updateQR-dec with (absolute) test R(i,i) < eps|\n"
	   			"  QR2-filter: en-block QR-dec with test |v_orth| < eps * |v|\n"
          "Please note that a QR1 is based on Given's rotations whereas QR2 uses "
          "modified Gram-Schmidt. This can give different results even when no columns "
          "are filtered out.");
    tag.addSubtag(tagFilter);

    XMLTag tagPreconditioner(*this, TAG_PRECONDITIONER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<std::string> attrPreconditionerType(ATTR_TYPE);
    ValidatorEquals<std::string> valid1 ( VALUE_CONSTANT_PRECONDITIONER);
    ValidatorEquals<std::string> valid2 ( VALUE_VALUE_PRECONDITIONER);
    ValidatorEquals<std::string> valid3 ( VALUE_RESIDUAL_PRECONDITIONER);
    ValidatorEquals<std::string> valid4 ( VALUE_RESIDUAL_SUM_PRECONDITIONER);
    attrPreconditionerType.setValidator ( valid1 || valid2 || valid3 || valid4 );
    attrPreconditionerType.setDocumentation("To improve the performance of a parallel or a multi coupling schemes a preconditioner"
        " can be applied. A constant preconditioner scales every post-processing data by a constant value, which you can define as"
        " an attribute of data. "
        " A value preconditioner scales every post-processing data by the norm of the data in the previous timestep."
        " A residual preconditioner scales every post-processing data by the current residual."
        " A residual-sum preconditioner scales every post-processing data by the sum of the residuals from the current timestep.");
    tagPreconditioner.addAttribute(attrPreconditionerType);
    tag.addSubtag(tagPreconditioner);
  }
  else if (tag.getName() == VALUE_ManifoldMapping){

    // add coarse model optimization PostProcessing Tag
    // new PP config for coarse model optimization method (recursive definition)
     // _coarseModelOptimizationConfig->clear();
      if (_coarseModelOptimizationConfig.get() == nullptr) {
        _coarseModelOptimizationConfig = PtrPostProcessingConfiguration(
            new PostProcessingConfiguration(_meshConfig));
      }
      _coarseModelOptimizationConfig->setIsAddManifoldMappingTagAllowed(false);
      _coarseModelOptimizationConfig->connectTags(tag);

    XMLTag tagEstimateJacobian(*this, TAG_ESTIMATEJACOBIAN, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<bool> attrBoolValue(ATTR_VALUE);
    attrBoolValue.setDocumentation("If manifold mapping is used as post-processing one can switch"
                " between explicit estimation and updating of the Jacobian (multi-vector method)"
                " and a matrix free computation. The default is matrix free.");
    tagEstimateJacobian.addAttribute(attrBoolValue);
    tag.addSubtag(tagEstimateJacobian );


    XMLTag tagMaxUsedIter(*this, TAG_MAX_USED_ITERATIONS, XMLTag::OCCUR_ONCE );
    XMLAttribute<int> attrIntValue(ATTR_VALUE );
    tagMaxUsedIter.addAttribute(attrIntValue );
    tag.addSubtag(tagMaxUsedIter );

    XMLTag tagTimestepsReused(*this, TAG_TIMESTEPS_REUSED, XMLTag::OCCUR_ONCE );
    tagTimestepsReused.addAttribute(attrIntValue );
    tag.addSubtag(tagTimestepsReused );

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    XMLAttribute<double> attrScaling(ATTR_SCALING);
    attrScaling.setDefaultValue(1.0);
    attrScaling.setDocumentation("If the absolute values of two coupling variables"
         " differ too much, a scaling improves the performance of V-IQN-IMVJ");
    tagData.addAttribute(attrScaling);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);

    XMLTag tagFilter(*this, TAG_FILTER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<std::string> attrFilterName(ATTR_TYPE );
    ValidatorEquals<std::string> validQR1(VALUE_QR1FILTER );
    ValidatorEquals<std::string> validQR1abs(VALUE_QR1_ABSFILTER );
    ValidatorEquals<std::string> validQR2(VALUE_QR2FILTER );
    attrFilterName.setValidator (validQR1 || validQR1abs || validQR2);
    tagFilter.addAttribute(attrFilterName);
    XMLAttribute<double> attrSingularityLimit(ATTR_SINGULARITYLIMIT);
    attrSingularityLimit.setDefaultValue(1e-16);
    tagFilter.addAttribute(attrSingularityLimit);
    tagFilter.setDocumentation("Type of filtering technique that is used to "
          "maintain good conditioning in the least-squares system. Possible filters:\n"
          "  QR1-filter: updateQR-dec with (relative) test R(i,i) < eps *||R||\n"
          "  QR1_absolute-filter: updateQR-dec with (absolute) test R(i,i) < eps|\n"
          "  QR2-filter: en-block QR-dec with test |v_orth| < eps * |v|\n"
          "Please note that a QR1 is based on Given's rotations whereas QR2 uses "
          "modified Gram-Schmidt. This can give different results even when no columns "
          "are filtered out." );
    tag.addSubtag(tagFilter);

    XMLTag tagPreconditioner(*this, TAG_PRECONDITIONER, XMLTag::OCCUR_NOT_OR_ONCE );
    XMLAttribute<std::string> attrPreconditionerType(ATTR_TYPE);
    ValidatorEquals<std::string> valid1 ( VALUE_CONSTANT_PRECONDITIONER);
    ValidatorEquals<std::string> valid2 ( VALUE_VALUE_PRECONDITIONER);
    ValidatorEquals<std::string> valid3 ( VALUE_RESIDUAL_PRECONDITIONER);
    ValidatorEquals<std::string> valid4 ( VALUE_RESIDUAL_SUM_PRECONDITIONER);
    attrPreconditionerType.setValidator ( valid1 || valid2 || valid3 || valid4 );
    attrPreconditionerType.setDocumentation("To improve the performance of a parallel or a multi coupling schemes a preconditioner"
       " can be applied. A constant preconditioner scales every post-processing data by a constant value, which you can define as"
       " an attribute of data. "
       " A value preconditioner scales every post-processing data by the norm of the data in the previous timestep."
       " A residual preconditioner scales every post-processing data by the current residual."
       " A residual-sum preconditioner scales every post-processing data by the sum of the residuals from the current timestep.");
    tagPreconditioner.addAttribute(attrPreconditionerType);
    tag.addSubtag(tagPreconditioner);

  }
  else if (tag.getName() == VALUE_BROYDEN){
    XMLTag tagInitRelax(*this, TAG_INIT_RELAX, XMLTag::OCCUR_ONCE );
    XMLAttribute<double> attrDoubleValue(ATTR_VALUE);
    tagInitRelax.addAttribute(attrDoubleValue);
    XMLAttribute<bool> attrEnforce(ATTR_ENFORCE);
    attrEnforce.setDefaultValue(false);
    tagInitRelax.addAttribute(attrEnforce);
    tag.addSubtag(tagInitRelax);

    XMLTag tagMaxUsedIter(*this, TAG_MAX_USED_ITERATIONS, XMLTag::OCCUR_ONCE );
    XMLAttribute<int> attrIntValue(ATTR_VALUE );
    tagMaxUsedIter.addAttribute(attrIntValue );
    tag.addSubtag(tagMaxUsedIter );

    XMLTag tagTimestepsReused(*this, TAG_TIMESTEPS_REUSED, XMLTag::OCCUR_ONCE );
    tagTimestepsReused.addAttribute(attrIntValue );
    tag.addSubtag(tagTimestepsReused );

    XMLTag tagData(*this, TAG_DATA, XMLTag::OCCUR_ONCE_OR_MORE );
    XMLAttribute<std::string> attrName(ATTR_NAME);
    XMLAttribute<std::string> attrMesh(ATTR_MESH);
    XMLAttribute<double> attrScaling(ATTR_SCALING);
    attrScaling.setDefaultValue(1.0);
    attrScaling.setDocumentation("If the absolute values of two coupling variables"
         " differ too much, a scaling improves the performance of Broyden post-processing.");
    tagData.addAttribute(attrScaling);
    tagData.addAttribute(attrName);
    tagData.addAttribute(attrMesh);
    tag.addSubtag(tagData);
  }
  else {
    preciceError("addTypeSpecificSubtag()", "Post-processing of type \""
                 << tag.getName() << "\" is unknown!" );
  }
}

}} // namespace precice, cplscheme
