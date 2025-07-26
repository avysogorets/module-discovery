// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilocollexpr/iloanycollexpri.h
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

#ifndef __ADVANCED_iloanycollexpriH
#define __ADVANCED_iloanycollexpriH

#ifdef _WIN32
#pragma pack(push, 8)
#endif
#include <ilopl/ilosys.h>

#include <ilopl/iloforallbase.h>

class IloTupleIndexI;

class IloSymbolCollectionExprGeneratorI;

class IloSymbolCollectionIndexI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
public:
  IloSymbolCollectionIndexI(IloEnvI* env, const char* name=0);
  virtual ~IloSymbolCollectionIndexI();
  IloNum eval(const IloAlgorithm alg) const;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  ILOEXTROTHERDECL
  virtual IloBool hasNongroundType() const ILO_OVERRIDE;

  IloInt getKey() const{
    return (IloInt)this;
  }
  void errorNotSubstituted() const{
    throw IloIndex::NotSubstituted(this);
  }
};

class IloSymbolCollectionTupleCellExprI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloTupleExprI* _tuple;
  IloSymbolI* _colName;
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolCollectionTupleCellExprI(IloEnvI* env, IloTupleExprI* tuple, IloSymbolI* colName);
  
  virtual ~IloSymbolCollectionTupleCellExprI();
  
  IloTupleExprI* getTuple() const { return _tuple; }
  
  IloSymbolI* getColumnName() const { return _colName; }
  ILOEXTROTHERDECL
};

class IloSymbolCollectionExprGeneratorI : public IloSymbolGeneratorI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _coll;
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolCollectionExprGeneratorI(IloEnvI* env, IloSymbolIndexI* x, IloSymbolCollectionExprI* expr);
  virtual ~IloSymbolCollectionExprGeneratorI();
  
  IloSymbolCollectionExprI* getCollection() const { return _coll; }
  
  virtual IloBool generatesDuplicates() const ILO_OVERRIDE;
  ILOEXTROTHERDECL
  virtual IloDiscreteDataCollectionI* getDiscreteDataCollection() const ILO_OVERRIDE;
  virtual IloBool hasDiscreteDataCollection() const ILO_OVERRIDE;
};

class IloEvalSymbolCollectionExprI;
class IloEvalSymbolCollectionExprIIterator : public IloAnyDefaultDataIterator {
  IloEvalSymbolCollectionExprI*  _expr;
public:
  
  IloEvalSymbolCollectionExprIIterator(IloGenAlloc* heap,
    IloEvalSymbolCollectionExprI* expr);
  
  virtual ~IloEvalSymbolCollectionExprIIterator();
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloAny value, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloSymbolCollectionTupleCellExprIIterator : public IloAnyDefaultDataIterator {
  IloSymbolCollectionTupleCellExprI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloSymbolCollectionTupleCellExprIIterator(IloGenAlloc* heap,
    const IloSymbolCollectionTupleCellExprI* rangeexpr,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloSymbolCollectionTupleCellExprIIterator();
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloAny value, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloSymbolCollectionIndexIIterator : public IloAnyDefaultDataIterator {
  IloSymbolCollectionIndexI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloSymbolCollectionIndexIIterator(IloGenAlloc* heap,
    const IloSymbolCollectionIndexI* rangeexpr,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloSymbolCollectionIndexIIterator();
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloAny value, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloSymbolCollectionSubMapExprI;

class IloSymbolCollectionSubMapExprIIterator : public IloAnyDefaultDataIterator {
  const IloSymbolCollectionSubMapExprI*  _expr;
  IloAdvModelEvaluatorI* _evaluator;
public:
  
  IloSymbolCollectionSubMapExprIIterator(IloGenAlloc* heap,
    const IloSymbolCollectionSubMapExprI* map,
    const IloAdvModelEvaluatorI* eval);
  
  virtual ~IloSymbolCollectionSubMapExprIIterator();
  virtual void reset(IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
  virtual void reset(IloAny value, IloBool catchInvalidCollection = IloFalse) ILO_OVERRIDE;
};

class IloSymbolAggregateSetExprI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
    IloExtendedComprehensionI* _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolAggregateSetExprI(IloExtendedComprehensionI* comp);

  virtual ~IloSymbolAggregateSetExprI();

  IloExtendedComprehensionI* getComprehension() const { return _comp; }

  IloSymbolExprI* getBody() const { return (IloSymbolExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloTupleAggregateSetExprI : public IloTupleSetExprArgI {
  ILOEXTRDECL
    IloExtendedComprehensionI* _comp;

  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloTupleAggregateSetExprI(IloTupleSchemaI const *schema, IloExtendedComprehensionI* comp);

  virtual ~IloTupleAggregateSetExprI();

  IloExtendedComprehensionI* getComprehension() const { return _comp; }

  IloTupleExprI* getBody() const { return (IloTupleExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloSymbolAggregateUnionSetExprI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
  IloExtendedComprehensionI* _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolAggregateUnionSetExprI(IloExtendedComprehensionI* comp);

  virtual ~IloSymbolAggregateUnionSetExprI();

    IloExtendedComprehensionI* getComprehension() const { return _comp; }

    IloSymbolCollectionExprI* getBody() const { return (IloSymbolCollectionExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloTupleAggregateUnionSetExprI : public IloTupleSetExprArgI {
  ILOEXTRDECL
  IloExtendedComprehensionI* _comp;

  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloTupleAggregateUnionSetExprI(IloExtendedComprehensionI* comp);

  virtual ~IloTupleAggregateUnionSetExprI();

    IloExtendedComprehensionI* getComprehension() const { return _comp; }

    IloTupleSetExprI* getBody() const { return (IloTupleSetExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloSymbolAggregateInterSetExprI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
  IloExtendedComprehensionI* _comp;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolAggregateInterSetExprI(IloExtendedComprehensionI* comp);

  virtual ~IloSymbolAggregateInterSetExprI();

    IloExtendedComprehensionI* getComprehension() const { return _comp; }

    IloSymbolCollectionExprI* getBody() const { return (IloSymbolCollectionExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloTupleAggregateInterSetExprI : public IloTupleSetExprArgI {
  ILOEXTRDECL
  IloExtendedComprehensionI* _comp;

  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloTupleAggregateInterSetExprI(IloExtendedComprehensionI* comp);

  virtual ~IloTupleAggregateInterSetExprI();

    IloExtendedComprehensionI* getComprehension() const { return _comp; }

    IloTupleSetExprI* getBody() const { return (IloTupleSetExprI*)_comp->getExtent(); }
  ILOEXTROTHERDECL
};

class IloSymbolCollectionConstI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloAnyCollectionI* _coll;
  IloBool _ownsColl;
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionConstI(IloEnvI* env, IloAnyCollectionI* coll, IloBool ownsColl=IloFalse);
  virtual ~IloSymbolCollectionConstI();
  IloAnyCollectionI* getCollection() const { return _coll; }
  ILOEXTROTHERDECL
};

class IloConditionalSymbolSetExprI : public IloConditionalExprI< IloSymbolCollectionExprI, IloConditionalSymbolSetExprI > {
  ILOEXTRDECL

public:
  IloConditionalSymbolSetExprI(IloEnvI* env, IloConstraintI* cond, IloSymbolCollectionExprI* left, IloSymbolCollectionExprI* right )
    : IloConditionalExprI< IloSymbolCollectionExprI, IloConditionalSymbolSetExprI >( env, cond, left, right ) {}
  virtual ~IloConditionalSymbolSetExprI(){}
};

class IloConditionalTupleSetExprI : public IloConditionalExprI< IloTupleSetExprI, IloConditionalTupleSetExprI > {

  ILOEXTRDECL
public:
  IloConditionalTupleSetExprI(IloEnvI* env, IloConstraintI* cond, IloTupleSetExprI* left, IloTupleSetExprI* right )
    : IloConditionalExprI< IloTupleSetExprI, IloConditionalTupleSetExprI >( env, cond, left, right ) {}
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
