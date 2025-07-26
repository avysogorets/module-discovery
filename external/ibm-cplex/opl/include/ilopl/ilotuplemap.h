// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilotuplemap.h
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corp. 2000, 2024
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// ---------------------------------------------------------------------------

#ifndef __ADVANCED_ilotuplemapH
#define __ADVANCED_ilotuplemapH

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <ilopl/ilosys.h>

#include <ilopl/iltuple/ilotuplemapi.h>

class IloTupleMap {
private:
protected:
  IloAbstractTupleMapI* _impl;
public:
  typedef IloAbstractTupleMapI ImplClass;
  IloTupleMap(IloAbstractTupleMapI* impl) : _impl(impl) {}
  IloTupleMap() : _impl(0) {}
  IloAbstractTupleMapI* getImpl() const { return _impl; }
  
  IloTupleMap(IloEnv env,
    IloMapIndexer indexer,
    const IloTupleSchema& schema);

  
  void end();
  
  void endElements();
  
  IloEnv getEnv() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getEnv");
    return _impl->getEnv();
  }

  
  IloDiscreteDataCollection getIndexer() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getIndexer");
    return IloDiscreteDataCollection(_impl->getIndexer());
  }
  
  IloDiscreteDataCollection getIndexer(IloInt i) const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getIndexer");
    return IloDiscreteDataCollection(_impl->getIndexer(i));
  }

  
  IloInt getSize() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getSize");
    return _impl->getSize();
  }

  
  IloInt getNbDim() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getNbDim");
    return _impl->getNbDim();
  }

  
  IloInt getTotalSize() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getTotalSize");
    return _impl->getTotalSize();
  }
  
  IloInt getNonEmptySlotSize() const {
    IloOplAssert(_impl, "Empty Handle in IloTupleMap::getTotalSize");
    return _impl->getNonEmptySlotSize();
  }
  
  IloMapIndexer makeMapIndexer() const;

  
  IloAbstractTupleMapI* copy() const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::copy");
    return _impl->getCopy();
  }

    
  IloTupleBuffer makeTupleBuffer() const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::makeTupleBuffer");
    return _impl->makeTupleBuffer();
  }

    
  IloTuple makeTuple() const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::makeTuple");
    return _impl->makeTuple();
  }

  
  const char * getName() const {
    if (getImpl() == 0)
      throw IloEmptyHandleException("IloTupleMap::getName : Using empty handle");
    return getImpl()->getName();
  }
  
  void setName(const char * name) const {
    if (getImpl() == 0)
      throw IloEmptyHandleException("IloTupleMap::setName : Using empty handle");
    getImpl()->setName(name);
  }

  
  void setAt(IloMapIndexArray indices, IloTuple value) {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::setAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::setAt number of dimensions mismatch");
    _impl->setAt(indices, value);
  }

  
  void setAt(IloMapIndexArray indices, IloTupleBuffer value) {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::setAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::setAt number of dimensions mismatch");
    _impl->setAt(indices, value);
  }
  
  void setAt(IloMapIndexArray indices, IloOplObject value) {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::setAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::setAt number of dimensions mismatch");
    _impl->setAt(indices, value);
  }
  
  IloOplObject getAt(IloMapIndexArray indices) const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::getAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::getAt number of dimensions mismatch");
    return _impl->getAt(indices);
  }

  
  void getAt(IloMapIndexArray indices, IloTuple tuple) const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::getAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::getAt number of dimensions mismatch");
    IloOplAssert( 0 != tuple.getImpl(), "Empty handle for tuple. You should call makeTuple() first." );
    _impl->getAt(indices, tuple);
  }

  
  void getAt(IloMapIndexArray indices, IloTupleBuffer buffer) const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::getAt");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::getAt number of dimensions mismatch");
    IloOplAssert( 0 != buffer.getImpl(), "Empty handle for buffer. You should call makeTupleBuffer() first." );
    _impl->getAt(indices, buffer);
  }

  void getAtAbsoluteIndex(IloIntFixedArray indices, IloTuple tuple) const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::getAtAbsoluteIndex");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::getAtAbsoluteIndex number of dimensions mismatch");
    _impl->getAtAbsoluteIndex(indices, tuple);
  }

  void getAtAbsoluteIndex(IloIntFixedArray indices, IloTupleBuffer buffer) const {
    IloOplAssert (_impl, "Empty Handle in IloTupleMap::getAtAbsoluteIndex");
    IloOplAssert (getNbDim() == indices.getSize(),
      "IloTupleMap::getAtAbsoluteIndex number of dimensions mismatch");
    _impl->getAtAbsoluteIndex(indices, buffer);
  }

  
  IloTupleSchema getSchema() const {
    IloOplAssert(getImpl(), "IloTupleMap::getSchema using empty handle");
    return IloTupleSchema(getImpl()->getSchema());
  }

  DEFINE_EXPR_ACCESSORS(IloTupleSubMapExpr)

  
  IloBool isDefaultValue(IloTuple tuple){
    IloOplAssert(getImpl(), "IloTupleMap::isDefaultValue using empty handle");
    return getImpl()->isDefaultValue(tuple );
  }
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
