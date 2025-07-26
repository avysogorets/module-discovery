// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/iltuple/ilocollectioncolumni.h
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

#ifndef __ADVANCED_ilocollectioncolumniH
#define __ADVANCED_ilocollectioncolumniH

#ifdef _WIN32
#pragma pack(push, 8)
#endif
#include <ilopl/ilosys.h>

#include <ilopl/iltuple/ilodatacolumni.h>

extern IloInt  IloIntColHashFunction(IloAny key, IloInt size);
extern IloInt  IloNumColHashFunction(IloAny key, IloInt size);
extern IloInt  IloAnyColHashFunction(IloAny key, IloInt size);

class IloIntMap;
class IloNumMap;
class IloAnyMap;

class IloMapI;
class IloObjectBase;

class IloIntMapAsCollectionI : public IloIntCollectionI {
  ILORTTIDECL
private:
  IloMapI* _map;
public:
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  IloIntMapAsCollectionI(IloEnvI* env, const IloIntMap map);
  IloIntMap getMap() const;
  IloDiscreteDataCollectionI* getIndexer() const;
  virtual ~IloIntMapAsCollectionI();
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE;
  virtual IloInt getSize() const ILO_OVERRIDE;
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;
  virtual IloIntArray getArray() const ILO_OVERRIDE;
  virtual IloBool contains(IloInt e) const ILO_OVERRIDE;
  virtual IloInt getValue(IloInt index) const ILO_OVERRIDE;
  virtual IloDataIterator* iterator(IloGenAlloc* heap) const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& os) const ILO_OVERRIDE;
  virtual IloBool isMapAsCollection() const ILO_OVERRIDE { return IloTrue; }
};

class IloNumMapAsCollectionI : public IloNumCollectionI {
  ILORTTIDECL
private:
  IloMapI* _map;
public:
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  IloNumMapAsCollectionI(IloEnvI* env, const IloNumMap map);
  IloNumMap getMap() const;
  IloDiscreteDataCollectionI* getIndexer() const;
  virtual ~IloNumMapAsCollectionI();
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE;
  virtual IloInt getSize() const ILO_OVERRIDE;
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;
  virtual IloNumArray getArray() const ILO_OVERRIDE;
  virtual IloBool contains(IloNum e) const ILO_OVERRIDE;
  virtual IloNum getValue(IloInt index) const ILO_OVERRIDE;
  virtual IloDataIterator* iterator(IloGenAlloc* heap) const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& os) const ILO_OVERRIDE;
  virtual IloBool isMapAsCollection() const ILO_OVERRIDE { return IloTrue; }
};

class IloCollectionColumnI : public IloAnyDataColumnI {
  IloBool _mustDelete;
protected:
  IloDiscreteDataCollectionI* _indexer; 
  ILORTTIDECL
protected:
  virtual void updateHashForSelect(IloInt index, IloAny value, IloBool addIndex = IloTrue) ILO_OVERRIDE;
public:
  using IloAnyDataColumnI::getIndex;
  
  IloCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer, const IloAnyArray array, IloDiscreteDataCollectionI* defaultValue = 0);
  IloCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer): IloAnyDataColumnI(env), _mustDelete(IloTrue), _indexer(indexer) {}
  virtual ~IloCollectionColumnI();
  virtual void add(IloAny elt) ILO_OVERRIDE;
  virtual void add(IloAnyDataColumnI* set) ILO_OVERRIDE;
  virtual void discard(IloAny value) ILO_OVERRIDE;
  virtual void remove(IloInt index) ILO_OVERRIDE;
  virtual void setValue(IloInt index, IloAny value) ILO_OVERRIDE;
  virtual void empty() ILO_OVERRIDE;
  virtual void setDefaultValue(const IloAny coll) ILO_OVERRIDE;
  void mustDelete(IloBool flag){ _mustDelete = flag; }
  virtual IloInt getIndex(IloAny val) const ILO_OVERRIDE;
  virtual IloIntArray makeIndexArray(IloAny value) const ILO_OVERRIDE;
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE {
    throw IloWrongUsage("IloCollectionColumnI does not have a Data Type");
    ILOUNREACHABLE(return IloDataCollection::IloDataType(0);)
  }
  IloBool isSetColumn() const { return _indexer==0;}
  IloBool compareIndexer(IloDiscreteDataCollectionI* coll) const;
  virtual void checkBeforeUsing(IloDiscreteDataCollectionI* coll) const = 0;
  
  
};

typedef IloArray<IloTupleCellArrayI*> IloTrial;

class IloTupleRefDataColumnI : public IloIntDataColumnI {
  ILORTTIDECL
private:
  IloBool _checkReference;
  IloTrial _hashForKeys;
public:
  using IloIntDataColumnI::getIndex;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;

  IloTupleRefDataColumnI(IloEnvI* env, IloTupleSetI* refered, IloInt n, IloBool checkReferences);
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;

  IloBool checkReferences() const { return _checkReference;};
  void setCheckReferences(IloBool flag) { _checkReference = flag; };
  IloInt commit(IloTupleCellArray line);
  IloBool setLine(IloInt, IloTupleCellArray line);

  IloTupleCellArray getOrMakeSharedTupleCells(IloTuplePathBuffer);
  IloTupleCellArray getOrMakeEmptySharedTupleCells();
  IloTupleCellArray getOrMakeSharedTupleCells(IloInt line);
  IloTupleCellArray getOrMakeSharedKeyCells(IloInt line);
  IloTupleCellArray getOrMakeSharedKeyCells(IloTupleCellArray);
  IloTupleCellArray getOrMakeEmptySharedKeyCells();
  void addTupleCells(IloTupleCellArray array, IloInt line) const;

  IloInt getTupleIndex(IloTupleCellArray array);
  inline IloTupleSetI* getTupleCollection() const{return (IloTupleSetI*)(void*)_refered;}
  IloTrial getHashForKeys() const{ return _hashForKeys;}

  IloInt getWidth() const;
  virtual ~IloTupleRefDataColumnI();
  virtual void empty() ILO_OVERRIDE;
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE {
    return IloDataCollection::TupleRefColumn;
  }
  virtual IloBool isIntDataColumn() const ILO_OVERRIDE { return IloFalse;}
  virtual IloBool isTupleRefColumn() const ILO_OVERRIDE { return IloTrue;}
  virtual void display(ILOSTD(ostream)& os) const ILO_OVERRIDE;
  virtual void displayKeys(ILOSTD(ostream)& os) const;
  virtual void remove(IloInt index) ILO_OVERRIDE;
  virtual void remove(IloTupleCellArray cells);
};

class IloIntCollectionColumnI : public IloCollectionColumnI {
  ILORTTIDECL
public:
  IloIntCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer, const IloAnyArray array, IloDiscreteDataCollectionI* defaultFalue = 0);
  IloIntCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer, IloInt n=0);
  virtual IloBool isIntCollectionColumn() const ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE {
    return IloDataCollection::IntCollectionColumn;
  }
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;
  virtual void checkBeforeUsing(IloDiscreteDataCollectionI* coll) const ILO_OVERRIDE;
  virtual IloAny getValue(IloInt index) const ILO_OVERRIDE;
  virtual void enableSelectIndexes() ILO_OVERRIDE {
    if (!_hashForSelect) _hashForSelect = new (getEnv()) IloAnyDataTableHash(getEnv(), IloIntColHashFunction, IloIntCollectionCompFunction);
  }
};

class IloNumCollectionColumnI : public IloCollectionColumnI {
  ILORTTIDECL
public:
  IloNumCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer, const IloAnyArray array, IloDiscreteDataCollectionI* defaultFalue = 0);
  IloNumCollectionColumnI(IloEnvI* env, IloDiscreteDataCollectionI* indexer, IloInt n=0);
  virtual IloBool isNumCollectionColumn() const ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE {
    return IloDataCollection::NumCollectionColumn;
  }
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;
  virtual void checkBeforeUsing(IloDiscreteDataCollectionI* coll) const ILO_OVERRIDE;
  virtual IloAny getValue(IloInt index) const ILO_OVERRIDE;
  virtual void enableSelectIndexes() ILO_OVERRIDE {
    if (!_hashForSelect) _hashForSelect = new (getEnv()) IloAnyDataTableHash(getEnv(), IloNumColHashFunction, IloNumCollectionCompFunction);
  }
};

class IloAnyCollectionColumnI : public IloCollectionColumnI {
  ILORTTIDECL
public:
  IloAnyCollectionColumnI(IloEnvI* env, const IloAnyArray array, IloDiscreteDataCollectionI* defaultValue = 0);
  IloAnyCollectionColumnI(IloEnvI* env, IloInt n=0);
  virtual IloBool isAnyCollectionColumn() const ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  
  virtual IloDataCollection::IloDataType getDataType() const ILO_OVERRIDE {
    return IloDataCollection::AnyCollectionColumn;
  }
  virtual IloObjectBase getMapItem(IloInt idx) const ILO_OVERRIDE;
  virtual void checkBeforeUsing(IloDiscreteDataCollectionI* ) const ILO_OVERRIDE {}
  virtual IloAny getValue(IloInt index) const ILO_OVERRIDE;
  virtual void enableSelectIndexes() ILO_OVERRIDE {
    if (!_hashForSelect) _hashForSelect = new (getEnv()) IloAnyDataTableHash(getEnv(), IloAnyColHashFunction, IloAnyCollectionCompFunction);
  }
};

class IloCollectionUtil {
public:
  static IloIntMap getMap(IloIntCollection coll);
  static IloNumMap getMap(IloNumCollection coll);
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
