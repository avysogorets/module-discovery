// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilosortedcollectioni.h
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

#ifndef __ADVANCED_ilosortedcollectioniH
#define __ADVANCED_ilosortedcollectioniH

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <ilopl/ilosys.h>
#include <ilopl/ilotuplecollectioni.h>

IloIntArray intersectAscSortedIndex(IloEnv env, IloIntArray set1, IloIntArray set2);
IloNumArray intersectAscSortedIndex(IloEnv env, IloNumArray set1, IloNumArray set2);
IloAnyArray intersectAscSortedIndex(IloEnv env, IloAnyArray set1, IloAnyArray set2);

IloIntArray intersectDescSortedIndex(IloEnv env, IloIntArray set1, IloIntArray set2);
IloNumArray intersectDescSortedIndex(IloEnv env, IloNumArray set1, IloNumArray set2);
IloAnyArray intersectDescSortedIndex(IloEnv env, IloAnyArray set1, IloAnyArray set2);

class IloSortedIntSetI : public IloIntSetI {
  ILORTTIDECL
public:
  
  virtual ~IloSortedIntSetI() {
    if( _oldIndexPositions.getImpl() ) {
      _oldIndexPositions.end();
    }
  }
protected:
  IloSortedIntSetI(IloEnvI* env);
  IloSortedIntSetI(IloEnvI* env, IloSortedIntSetI* S) : IloIntSetI(env, S), _oldIndexPositions( 0 ), _canSort( IloTrue ) {}
public:
  virtual void processBeforeFill() ILO_OVERRIDE {
    _canSort = IloFalse;
  }
  virtual void processAfterFill( IloBool generateOldIndex = IloFalse ) ILO_OVERRIDE {
    _canSort = IloTrue;
    if( generateOldIndex ) {
      initOldIndexes();
    }
    sort();
  }
  virtual void addWithoutSort(IloInt elt) ILO_OVERRIDE { IloIntSetI::add(elt); }

  virtual const IloIntArray getOldIndexPositions() ILO_OVERRIDE { return _oldIndexPositions; }

  virtual void endOldIndexes() ILO_OVERRIDE {
    if( _oldIndexPositions.getImpl() ) {
      _oldIndexPositions.end();
    }
  }
protected:
  IloBool canSort() const {
    return _canSort;
  }
  void initOldIndexes();
  IloIntArray _oldIndexPositions;
  IloBool _canSort;
};

class IloAscSortedIntSetI : public IloSortedIntSetI {
  ILORTTIDECL
public:
  
  virtual ~IloAscSortedIntSetI() {}

  IloAscSortedIntSetI(IloEnvI* env);
  IloAscSortedIntSetI(IloEnvI* env, IloAscSortedIntSetI* S) : IloSortedIntSetI(env, S){}

  virtual IloBool isSortedAsc() const ILO_OVERRIDE { return IloTrue; }
  virtual IloInt getLB() const ILO_OVERRIDE {
    return getFirst();
  }
  virtual IloInt getUB() const ILO_OVERRIDE {
    return getLast();
  }

  virtual void add(IloInt elt) ILO_OVERRIDE;
  virtual void add(IloIntSetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloIntSetI* set) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  virtual void sort(IloBool updateHash = IloTrue) ILO_OVERRIDE;
};

class IloDescSortedIntSetI : public IloSortedIntSetI {
  ILORTTIDECL
public:
  
  virtual ~IloDescSortedIntSetI(){}

  IloDescSortedIntSetI(IloEnvI* env);
  IloDescSortedIntSetI(IloEnvI* env, IloDescSortedIntSetI* S) : IloSortedIntSetI(env, S){}

  virtual IloBool isSortedDesc() const ILO_OVERRIDE { return IloTrue; }
  virtual IloInt getLB() const ILO_OVERRIDE {
    return getFirst();
  }
  virtual IloInt getUB() const ILO_OVERRIDE {
    return getLast();
  }

  virtual void add(IloInt elt) ILO_OVERRIDE;
  virtual void add(IloIntSetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloIntSetI* set) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  virtual void sort(IloBool updateHash = IloTrue) ILO_OVERRIDE;
};

class IloSortedNumSetI : public IloNumSetI {
  ILORTTIDECL
public:
  
  virtual ~IloSortedNumSetI(){
    if( _oldIndexPositions.getImpl() ) {
      _oldIndexPositions.end();
    }
  }
protected:
  IloSortedNumSetI(IloEnvI* env);
  IloSortedNumSetI(IloEnvI* env, IloSortedNumSetI* S) : IloNumSetI(env, S), _oldIndexPositions( 0 ), _canSort( IloTrue ) {}
public:
  virtual void processBeforeFill() ILO_OVERRIDE {
    _canSort = IloFalse;
  }
  virtual void processAfterFill( IloBool generateOldIndex = IloFalse ) ILO_OVERRIDE {
    _canSort = IloTrue;
    if( generateOldIndex ) {
      initOldIndexes();
    }
    sort();
  }
  virtual void addWithoutSort(IloNum elt) ILO_OVERRIDE { IloNumSetI::add(elt); }

  virtual void endOldIndexes() ILO_OVERRIDE {
    if( _oldIndexPositions.getImpl() ) {
      _oldIndexPositions.end();
    }
  }
protected:
  IloBool canSort() const {
    return _canSort;
  }

  void initOldIndexes();
  IloIntArray _oldIndexPositions;
  IloBool _canSort;
};

class IloAscSortedNumSetI : public IloSortedNumSetI {
  ILORTTIDECL
public:
  
  virtual ~IloAscSortedNumSetI(){}

  IloAscSortedNumSetI(IloEnvI* env);
  IloAscSortedNumSetI(IloEnvI* env, IloAscSortedNumSetI* S) : IloSortedNumSetI(env, S){}

  virtual IloBool isSortedAsc() const ILO_OVERRIDE { return IloTrue; }
  virtual IloNum getLB() const ILO_OVERRIDE {
    return getFirst();
  }
  virtual IloNum getUB() const ILO_OVERRIDE {
    return getLast();
  }

  virtual void add(IloNum elt) ILO_OVERRIDE;
  virtual void add(IloNumSetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloNumSetI* set) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  virtual void sort(IloBool updateHash = IloTrue) ILO_OVERRIDE;
};

class IloDescSortedNumSetI : public IloSortedNumSetI {
  ILORTTIDECL
public:
  
  virtual ~IloDescSortedNumSetI(){}

  IloDescSortedNumSetI(IloEnvI* env);
  IloDescSortedNumSetI(IloEnvI* env, IloDescSortedNumSetI* S) : IloSortedNumSetI(env, S){}

  virtual IloBool isSortedDesc() const ILO_OVERRIDE { return IloTrue; }
  virtual IloNum getLB() const ILO_OVERRIDE {
    return getFirst();
  }
  virtual IloNum getUB() const ILO_OVERRIDE {
    return getLast();
  }

  virtual void add(IloNum elt) ILO_OVERRIDE;
  virtual void add(IloNumSetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloNumSetI* set) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  virtual void sort(IloBool updateHash = IloTrue) ILO_OVERRIDE;
};

class IloSortElement;
class IloSortedSymbolSetI : public IloSymbolSetI {
  ILORTTIDECL
public:
  
  virtual ~IloSortedSymbolSetI(){}
protected:
  IloSortedSymbolSetI(IloEnvI* env);
  IloSortedSymbolSetI(IloEnvI* env, IloSortedSymbolSetI* S) : IloSymbolSetI(env, S), _canSort( IloTrue ) {
    setType(IloDataCollection::SymbolSet);
  }
  IloArray<IloSortElement> makeSort();
  IloBool _canSort;

  IloBool canSort() const {
    return _canSort;
  }
public:
  virtual void processBeforeFill() ILO_OVERRIDE {
    _canSort = IloFalse;
  }
  virtual void processAfterFill( IloBool  = IloFalse  ) ILO_OVERRIDE {
    _canSort = IloTrue;
    sort();
  }
  virtual void addWithoutSort(IloAny elt) ILO_OVERRIDE { IloSymbolSetI::add(elt); }
  virtual void addWithoutSort(IloSymbol elt) { addWithoutSort(elt.getImpl()); }
  virtual void sort(IloBool updateHash = IloTrue) ILO_OVERRIDE;
  virtual IloAnyArray makeSortedIndexes(IloArray<IloSortElement> tmp );
};

class IloAscSortedSymbolSetI : public IloSortedSymbolSetI {
  ILORTTIDECL
public:
  
  virtual ~IloAscSortedSymbolSetI(){}

  IloAscSortedSymbolSetI(IloEnvI* env);
  IloAscSortedSymbolSetI(IloEnvI* env, IloAscSortedSymbolSetI* S) : IloSortedSymbolSetI(env, S){}

  virtual IloBool isSortedAsc() const ILO_OVERRIDE { return IloTrue; }

  virtual void add(IloAny elt) ILO_OVERRIDE;
  virtual void add(IloAnySetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloAnySetI* set);
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI* env) const ILO_OVERRIDE;
  virtual IloAnyArray makeSortedIndexes(IloArray<IloSortElement> tmp ) ILO_OVERRIDE;
};

class IloDescSortedSymbolSetI : public IloSortedSymbolSetI {
  ILORTTIDECL
public:
  
  virtual ~IloDescSortedSymbolSetI(){}

  IloDescSortedSymbolSetI(IloEnvI* env);
  IloDescSortedSymbolSetI(IloEnvI* env, IloDescSortedSymbolSetI* S) : IloSortedSymbolSetI(env, S){}

  virtual IloBool isSortedDesc() const ILO_OVERRIDE { return IloTrue; }

  virtual void add(IloAny elt) ILO_OVERRIDE;
  virtual void add(IloAnySetI* set) ILO_OVERRIDE;
  virtual void setIntersection(IloAnySetI* set);
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual IloAnyArray makeSortedIndexes(IloArray<IloSortElement> tmp ) ILO_OVERRIDE;
};

class IloSortedTupleSetI : public IloTupleSetI {
  ILORTTIDECL
protected:
  IloIntArray _sortedIdxes;
  IloIntArray _absIdxes;
  IloBool _canSort;
  IloSortedTupleSetI(IloEnv env, IloTupleSchemaI const *schema)
    : IloTupleSetI(env, schema), _sortedIdxes(env), _absIdxes(env), _canSort( IloTrue ) {
  }
  IloSortedTupleSetI(IloEnvI* env, IloSortedTupleSetI* S);
  IloBool canSort() const {
    return _canSort;
  }
public:
  IloArray<IloSortElement> makeSort();
  virtual void fillSortedIndexes(IloArray<IloSortElement> tmp);
  virtual void sort(IloBool var = IloTrue) ILO_OVERRIDE;

  IloIntArray getSortedIndexes() const{ return _sortedIdxes; }
  IloIntArray getAbsoluteIndexes() const{ return _absIdxes; }
  virtual ~IloSortedTupleSetI(){
    _sortedIdxes.end();
    _absIdxes.end();
  }
  virtual void processBeforeFill() ILO_OVERRIDE {
    _canSort = IloFalse;
  }
  virtual void processAfterFill( IloBool  = IloFalse ) ILO_OVERRIDE {
    _canSort = IloTrue;
    sort();
  }
  virtual IloInt commitWithoutSort(IloTupleCellArray line, IloBool check) ILO_OVERRIDE {
    return IloTupleSetI::commit(line, check);
  }
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloInt getTupleIndexFromAbsoluteIndex(IloInt idx) const ILO_OVERRIDE {
    return idx >=0 && idx < _absIdxes.getSize() ? _absIdxes[idx] : -1;
  }
  virtual IloInt getAbsoluteIndexFromTupleIndex(IloInt idx) const ILO_OVERRIDE {
    return idx >=0 && idx < _sortedIdxes.getSize() ? _sortedIdxes[idx] : -1;
  }
};

class IloAscSortedTupleSetI : public IloSortedTupleSetI {
using IloSortedTupleSetI::commit;
  ILORTTIDECL
public:
  
  IloAscSortedTupleSetI(IloEnv env, IloTupleSchemaI const *schema) : IloSortedTupleSetI(env, schema){}
  IloAscSortedTupleSetI(IloEnvI* env, IloAscSortedTupleSetI* S) : IloSortedTupleSetI(env, S){}

  virtual ~IloAscSortedTupleSetI(){
  }
  virtual IloBool isSortedAsc() const ILO_OVERRIDE { return IloTrue; }

  virtual IloInt commit(IloTupleCellArray line, IloBool check) ILO_OVERRIDE;
  virtual IloInt setLine(IloInt index, IloTupleCellArray line, IloBool check) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual void fillSortedIndexes(IloArray<IloSortElement> tmp ) ILO_OVERRIDE;
};

class IloDescSortedTupleSetI : public IloSortedTupleSetI {
using IloSortedTupleSetI::commit;
  ILORTTIDECL
public:
  
  IloDescSortedTupleSetI(IloEnv env, IloTupleSchemaI const *schema) : IloSortedTupleSetI(env, schema){}
  IloDescSortedTupleSetI(IloEnvI* env, IloDescSortedTupleSetI* S) : IloSortedTupleSetI(env, S){}

  virtual ~IloDescSortedTupleSetI(){
  }
  virtual IloBool isSortedDesc() const ILO_OVERRIDE { return IloTrue; }

  virtual IloInt commit(IloTupleCellArray line, IloBool check) ILO_OVERRIDE;
  virtual IloInt setLine(IloInt index, IloTupleCellArray line, IloBool check) ILO_OVERRIDE;
  virtual IloDataCollectionI* copy() const ILO_OVERRIDE;
  virtual IloDataCollectionI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual void fillSortedIndexes(IloArray<IloSortElement> tmp ) ILO_OVERRIDE;
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
