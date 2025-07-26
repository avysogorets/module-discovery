// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilomapi.h
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

#ifndef __ADVANCED_ilomapiH
#define __ADVANCED_ilomapiH

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <ilopl/ilosys.h>
#include <ilopl/ilohash.h>

#include <ilconcert/ilocollection.h>
#include <ilopl/ilocollexprbase.h>
#include <ilopl/ilotuple.h>
#include <ilconcert/iloextractable.h>
#include <ilconcert/iloexpressioni.h>
#include <ilopl/ilosoplcp.h>
#include <ilconcert/ilomapi.h>

class IloMapI;
class IloAbstractTupleMapI;
class IloAbstractMapI;

class IloMapI;

class IloOplObject : public IloObjectBase {
public:
private:
  IloOplObject() {
  }
public:
  virtual ~IloOplObject() {}
  void operator= (const IloOplObject& i);
  
  IloOplObject(IloOplObject::Type t, IloAny e);
  IloOplObject(const IloOplObject& x);

  
  IloOplObject(const IloObjectBase x) : IloObjectBase( x ) {
  }

  
  IloOplObject(IloInt x):IloObjectBase(x){}
  
  IloOplObject(IloNum x):IloObjectBase(x){}
  
  IloOplObject(const char* x):IloObjectBase(x){}
  
  IloOplObject(IloSymbol x):IloObjectBase(x){}

  
  IloOplObject(IloIntExprArg x);
  
  IloOplObject(IloIntVar x);
  
  IloOplObject(IloIntIndex x);
  
  IloOplObject(IloNumExprArg x);
  
  IloOplObject(IloNumVar x);
  
  IloOplObject(IloNumIndex x);
  
  IloOplObject(IloSymbolExprArg x);
  
  IloOplObject(IloSymbolIndex x);
  
  IloOplObject(IloTupleIndex x);
  
  IloOplObject(IloTupleExprArg x);
  
  IloOplObject(IloTuple x);
  
  IloOplObject(IloTuplePattern x);

  IloOplObject(IloDiscreteDataCollection x);

  IloOplObject(IloIntCollectionExprArg x);
  IloOplObject(IloIntCollectionIndex x);
  IloOplObject(IloIntCollection x);
  IloOplObject(IloNumRange x);
  IloOplObject(IloNumCollectionExprArg x);
  IloOplObject(IloNumCollectionIndex x);
  IloOplObject(IloNumCollection x);

  IloOplObject(IloSymbolCollectionExprArg x);
  IloOplObject(IloTupleSetExprArg x);
  IloOplObject(IloSymbolCollectionIndex x);
  
  IloOplObject(IloAnyCollection x);

  
  IloOplObject(IloConstraint x);
  IloOplObject(IloIntervalVar x);
  IloOplObject(IloPiecewiseFunctionExpr x);
  IloOplObject(IloIntervalSequenceVar x);
  IloOplObject(IloCumulFunctionExpr x);
  IloOplObject(IloStateFunctionExpr x);
  
  IloOplObject(IloExtractable x);
  IloOplObject(IloMapI* x);

  
  IloOplObject getClone(IloEnvI* env);

  
  IloIntCollectionI* asIntCollection() const {
    IloOplAssert(_type == IntCollectionConst, "Map Item is not a collection of int.");
    return (IloIntCollectionI*)_data._any;
  }

  
  IloIntSet asIntSet() const {
    IloOplAssert(_type == IntCollectionConst, "Map Item is not a collection of int.");
    IloIntCollectionI* res = (IloIntCollectionI*)_data._any;
    if (res!=NULL) IloTestAndRaise(res->getDataType() == IloDataCollection::IntSet, "Map Item is not an intSet");
    return (IloIntSetI*)res;
  }

  
  IloNumCollectionI* asNumCollection() const {
    IloOplAssert(_type == NumCollectionConst, "Map Item is not a collection of num.");
    return (IloNumCollectionI*)_data._any;
  }
  
  IloNumSet asNumSet() const {
    IloOplAssert(_type == NumCollectionConst, "Map Item is not a collection of int.");
    IloNumCollectionI* res = (IloNumCollectionI*)_data._any;
    if (res!=NULL) IloTestAndRaise(res->getDataType() == IloDataCollection::NumSet, "Map Item is not a numSet");
    return (IloNumSetI*)res;
  }

  void getValue(IloIntCollection& res) { res = asIntCollection(); }
  void getValue(IloNumCollection& res) { res = asNumCollection(); }
  void getValue(IloAnyCollection& res) { res = asAnyCollection(); }
  
  IloAnyCollectionI* asAnyCollection() const {
    IloOplAssert(_type == SymbolCollectionConst || _type==TupleCollectionConst, "Map Item is not a collection of any.");
    return (IloAnyCollectionI*)_data._any;
  }
  
  IloSymbolSet asSymbolSet() const {
    IloOplAssert(_type == SymbolCollectionConst, "Map Item is not a collection of int.");
    IloAnyCollectionI* res = (IloAnyCollectionI*)_data._any;
    if (res!=NULL) IloTestAndRaise(res->isSymbolSet(), "Map Item is not a symbolSet");
    return (IloAnySetI*)res;
  }
  
  IloTupleCollectionI* asTupleCollection() const {
    IloOplAssert(_type == TupleCollectionConst, "Map Item is not a collection of int.");
    IloAnyCollectionI* res = (IloAnyCollectionI*)_data._any;
    if (res!=NULL) IloTestAndRaise(res->isTupleCollection(), "Map Item is not a tupleCollection");
    return (IloTupleCollectionI*)res;
  }
  
  IloTupleSet asTupleSet() const {
    IloOplAssert(_type == TupleCollectionConst, "Map Item is not a collection of int.");
    IloAnyCollectionI* res = (IloAnyCollectionI*)_data._any;
    if (res!=NULL) IloTestAndRaise(res->isTupleSet(), "Map Item is not a tupleSet");
    return (IloTupleSetI*)res;
  }

  
  IloTuple asTuple() const {
    IloOplAssert(_type == TupleConst, "Map Item is not a tuple.");
    return (IloTupleI*)_data._any;
  }

  void getValue(IloExtractable& res) { res = asExtractable(); }
  
  IloIntExprArg asIntExpr() const {
    IloOplAssert(isInt() && isExtractable(), "Map Item is not an integer expression.");
    return (IloIntExprI*)_data._any;
  }
  
  IloNumExprArg asNumExpr() const {
    IloOplAssert(isNum() && isExtractable(), "Map Item is not a numeric expression.");
    return (IloNumExprI*)_data._any;
  }
  
  IloIntCollectionExprI* asIntCollectionExpr() const {
    IloOplAssert(isIntCollection() && isExtractable(), "Map Item is not an intcollection expression.");
    return (IloIntCollectionExprI*)_data._any;
  }
  
  IloNumCollectionExprI* asNumCollectionExpr() const {
    IloOplAssert(isNumCollection() && isExtractable(), "Map Item is not a numcollection expression.");
    return (IloNumCollectionExprI*)_data._any;
  }
  
  IloSymbolCollectionExprI* asSymbolCollectionExpr() const {
    IloOplAssert(isSymbolCollection() && isExtractable(), "Map Item is not an anycollection expression.");
    return (IloSymbolCollectionExprI*)_data._any;
  }
  
  IloTupleSetExprI* asTupleSetExpr() const {
    IloOplAssert(isTupleCollection() && isExtractable(), "Map Item is not an anycollection expression.");
    return (IloTupleSetExprI*)_data._any;
  }

  
  IloTuplePatternI* asPattern() const {
    IloOplAssert(isPattern(), "Map Item is not a tuple pattern.");
    return (IloTuplePatternI*)_data._any;
  }
  
  IloTupleIndexI* asTupleIndex() const {
    IloOplAssert(_type == TupleIndex, "Map Item is not a tuple index.");
    return (IloTupleIndexI*)_data._any;
  }
  IloTupleExprI* asTupleExpr() const {
    IloOplAssert(isAny() && isExtractable() && !isConstraint(), "Map Item is not an anyexpression.");
    return (IloTupleExprI*)_data._any;
  }
  IloSymbolExprArg asSymbolExpr() const {
    IloOplAssert(isAny() && isExtractable() && !isConstraint(), "Map Item is not an anyexpression.");
    return (IloSymbolExprI*)_data._any;
  }
  
  IloAnyExprI* asAnyExpr() const {
    IloOplAssert(isAny() && isExtractable() && !isConstraint(), "Map Item is not an anyexpression.");
    return (IloAnyExprI*)_data._any;
  }
  
  IloConstraint asConstraint() const {
    IloOplAssert(isConstraint(), "Map Item is not a constraint.");
    return (IloConstraintI*)_data._any;
  }

  IloIntervalVarI* asIntervalExpr() const {
    IloOplAssert(isInterval(), "Map Item is not an interval.");
    return (IloIntervalVarI*)_data._any;
  }

  IloAdvPiecewiseFunctionI* asPiecewiseFunction() const {
    IloOplAssert(isPiecewiseFunction(), "Map Item is not a piecewise function.");
    return (IloAdvPiecewiseFunctionI*)_data._any;
  }

  IloIntervalSequenceVarI* asSequenceExpr() const {
    IloOplAssert(isSequence(), "Map Item is not a sequence.");
    return (IloIntervalSequenceVarI*)_data._any;
  }

  IloCumulFunctionExprI* asCumulFunctionExpr() const {
    IloOplAssert(isCumulFunctionExpr(), "Map Item is not a cumul-function-expr.");
    return (IloCumulFunctionExprI*)_data._any;
  }
  IloStateFunctionExprI* asStateFunctionExpr() const {
    IloOplAssert(isStateFunctionExpr(), "Map Item is not a state-function-expr.");
    return (IloStateFunctionExprI*)_data._any;
  }

#ifdef ILO_WIN64
  IloNum asNum() const;
#endif

  
  IloMapI* asSubMap() const {
    if (!isSubMap()) {
      throw IloWrongUsage("Map Item is not a submap.");
    }
    IloOplAssert(isSubMap(), "Map Item is not a submap.");
    return (IloMapI*)_data._any;
  }
  void getValue(IloMapI*& res) { res = asSubMap(); }
  void getValue(IloSymbol& res) { res = asSymbol(); }
  
  void display(ILOSTD(ostream)& out) const;
  
  static Type GetType(IloIntCollection) { return IntCollectionConst; }
  static Type GetType(IloNumCollection) { return NumCollectionConst; }
  static Type GetType(IloTupleCollection) { return TupleCollectionConst; }
  static Type GetType(IloSymbolSet) { return SymbolCollectionConst; }

  static Type GetType(IloIntExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloIntIndexI::GetTypeInfo())
      return IntIndex;
    return IntExpr;
  }
  static Type GetType(IloIntIndex) { return IntIndex; }
  static Type GetType(IloNumExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloNumIndexI::GetTypeInfo()) return NumIndex;
    return NumExpr;
  }
  static Type GetType(IloNumIndex) { return NumIndex; }
  static Type GetType(IloSymbolExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloSymbolIndexI::GetTypeInfo()) return SymbolIndex;
    return SymbolExpr;
  }
  static Type GetType(IloSymbolIndex) { return SymbolIndex; }
  static Type GetType(IloTupleExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloTuplePatternI::GetTypeInfo())
      return GetType(IloTuplePattern((IloTuplePatternI*)e.getImpl()));
    else if (e.getImpl() && e.getImpl()->getTypeInfo() == IloTupleIndexI::GetTypeInfo())
      return TupleIndex;
    return TupleExpr;
  }
  static Type GetType(IloTuplePattern p) {
    return (p.getImpl()->isConst() ? TuplePatternConst : TuplePattern);
  }
  static Type GetType(IloIntCollectionExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloIntCollectionIndexI::GetTypeInfo())
      return IntCollectionIndex;
    return IntCollectionExpr;
  }
  static Type GetType(IloIntCollectionIndex) { return IntCollectionIndex; }
  static Type GetType(IloNumCollectionExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloNumCollectionI::GetTypeInfo())
      return NumCollectionIndex;
    return NumCollectionExpr;
  }
  static Type GetType(IloNumCollectionIndex) { return NumCollectionIndex; }
  static Type GetType(IloSymbolCollectionExprArg e) {
    if (e.getImpl() && e.getImpl()->getTypeInfo() == IloSymbolCollectionIndexI::GetTypeInfo())
      return SymbolCollectionIndex;
    return SymbolCollectionExpr;
  }
  static Type GetType(IloSymbolCollectionIndex) { return SymbolCollectionIndex; }

  static Type GetType(IloExtractable x) {
    if (!x.getImpl()) return Extractable;
    IloTypeInfo type = x.getImpl()->getTypeInfo();
    if (type == IloIntIndexI::GetTypeInfo()) return IntIndex;
    else if (type == IloNumIndexI::GetTypeInfo()) return NumIndex;
    else if (type == IloSymbolIndexI::GetTypeInfo()) return SymbolIndex;
    else if (type == IloTupleIndexI::GetTypeInfo()) return TupleIndex;
    else if (type == IloIntCollectionIndexI::GetTypeInfo()) return IntCollectionIndex;
    else if (type == IloNumCollectionIndexI::GetTypeInfo()) return NumCollectionIndex;
    else if (type == IloSymbolCollectionIndexI::GetTypeInfo()) return SymbolCollectionIndex;
    else if (type == IloConstraintI::GetTypeInfo()) return ConstraintExpr;
    else if (type == IloTupleExprI::GetTypeInfo()) return GetType(IloTupleExprArg((IloTupleExprI*)x.getImpl()));
    else if (x.getImpl()->isType(IloIntExprI::GetTypeInfo())) return IntExpr;
    else if (x.getImpl()->isType(IloNumExprI::GetTypeInfo())) return NumExpr;
    else if (IloIntervalVar::MatchTypeInfo(x.getImpl()->getTypeInfo()))
      return IntervalExpr;
    else if (IloAdvPiecewiseFunction::MatchTypeInfo(x.getImpl()->getTypeInfo()))
      return PiecewiseFunctionExpr;
    else if (IloIntervalSequenceExprArg::MatchTypeInfo(x.getImpl()->getTypeInfo()))
      return SequenceExpr;
    else if (IloCumulFunctionExpr::MatchTypeInfo(x.getImpl()->getTypeInfo()))
      return CumulFunctionExpr;
    if (IloStateFunctionExpr::MatchTypeInfo(x.getImpl()->getTypeInfo())) {
      return StateFunctionExpr;
    }
    return Extractable;
  }
  static Type GetType(IloConstraint) { return ConstraintExpr; }
  static Type GetType(IloMapI*) { return SubMap; }
  using IloObjectBase::GetType;
  static Type GetType(IloTuple x);

  using IloObjectBase::getValue;
};

#define IloMapItem IloOplObject

typedef IloArray<IloOplObject> IloMapIndexArrayBase;

class ILO_EXPORTED IloMapIndexArray : public IloMapIndexArrayBase {
public:
  typedef IloDefaultArrayI ImplClass;
  
  IloMapIndexArray(IloDefaultArrayI* i=0) : IloMapIndexArrayBase(i) {}
  IloMapIndexArray(const IloMapIndexArray& copy) : IloMapIndexArrayBase(copy) {}
  
  IloMapIndexArray(const IloEnv env, IloInt n = 0) : IloMapIndexArrayBase(env, n) {}

#ifndef CPPREF_GENERATION
  const IloOplObject& operator[] (IloInt i) const {
    return IloMapIndexArrayBase::operator[](i);
  }
  IloOplObject& operator[] (IloInt i) {
    return IloMapIndexArrayBase::operator[](i);
  }
#endif
  IloMapIndexArray makeClone(IloEnvI* env) const{
    IloMapIndexArray res(env);
    for (IloInt i=0; i< getSize(); i++){
      IloOplObject item = this->operator[](i);
      if (item.isExtractable())
        res.add( IloExtractable(env->getClone(item.asExtractable().getImpl())) );
      else
        res.add( item );
    }
    return res;
  }
  void endElements();
  void lockElements();
  IloBool isConstant() const;
#ifdef CPPREF_GENERATION
  
  void add(const char* index);
  
  void add(IloInt index);
  
  void add(IloNum index);
  
  void add(IloTuple index);
  
  void clear();
  
  IloInt getSize() const;
#endif
};

class IloMapException  : public IloException {
  const IloAbstractMapI* _map;
  const IloExtractableI* _obj;
public:
  
  IloMapException (const IloAbstractMapI*);
  IloMapException (const IloAbstractMapI*, const IloExtractableI*);
  
  virtual void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
  const IloAbstractMapI* getMap() const { return (_map); }
  
  const IloExtractableI* getExtractable() const { return (_obj); }
  
  void setContext(IloExtractableI* context) { _obj = context; }
};

class IloMapOutOfBoundException  : public IloMapException {
  const IloOplObject _index;
public:
  
  IloMapOutOfBoundException (const IloAbstractMapI*, const IloOplObject, const IloExtractableI*);
  IloMapOutOfBoundException (const IloAbstractMapI*, const IloOplObject);
  
  void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  const IloOplObject getMapItem() const { return (_index); }

  virtual const char* getMessage() const ILO_OVERRIDE;
};

class IloMapUnboundIndexException  : public IloMapException {
public:
  
  IloMapUnboundIndexException (const IloAbstractMapI*, const IloExtractableI*);
  
  void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloLockedMapException  : public IloMapException {
public:
  IloLockedMapException (const IloAbstractMapI*);
  
  void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloWrongMapDimensionException  : public IloMapException {
  const IloInt _expectedNbDim;
public:
  IloWrongMapDimensionException (const IloAbstractMapI*, const IloInt,
    const IloExtractableI*);
  IloWrongMapDimensionException (const IloAbstractMapI*, const IloInt);
  
  IloInt getExpectedNbOfDimension() const { return (_expectedNbDim); }
  
  void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloMapsDimensionException  : public IloException {
  const IloInt _size1;
  const char* _name1;
  const IloInt _size2;
  const char* _name2;
public:
  IloMapsDimensionException (const IloInt, const IloInt, const char*, const char*);
  IloInt getSize1() const { return _size1; }
  IloInt getSize2() const { return _size2; }
  const char* getName1() const { return _name1; }
  const char* getName2() const { return _name2; }
  void print(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual const char* getMessage() const ILO_OVERRIDE;
};

class IloMapExtractIndexI : public IloExtractableI {
  ILOEXTRDECL
protected:
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  IloMapExtractIndexI(IloEnvI* env) : IloExtractableI(env) {}
  virtual ~IloMapExtractIndexI(){}
public:
  static IloMapExtractIndexI* make(IloEnvI*, IloOplObject midx);
  static IloMapExtractIndexI* make(IloIntExprArg e);
  static IloMapExtractIndexI* make(IloNumExprArg e);
  static IloMapExtractIndexI* make(IloSymbolExprArg e);
  static IloMapExtractIndexI* make(IloTupleExprArg e);
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE = 0;
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE = 0;
  virtual IloNum eval(const IloAlgorithm alg) const = 0;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloMapI* m) const = 0;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const = 0;
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const = 0;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m,
    IloInt dim=0) const = 0;
  virtual IloBool isIntIndex() const;
  virtual IloBool isExtractableIndex() const;
  virtual IloBool isIntExprIndex() const;
  virtual IloInt getIntIndex() const;
  virtual IloNum getNumIndex() const;
  virtual IloAny getAnyIndex() const;
  IloIntExprI* getIntExprIndex() const;
  IloNumExprI* getNumExprIndex() const;
  IloAnyExprI* getAnyExprIndex() const{
    IloExtractableI* ex = getExtractableIndex();
    return (IloAnyExprI*)ex;
  }
  virtual IloExtractableI* getExtractableIndex() const;
};

class IloMapIntIndexI : public IloMapExtractIndexI {
  ILOEXTRDECL
private:
  IloInt _value;
public:
  IloMapIntIndexI(IloEnvI* env, IloInt value) : IloMapExtractIndexI(env), _value(value) {}
  virtual ~IloMapIntIndexI();
  virtual IloBool isIntIndex() const ILO_OVERRIDE;
  virtual IloInt getIntIndex() const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloMapNumIndexI : public IloMapExtractIndexI {
  ILOEXTRDECL
private:
  IloNum _value;
public:
  IloMapNumIndexI(IloEnvI* env, IloNum value) : IloMapExtractIndexI(env), _value(value) {}
  virtual ~IloMapNumIndexI();
  virtual IloNum getNumIndex() const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloMapAnyIndexI : public IloMapExtractIndexI {
  ILOEXTRDECL
protected:
  IloAny _value;
public:
  IloMapAnyIndexI(IloEnvI* env, IloAny value)
    : IloMapExtractIndexI(env), _value(value){}
  virtual ~IloMapAnyIndexI();
  virtual IloAny getAnyIndex() const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloMapSymbolIndexI : public IloMapAnyIndexI {
  ILOEXTRDECL
public:
  IloMapSymbolIndexI(IloEnvI* env, IloSymbolI* value)
    : IloMapAnyIndexI(env, value) {}
  virtual ~IloMapSymbolIndexI();
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
};

class IloMapTupleIndexI : public IloMapAnyIndexI {
  ILOEXTRDECL
public:
  IloMapTupleIndexI(IloEnvI* env, IloTupleI* value)
    : IloMapAnyIndexI(env, value) {}
  virtual ~IloMapTupleIndexI();
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
};

class IloMapExtractableIndexI : public IloMapExtractIndexI {
  ILOEXTRDECL
protected:
  IloExtractableI* _expr;
  IloMapExtractableIndexI(IloEnvI* env, IloExtractableI* expr);
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  virtual ~IloMapExtractableIndexI();
  virtual IloBool isExtractableIndex() const ILO_OVERRIDE;
  virtual IloExtractableI* getExtractableIndex() const ILO_OVERRIDE;
  virtual void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt getAbsoluteIndex(const IloAbstractTupleMapI* m, IloInt dim=0) const ILO_OVERRIDE;
  virtual void atRemove(IloExtractableI* sub, IloAny) ILO_OVERRIDE {}
};

class IloMapNumExprIndexI : public IloMapExtractableIndexI {
  ILOEXTRDECL
public:
  IloMapNumExprIndexI(IloEnvI* env, IloNumExprI* expr);
  virtual ~IloMapNumExprIndexI();
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloMapIntExprIndexI : public IloMapNumExprIndexI {
  ILOEXTRDECL
public:
  IloMapIntExprIndexI(IloEnvI* env, IloIntExprI* expr)
    : IloMapNumExprIndexI(env, expr) {}
  virtual ~IloMapIntExprIndexI();
  virtual IloBool isIntExprIndex() const ILO_OVERRIDE;
  
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloMapAnyExprIndexI : public IloMapExtractableIndexI {
  ILOEXTRDECL
public:
  IloMapAnyExprIndexI(IloEnvI* env, IloAnyExprI* expr);
  virtual ~IloMapAnyExprIndexI();
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg, IloMapI* m) const ILO_OVERRIDE;
  virtual IloInt evalAbsoluteIndex(const IloAlgorithm alg,
    IloAbstractTupleMapI* m,
    IloInt dim=0) const ILO_OVERRIDE;
};

class IloAbstractMapI : public IloRttiEnvObjectI {
  ILORTTIDECL
private:
  IloAnyArray _accessors;
  IloMapIndexArray _shared;
protected:
  IloDiscreteDataCollectionI* _indexer;
public:
  IloAbstractMapI(IloEnvI*, IloDiscreteDataCollectionI*);
  IloMapIndexArray getOrMakeSharedIndexArray(){
    if (_shared.getImpl()) return _shared;
    _shared = IloMapIndexArray(getEnv(), getNbDim());
    return _shared;
  }
  IloDiscreteDataCollectionI* getIndexer() const { return _indexer; }
  virtual ~IloAbstractMapI();
  virtual IloDiscreteDataCollectionI* getIndexer(IloInt i) const = 0;
  virtual const char* getName() const = 0;
  virtual void setName(const char* name) = 0;
  virtual IloInt getNbDim() const = 0;
  virtual IloInt getTotalSize() = 0;
  virtual IloInt getSize() const = 0;
  virtual IloOplObject getAt(IloMapIndexArray indices) const = 0;
  virtual void setAt(IloMapIndexArray indices, IloOplObject value) = 0;
  virtual void setAtAbsoluteIndex(IloIntFixedArray indices, IloOplObject value) = 0;
  virtual IloOplObject getAtAbsoluteIndex(IloIntFixedArray indices) const = 0;
  virtual void display(ILOSTD(ostream)& out) const = 0;
  virtual void copyContent(const IloAbstractMapI*) = 0;
  virtual IloBool isOplRefCounted() const ILO_OVERRIDE { return IloTrue; }
  void registerAccess(IloRttiEnvObjectI* access);
  void unregisterAccess(IloRttiEnvObjectI* access);
  IloAnyArray getAccessors() const { return _accessors;}
  virtual void garbage() ILO_OVERRIDE;
  virtual IloInt getNonEmptySlotSize() { return getTotalSize(); }
};

class IloMapI : public IloAbstractMapI {
  ILORTTIDECL
private:
  
  
  IloInt _nbDim;
  IloArrayI* _values;
  IloInt _totalSize;
  char* _name;
  IloTupleSchemaI const *_auxData;
  IloOplObject::Type _itemType;
  IloMapIndexer _sharedIndexers;

  void* getData(IloInt i) const { return  _values->getData(i); }
  IloOplObject getAt(IloOplObject index) const {
    IloInt absIdx = getAbsoluteIndex(index);
    IloOplObject item = getAtAbsoluteIndex(absIdx);
    return item;
  }
  IloOplObject getSubMapExpr(IloMapIndexArray indices, IloInt currIdx) const;
  IloDiscreteDataCollectionI* computeIndexer(IloInt i) const;
public:
  
  
  IloMapIndexer getOrMakeSharedMapIndexer();
  IloMapIndexer makeMapIndexer() const;

  void setAtAbsoluteIndex(IloInt i, IloOplObject value);
  IloOplObject getAtAbsoluteIndex(IloInt i) const;
  void setType(IloOplObject::Type type);
public:
  IloMapI(IloEnvI*, IloInt, IloDiscreteDataCollectionI*, IloInt);
  IloMapI(IloEnvI*, IloInt, IloDiscreteDataCollectionI*, IloArrayI*);
  IloMapI(IloEnvI*, const IloMapI*);
  virtual void copyContent(const IloAbstractMapI*) ILO_OVERRIDE;
  virtual ~IloMapI();
  IloMapI* getCopy() const;
  IloMapI* makeClone(IloEnvI*) const;
  IloOplObject::Type getItemType() const { return _itemType; }
  IloOplObject::Type getLastItemType() const;
  IloTupleSchemaI* getAuxData() const {return (IloTupleSchemaI*)_auxData;}
  void setAuxData(IloTupleSchemaI const *auxData);
  void zeroData() {_values->zeroData(); }
  inline IloInt getNbDim() const ILO_OVERRIDE { return _nbDim; }
  IloInt getTotalSize() ILO_OVERRIDE;
  IloInt getSize() const ILO_OVERRIDE { return _indexer->getSize(); }
  IloInt getValueSize() const { return _values->getSize(); }
  IloInt getNbElt() const;
  const char* getName() const ILO_OVERRIDE { return _name; }
  void setName(const char* name) ILO_OVERRIDE;
  IloArrayI* getValues() const { return _values; }
  void setValues(IloArrayI* values) { delete _values; _values=values; }
  using IloAbstractMapI::getIndexer;
  IloDiscreteDataCollectionI* getIndexer(IloInt i) const ILO_OVERRIDE;

  

  IloInt fastGetAbsoluteIndex(IloOplObject idx) const;
  IloInt getAbsoluteIndex(IloOplObject idx) const;
  IloMapVectorIndexI* getVectorIndex() const;
  using IloAbstractMapI::setAt;
  using IloAbstractMapI::getAt;
  virtual IloOplObject getAt(IloMapIndexArray indices) const ILO_OVERRIDE;
  virtual void setAt(IloMapIndexArray indices, IloOplObject value) ILO_OVERRIDE;
  using IloAbstractMapI::setAtAbsoluteIndex;
  using IloAbstractMapI::getAtAbsoluteIndex;
  virtual void setAtAbsoluteIndex(IloIntFixedArray indices, IloOplObject value) ILO_OVERRIDE;
  virtual IloOplObject getAtAbsoluteIndex(IloIntFixedArray indices) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloRttiEnvObjectI* makeOplClone(IloEnvI* env) const ILO_OVERRIDE {
    return (IloRttiEnvObjectI*)makeClone(env);
  }
  virtual IloInt getNonEmptySlotSize() ILO_OVERRIDE;
};

class IloMapIterator;

class IloMapVectorIndexI {
  friend class IloMapIterator;
  IloMapIndexer _indexers;
  IloIntArray _absVector;
  IloMapVectorIndexI(const IloMapI* m, IloIntArray index);
public:
  IloIntArray getAbsVector() const{ return _absVector;}
  IloMapIndexer getMapIndexer() const { return _indexers; }
  IloMapVectorIndexI(const IloMapI* m);
  ~IloMapVectorIndexI();
  IloInt getIntAt(IloInt dim) const;
  IloNum getNumAt(IloInt dim) const;
  IloAny getAnyAt(IloInt dim) const;
  IloSymbol getSymbolAt(IloInt dim) const {
    return IloSymbol((IloSymbolI*)getAnyAt(dim));
  }
  const char* getStringAt(IloInt dim) const {
    return ((IloSymbolI*)getAnyAt(dim))->getString();
  }
  void display(ILOSTD(ostream)& out) const;

  class Iterator {
    IloMapVectorIndexI* _coll;
    IloInt _curr;
  public:
    Iterator(IloMapVectorIndexI* l): _coll(l), _curr(0) { }
    void operator++();
    IloOplObject operator*() const;
    IloBool  ok() const;
    void reset();
    IloInt currentIdx() const { return _curr; }
  };
  Iterator* iterator(){ return new (_absVector.getEnv()) Iterator(this); }
};

class IloMapIterator {
protected:
  IloGenAlloc* _heap;
  const IloMapI* _map;
  IloIntArray _size;
  IloIntArray _index;
  IloArray<IloArrayI*> _currSubMap;
  IloBool _ok;
  IloMapVectorIndexI * _vi;
  virtual IloBool next(IloInt);
  void resetIndex(IloInt i);
  void checkVectorIndex();
public:
  
  virtual IloBool next();

  
  virtual void reset();

  
  IloMapIterator(IloGenAlloc* heap, const IloMapI* mlCompare);

  
  IloMapIterator(const IloMapI* m);

  virtual ~IloMapIterator();

  

  
  IloGenAlloc* getHeap() const { return _heap; }

  
  const IloMapI* getMapI() const { return _map; }

  
  IloBool ok() const {
    return _ok;
  }

  

  
  void operator++() {
    _ok = next();
  }

  void operator delete(void *p, size_t sz);
#ifdef ILODELETEOPERATOR
  void operator delete(void *p, const IloEnvI *);
  void operator delete(void *p, const IloEnv &);
#endif
  void displayCurrentAbsoluteVectorIndex(ILOSTD(ostream)& out) const;

  IloArrayI* getCurrentEltArray() const {
    return _currSubMap[_map->getNbDim()-1];
  }

  IloInt getCurrentDeepestIndex() const {
    return _index[_map->getNbDim()-1];
  }
  IloMapVectorIndexI* getVectorIndex() const;
  void fillVectorIndex(IloMapVectorIndexI* vi) const;
  IloInt vectorIndexGetIntAt(IloInt dim);
  IloNum vectorIndexGetNumAt(IloInt dim);
  IloAny vectorIndexGetAnyAt(IloInt dim);
  IloSymbol vectorIndexGetSymbolAt(IloInt dim);
  const char* vectorIndexGetStringAt(IloInt dim);
  void displayVectorIndex(ILOSTD(ostream)& out);
};

#define ILOSUBMAPHANDLE(_typeElt, _super) \
class name2(_typeElt, SubMapExprI); \
class name2(_typeElt, SubMapExpr) : public _super { \
  ILOEXTRHANDLE(name2(_typeElt, SubMapExpr), _super) \
public: \
  name2(_typeElt, SubMapExpr) subscriptOp(IloIntExprArg idx) const; \
  name2(_typeElt, SubMapExpr) subscriptOp(IloNumExprArg idx) const; \
  name2(_typeElt, SubMapExpr) subscriptOp(IloSymbolExprArg idx) const; \
  name2(_typeElt, SubMapExpr) subscriptOp(IloTupleExprArg idx) const; \
};

ILOSUBMAPHANDLE(IloInt, IloIntExprArg)
ILOSUBMAPHANDLE(IloIntExpr, IloIntExprArg)
ILOSUBMAPHANDLE(IloNum, IloNumExprArg)
ILOSUBMAPHANDLE(IloNumExpr, IloNumExprArg)
ILOSUBMAPHANDLE(IloSymbol, IloSymbolExprArg)

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
