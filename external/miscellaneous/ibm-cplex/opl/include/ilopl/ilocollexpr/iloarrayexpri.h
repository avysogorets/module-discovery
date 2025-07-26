// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilocollexpr/iloarrayexpri.h
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

#ifndef __ADVANCED_iloarrayexpriH
#define __ADVANCED_iloarrayexpriH

#ifdef _WIN32
#pragma pack(push, 8)
#endif
#include <ilopl/ilosys.h>

#include <ilopl/iloforallbase.h>
#include <ilopl/ilomapi.h>

class IloIntSetByExtensionExprI : public IloIntCollectionExprI {
  ILOEXTRDECL
  IloMapIndexArray _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntSetByExtensionExprI(IloEnvI* env, IloMapIndexArray e);
  virtual ~IloIntSetByExtensionExprI();

  IloMapIndexArray getArray() const { return _comp; }

  
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  virtual IloBool isInteger() const ILO_OVERRIDE;
};

class IloSymbolSetByExtensionExprI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
    IloMapIndexArray _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolSetByExtensionExprI(IloEnvI* env, IloMapIndexArray e);

  virtual ~IloSymbolSetByExtensionExprI();

  IloMapIndexArray getArray() const { return _comp; }
  ILOEXTROTHERDECL
};

class IloTupleSetByExtensionExprI : public IloTupleSetExprArgI {
  ILOEXTRDECL
  IloTupleSchemaI const *_schema;
  IloMapIndexArray _comp;

  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloTupleSetByExtensionExprI(IloEnvI* env, IloTupleSchemaI const *schema, IloMapIndexArray e);
  virtual ~IloTupleSetByExtensionExprI();
  
  IloTupleSchemaI const *getSchema() const { return _schema; }
  IloMapIndexArray getArray() const { return _comp; }
  ILOEXTROTHERDECL
};

class IloNumSetByExtensionExprI : public IloNumCollectionExprI {
  ILOEXTRDECL
    IloMapIndexArray _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumSetByExtensionExprI(IloEnvI* env, IloMapIndexArray e);
  virtual ~IloNumSetByExtensionExprI();

  IloMapIndexArray getArray() const { return _comp; }

  
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
};

class IloIntSetByExtensionExprIIterator : public IloIntDataIterator {
  IloIntSetByExtensionExprI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloIntSetByExtensionExprIIterator(IloGenAlloc* heap,
    const IloIntSetByExtensionExprI* expr,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloIntSetByExtensionExprIIterator();
  
  virtual IloBool next() ILO_OVERRIDE;
  
  virtual IloInt recomputeMin() const ILO_OVERRIDE;
  
  virtual IloInt recomputeMax() const ILO_OVERRIDE;
  void recomputeBounds(IloInt& min, IloInt& max) const;
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloInt intMin, IloInt intMax, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloNumSetByExtensionExprIIterator : public IloNumDataIterator {
  IloNumSetByExtensionExprI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloNumSetByExtensionExprIIterator(IloGenAlloc* heap,
    const IloNumSetByExtensionExprI* expr,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloNumSetByExtensionExprIIterator();
  
  virtual IloBool next() ILO_OVERRIDE;
  
  virtual IloNum recomputeLB() const ILO_OVERRIDE;
  
  virtual IloNum recomputeUB() const ILO_OVERRIDE;
  void recomputeBounds(IloNum& min, IloNum& max) const;
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloNum numLB, IloNum numUB, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloSymbolSetByExtensionExprIIterator : public IloAnyDefaultDataIterator {
  IloSymbolSetByExtensionExprI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloSymbolSetByExtensionExprIIterator(IloGenAlloc* heap,
    const IloSymbolSetByExtensionExprI* expr,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloSymbolSetByExtensionExprIIterator();
  
  virtual IloBool next() ILO_OVERRIDE;
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloAny value, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
