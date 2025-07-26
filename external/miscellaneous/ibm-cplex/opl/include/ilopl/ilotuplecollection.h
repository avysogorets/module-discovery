// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilotuplecollection.h
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

#ifndef __ADVANCED_ilotuplecollectionH
#define __ADVANCED_ilotuplecollectionH

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <ilopl/ilosys.h>
#include <ilconcert/iloextractable.h>
#include <ilopl/iltuple/ilodatacolumni.h>
#include <ilopl/iltuple/ilotuplebuffer.h>
#include <ilconcert/iloanyexpri.h>

class IloTuple;
class IloTupleBuffer;
class IloTupleSetI;
class IloTupleCollectionI;
class IloTupleSchemaI;
class IloTupleRequestI;
class IloTupleRequest;
class IloTupleSchema;
class IloTupleIndex;
class IloTuplePattern;
class IloGenerator;
class IloTupleIterator;
class IloTupleExprArg;
class IloTupleSetMap;

class IloColumnDefinitionI;
class IloTupleSetArray;
class IloSymbolArray;

class IloTupleSchema  {
private:
  IloTupleSchemaI const *_impl;
public:
  
  IloTupleSchema(IloTupleSchemaI const *impl = 0);

  
  IloTupleSchemaI const *getImpl() const {
    return _impl;
  }

  
  void end();

 
  IloInt getSize() const;

 
  const char* getColumnName(IloInt idx) const;

 
  IloEnv getEnv() const;

 
  void setName(const char* name);

 
  const char* getName() const;

 
  IloInt getTotalColumnNumber() const;

 
  IloInt getColumnIndex(const char* name) const;

 
  IloBool isInt(IloInt index) const;

 
  IloBool isNum(IloInt index) const;

 
  IloBool isSymbol(IloInt index) const;

 
  IloBool isTuple(IloInt index) const;

 
  IloBool isIntCollection(IloInt index) const;

 
  IloBool isNumCollection(IloInt index) const;

 
  IloBool isAnyCollection(IloInt index) const;

 
  IloBool isInt(IloIntArray path) const;

 
  IloBool isNum(IloIntArray path) const;

 
  IloBool isIntCollection(IloIntArray path) const;

 
  IloBool isNumCollection(IloIntArray path) const;

 
  IloBool isSymbol(IloIntArray path) const;

 
  IloBool isTuple(IloIntArray path) const;

  bool operator==(IloTupleSchema const &other) const;
  bool operator!=(IloTupleSchema const &other) const;

  
  IloTupleSchema getTupleColumn(IloInt colIndex) const;

  
  IloIntArray getSharedPathFromAbsolutePosition(IloInt position) const;
};

ILOSTD(ostream)& operator<<(ILOSTD(ostream)& out, const IloTupleSchema& s);

class IloTupleCollection : public IloAnyCollection {
public:
  
  class DuplicatedException : public IloException {
  private:
    IloTupleCollectionI* _set;
    IloInt _idx;
    IloTupleCellArray _duplicate;
  public:
    DuplicatedException(const char* message, IloTupleCollectionI* set, IloTupleCellArray duplicate, IloInt index);

    DuplicatedException( const DuplicatedException& dup );

    virtual const char* getMessage() const ILO_OVERRIDE;
    
    void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
    ~DuplicatedException();
    
    IloTupleCollectionI* getTupleCollection() const { return _set; }
    IloInt getIndex() const { return _idx; }
    
    IloTupleCellArray getTupleCellArray() const { return _duplicate; }
  };

  
  class DuplicatedKey : public DuplicatedException {
  public:
    DuplicatedKey(IloTupleCollectionI* set, IloTupleCellArray duplicate, IloInt index);
    ~DuplicatedKey(){}
  };

  
  class DuplicatedTuple : public DuplicatedException {
  public:
    DuplicatedTuple(IloTupleCollectionI* set, IloTupleCellArray duplicate, IloInt index);
    ~DuplicatedTuple(){}
  };

  
  class UnknownReference : public IloException {
  private:
    IloTupleCollectionI* _set;
    IloTupleCollectionI* _reference;
    IloTupleCellArray _cells;
  public:
    UnknownReference(IloTupleCollectionI* set, IloTupleCollectionI* ref, IloTupleCellArray duplicate);
    UnknownReference( const UnknownReference& dup );
    virtual const char* getMessage() const ILO_OVERRIDE;
    
    void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
    ~UnknownReference();
    
    IloTupleCollectionI* getTupleCollection() const { return _set; }
    
    IloTupleCollectionI* getReference() const { return _reference; }
    IloTupleCellArray getTupleCellArray() const { return _cells; }
  };

public:
  
  IloTupleCollection(IloTupleCollectionI* impl);
  
  IloTupleCollectionI* getImpl() const;
  IloTupleCollection(){ _impl = 0; }
  
  IloTupleCellArray getOrMakeSharedKeyCells(IloInt line);
  
  IloTupleCellArray getOrMakeSharedTupleCells(IloInt line);
};

class IloTupleSet : public IloTupleCollection {
public:
 
  IloTupleSchema getSchema() const;

 
  void setName(const char* name);

 
  const char* getName() const;

  IloTupleSet(){ _impl = 0; }

 
  IloTupleSet(IloTupleSetI* impl);

 
  IloTupleSet(IloEnv env, IloTupleSchema const &schema);
  IloTupleSet(IloEnv env, IloTupleSchema const &schema, IloDataCollection::SortSense sense);

 
  IloTuple makeNext(IloTuple value, IloInt n=0) const;

 
  IloTuple makePrevious(IloTuple value, IloInt n=0) const;

 
  IloTuple makeNextC(IloTuple value, IloInt n=0) const;

 
  IloTuple makePreviousC(IloTuple value, IloInt n=0) const;

 
  IloTuple makeFirst() const;

 
  IloTuple makeLast() const;

 
  IloTupleSetI* getImpl() const;

 
  const char* getColumnName(IloInt index) const;

 
  const char* getColumnName(IloIntArray path) const;

 
  void setColumnName(IloInt index, const char* name);

 
  void setColumnName(IloIntArray path, const char* name);

 
  IloTuple makeTuple(IloInt index) const;

 
  IloTupleBuffer makeTupleBuffer(IloInt index = -1) const;

 
  IloTuplePathBuffer makeLine(IloInt index) const;

 
  IloInt commit(IloTupleBuffer line, IloBool check = IloTrue);

 
  IloInt getLength() const;

 
  IloInt getSize() const;

 
  IloTupleIterator* iterator(IloGenAlloc* heap);

  
  IloTupleIterator* iterator();

 
  void clearSelectIndexes();

 
  void createSelectIndexes();

 
  void displayRow(IloInt i, ILOSTD(ostream)& out) const;

 
  IloBool isIn(IloTupleBuffer buffer);
 
  IloTuple find(IloTupleBuffer buffer);
 
  IloInt getIndex(IloTuple tuple) const;

  
  IloInt commit2HashTable(IloTupleCellArray array, IloBool check);
  
  void fillColumns();
  
  IloInt commit(IloTupleCellArray line, IloBool check);
  
  IloInt getTupleIndexFromAbsoluteIndex(IloInt idx) const;
};

class IloTupleIterator : public IloAnyDataIterator {
private:
  IloTupleRequestI* _request;
  IloIntArrayI* _reqResult;
  IloBool _ownsReqResult;
  IloInt _index;
  void initSlice();
public:
  virtual void setCollection(const IloDiscreteDataCollectionI* coll) ILO_OVERRIDE;
   
  IloInt getIndex() const {
    return _index;
  }
protected:
   
  IloTupleIterator(IloGenAlloc* heap);

   
        void initCollection(IloTupleSetI*);
public:
  virtual ~IloTupleIterator();

   
  IloTupleIterator(IloGenAlloc* heap, IloTupleSet coll);

   
  IloTupleIterator(IloTupleSet coll);

   
  IloTupleIterator(IloGenAlloc* heap, IloTupleSet coll, IloTupleRequest req, IloBool computeSlice=IloTrue);

   
  IloTupleIterator(IloTupleSet coll, IloTupleRequest req, IloBool computeSlice=IloTrue);

  
  IloTupleSet getTupleSet() const;

   
  virtual IloTupleSchemaI const *getSchema() const;

   
  void initRequest(IloTupleRequest req, IloBool computeSlice=IloTrue);

   
  void clearReqResult();

   
  virtual IloBool next() ILO_OVERRIDE;

   
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;

   
  virtual void reset(IloAny val, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;

#define AFC_FIX_TUPLEITERATOR
#ifdef AFC_FIX_TUPLEITERATOR
  
  IloTuple operator*();
#endif

#ifdef CPPREF_GENERATION
  
  IloBool ok() const {
    return _ok;
  }

  
  void operator++() {
    _ok = next();
  }
#endif
};

IloConstraint IloSubset(IloEnv env, IloTupleSetExprArg slice, IloTupleSetExprArg set);
IloConstraint IloSubsetEq(IloEnv env, IloTupleSetExprArg slice, IloTupleSetExprArg set);

IloIntExprArg IloOrd (IloTupleSetExprArg map, IloTupleExprArg y);

IloConstraint IloOrdered(IloTupleSetExprArg coll, IloTupleExprArg exp1, IloTupleExprArg exp2);

IloTupleExprArg IloPreviousC(IloTupleSetExprArg set, IloTupleExprArg value, IloIntExprArg n);

IloTupleExprArg IloNextC(IloTupleSetExprArg set, IloTupleExprArg value, IloIntExprArg n);

IloTupleExprArg IloPrevious(IloTupleSetExprArg set, IloTupleExprArg value, IloIntExprArg n);

IloTupleExprArg IloNext(IloTupleSetExprArg set, IloTupleExprArg value, IloIntExprArg n);

IloTupleExprArg IloItem(IloTupleSetExprArg set, IloIntExprArg n);

IloTupleExprArg IloItem(IloTupleSetExprArg set, IloTupleExprArg n);

IloTupleExprArg IloPreviousC(IloTupleSetExprArg set, IloTupleExprArg value);

IloTupleExprArg IloNextC(IloTupleSetExprArg set, IloTupleExprArg value);

IloTupleExprArg IloPrevious(IloTupleSetExprArg set, IloTupleExprArg value);

IloTupleExprArg IloNext(IloTupleSetExprArg set, IloTupleExprArg value);

IloTupleExprArg IloFirst(IloTupleSetExprArg set);

IloTupleExprArg IloLast(IloTupleSetExprArg set);

IloTupleSetExprArg IloSymExclude(IloTupleSetExprArg expr1,IloTupleSetExprArg expr2);

IloTupleSetExprArg IloUnion(IloTupleSetExprArg expr1, IloTupleSetExprArg expr2);

IloTupleSetExprArg IloExclude(IloTupleSetExprArg expr1, IloTupleSetExprArg expr2);

IloTupleSetExprArg IloInter(IloTupleSetExprArg expr1, IloTupleSetExprArg expr2);

IloIntExprArg IloCard(const IloTupleSetExprArg e);

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
