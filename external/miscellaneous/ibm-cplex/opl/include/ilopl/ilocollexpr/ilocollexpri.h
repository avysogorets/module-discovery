// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilocollexpr/ilocollexpri.h
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

#ifndef __ADVANCED_ilocollexpriH
#define __ADVANCED_ilocollexpriH

#ifdef _WIN32
#pragma pack(push, 8)
#endif
#include <ilopl/ilosys.h>

#include <ilopl/iloforallbase.h>
#include <ilopl/ilotuple.h>
#include <ilopl/iloforalli.h>
#include <ilopl/ilocollexpr/ilointcollexpri.h>
#include <ilopl/ilocollexpr/ilonumcollexpri.h>
#include <ilopl/ilocollexpr/iloanycollexpri.h>

class IloIntCollectionCardI : public IloIntExprI {
  ILOEXTRDECL
    IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntCollectionCardI(IloEnvI* env, IloIntCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloIntCollectionExprI*)expr->intLockExpr()) {}
    virtual ~IloIntCollectionCardI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloIntCollectionExprI* getExpr() const { return _expr; }
};

class IloIntCollectionMinI : public IloIntExprI {
  ILOEXTRDECL
    IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntCollectionMinI(IloEnvI* env, IloIntCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloIntCollectionExprI*)expr->intLockExpr()) {}
    virtual ~IloIntCollectionMinI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloIntCollectionExprI* getExpr() const { return _expr; }
};

class IloIntCollectionMaxI : public IloIntExprI {
  ILOEXTRDECL
    IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntCollectionMaxI(IloEnvI* env, IloIntCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloIntCollectionExprI*)expr->intLockExpr()) {}
    virtual ~IloIntCollectionMaxI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloIntCollectionExprI* getExpr() const { return _expr; }
};

class IloNumCollectionCardI : public IloIntExprI {
  ILOEXTRDECL
    IloNumCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumCollectionCardI(IloEnvI* env, IloNumCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloNumCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloNumCollectionCardI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloNumCollectionExprI* getExpr() const { return _expr; }
};

class IloNumCollectionMinI : public IloNumExprI {
  ILOEXTRDECL
    IloNumCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumCollectionMinI(IloEnvI* env, IloNumCollectionExprI* expr) :
    IloNumExprI(env), _expr((IloNumCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloNumCollectionMinI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloNumCollectionExprI* getExpr() const { return _expr; }
};

class IloNumCollectionMaxI : public IloNumExprI {
  ILOEXTRDECL
    IloNumCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumCollectionMaxI(IloEnvI* env, IloNumCollectionExprI* expr) :
    IloNumExprI(env), _expr((IloNumCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloNumCollectionMaxI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloNumCollectionExprI* getExpr() const { return _expr; }
};

class IloSymbolCollectionCardI : public IloIntExprI {
  ILOEXTRDECL
    IloSymbolCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloSymbolCollectionCardI(IloEnvI* env, IloSymbolCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloSymbolCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloSymbolCollectionCardI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloSymbolCollectionExprI* getExpr() const { return _expr; }
};

class IloIntCollectionExprMemberI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloIntExprI* _exp;
  IloIntCollectionExprI* _coll;
public:
  
  IloIntCollectionExprMemberI(IloEnvI* env, IloIntExprI* exp, IloIntCollectionExprI* coll);
  
  virtual ~IloIntCollectionExprMemberI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloIntExprI* getExpr() const { return _exp; }
  
  IloIntCollectionExprI* getCollection() const { return _coll; }
  ILOEXTROTHERDECL
};

class IloNumCollectionExprMemberI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloNumExprI* _exp;
  IloNumCollectionExprI* _coll;
public:
  
  IloNumCollectionExprMemberI(IloEnvI* env, IloNumExprI* exp, IloNumCollectionExprI* coll);
  
  virtual ~IloNumCollectionExprMemberI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloNumExprI* getExpr() const { return _exp; }
  
  IloNumCollectionExprI* getCollection() const { return _coll; }
  ILOEXTROTHERDECL
};

class IloSymbolCollectionExprMemberI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloSymbolExprI* _exp;
  IloSymbolCollectionExprI* _coll;
public:
  
  IloSymbolCollectionExprMemberI(IloEnvI* env, IloSymbolExprI* exp, IloSymbolCollectionExprI* coll);
  virtual ~IloSymbolCollectionExprMemberI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloAnyExprI* getExpr() const { return _exp; }
  
  IloSymbolCollectionExprI* getCollection() const { return _coll; }
  ILOEXTROTHERDECL
};

class IloIntCollectionOrdI : public IloIntExprI {
  ILOEXTRDECL
  IloIntCollectionExprI* _coll;
  IloIntExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionOrdI(IloEnvI* env, IloIntCollectionExprI* coll, IloIntExprI* expr) :
    IloIntExprI(env), _coll((IloIntCollectionExprI*)coll->intLockExpr()), _expr(expr->intLockExpr()) {}
  virtual ~IloIntCollectionOrdI();
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  IloIntCollectionExprI* getCollection() const { return _coll; }
  IloIntExprI* getExpr() const { return _expr; }
};

class IloNumCollectionOrdI : public IloIntExprI {
  ILOEXTRDECL
  IloNumCollectionExprI* _coll;
  IloNumExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloNumCollectionOrdI(IloEnvI* env, IloNumCollectionExprI* coll, IloNumExprI* expr) :
    IloIntExprI(env), _coll((IloNumCollectionExprI*)coll->lockExpr()), _expr(expr->lockExpr()) {}
  virtual ~IloNumCollectionOrdI();
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  IloNumCollectionExprI* getCollection() const { return _coll; }
  IloNumExprI* getExpr() const { return _expr; }
};

class IloSymbolCollectionOrdI : public IloIntExprI {
  ILOEXTRDECL
  IloSymbolCollectionExprI* _coll;
  IloSymbolExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionOrdI(IloEnvI* env, IloSymbolCollectionExprI* coll, IloAnyExprI* expr) :
    IloIntExprI(env), _coll((IloSymbolCollectionExprI*)coll->lockExpr()), _expr((IloSymbolExprI*)expr->lockExpr()) {}
  virtual ~IloSymbolCollectionOrdI();
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  IloSymbolCollectionExprI* getCollection() const { return _coll; }
  IloSymbolExprI* getExpr() const { return _expr; }
};

class IloIntCollectionFirstI : public IloIntExprI {
  ILOEXTRDECL
  IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntCollectionFirstI(IloEnvI* env, IloIntCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloIntCollectionExprI*)expr->intLockExpr()) {}
    virtual ~IloIntCollectionFirstI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloIntCollectionExprI* getExpr() const { return _expr; }
};

class IloNumCollectionFirstI : public IloNumExprI {
  ILOEXTRDECL
  IloNumCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumCollectionFirstI(IloEnvI* env, IloNumCollectionExprI* expr) :
    IloNumExprI(env), _expr((IloNumCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloNumCollectionFirstI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloNumCollectionExprI* getExpr() const { return _expr; }
};

class IloIntCollectionLastI : public IloIntExprI {
  ILOEXTRDECL
  IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloIntCollectionLastI(IloEnvI* env, IloIntCollectionExprI* expr) :
    IloIntExprI(env), _expr((IloIntCollectionExprI*)expr->intLockExpr()) {}
    virtual ~IloIntCollectionLastI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloIntCollectionExprI* getExpr() const { return _expr; }
};

class IloNumCollectionLastI : public IloNumExprI {
  ILOEXTRDECL
  IloNumCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  
  IloNumCollectionLastI(IloEnvI* env, IloNumCollectionExprI* expr) :
    IloNumExprI(env), _expr((IloNumCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloNumCollectionLastI();
  
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
  
    virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  
    IloNumCollectionExprI* getExpr() const { return _expr; }
};

class IloSymbolCollectionFirstI : public IloSymbolExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionFirstI(IloEnvI* env, IloSymbolCollectionExprI* expr):
    IloSymbolExprI(env), _expr((IloSymbolCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloSymbolCollectionFirstI();
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
    IloSymbolCollectionExprI* getExpr() const { return _expr; }
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
};

class IloSymbolCollectionLastI : public IloSymbolExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionLastI(IloEnvI* env, IloSymbolCollectionExprI* expr):
    IloSymbolExprI(env), _expr((IloSymbolCollectionExprI*)expr->lockExpr()) {}
    virtual ~IloSymbolCollectionLastI();
    void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
    IloSymbolCollectionExprI* getExpr() const { return _expr; }
    virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
};

class IloIntCollectionUnionI : public IloIntCollectionExprI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _left;
  IloIntCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionUnionI(IloEnvI* env, IloIntCollectionExprI* left, IloIntCollectionExprI* right);
  virtual ~IloIntCollectionUnionI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloIntCollectionExprI* getLeft() const { return _left;}
  IloIntCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloNumCollectionUnionI : public IloNumCollectionExprI {
  ILOEXTRDECL
private:
  IloNumCollectionExprI* _left;
  IloNumCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloNumCollectionUnionI(IloEnvI* env, IloNumCollectionExprI* left, IloNumCollectionExprI* right);
  virtual ~IloNumCollectionUnionI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloNumCollectionExprI* getLeft() const { return _left;}
  IloNumCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloSymbolCollectionUnionI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _left;
  IloSymbolCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionUnionI(IloEnvI* env, IloSymbolCollectionExprI* left, IloSymbolCollectionExprI* right);
  virtual ~IloSymbolCollectionUnionI();
  IloSymbolCollectionExprI* getLeft() const { return _left;}
  IloSymbolCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloIntCollectionSymExcludeI : public IloIntCollectionExprI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _left;
  IloIntCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionSymExcludeI(IloEnvI* env, IloIntCollectionExprI* left, IloIntCollectionExprI* right);
  virtual ~IloIntCollectionSymExcludeI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloIntCollectionExprI* getLeft() const { return _left;}
  IloIntCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloNumCollectionSymExcludeI : public IloNumCollectionExprI {
  ILOEXTRDECL
private:
  IloNumCollectionExprI* _left;
  IloNumCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloNumCollectionSymExcludeI(IloEnvI* env, IloNumCollectionExprI* left, IloNumCollectionExprI* right);
  virtual ~IloNumCollectionSymExcludeI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloNumCollectionExprI* getLeft() const { return _left;}
  IloNumCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloSymbolCollectionSymExcludeI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _left;
  IloSymbolCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionSymExcludeI(IloEnvI* env, IloSymbolCollectionExprI* left, IloSymbolCollectionExprI* right);
  virtual ~IloSymbolCollectionSymExcludeI();
  IloSymbolCollectionExprI* getLeft() const { return _left;}
  IloSymbolCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloIntCollectionExcludeI : public IloIntCollectionExprI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _left;
  IloIntCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionExcludeI(IloEnvI* env, IloIntCollectionExprI* left, IloIntCollectionExprI* right);
  virtual ~IloIntCollectionExcludeI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloIntCollectionExprI* getLeft() const { return _left;}
  IloIntCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloNumCollectionExcludeI : public IloNumCollectionExprI {
  ILOEXTRDECL
private:
  IloNumCollectionExprI* _left;
  IloNumCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloNumCollectionExcludeI(IloEnvI* env, IloNumCollectionExprI* left, IloNumCollectionExprI* right);
  virtual ~IloNumCollectionExcludeI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloNumCollectionExprI* getLeft() const { return _left;}
  IloNumCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloSymbolCollectionExcludeI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _left;
  IloSymbolCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionExcludeI(IloEnvI* env, IloSymbolCollectionExprI* left, IloSymbolCollectionExprI* right);
  virtual ~IloSymbolCollectionExcludeI();
  IloSymbolCollectionExprI* getLeft() const { return _left;}
  IloSymbolCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloIntCollectionInterI : public IloIntCollectionExprI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _left;
  IloIntCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionInterI(IloEnvI* env, IloIntCollectionExprI* left, IloIntCollectionExprI* right);
  virtual ~IloIntCollectionInterI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloIntCollectionExprI* getLeft() const { return _left;}
  IloIntCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloNumCollectionInterI : public IloNumCollectionExprI {
  ILOEXTRDECL
private:
  IloNumCollectionExprI* _left;
  IloNumCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloNumCollectionInterI(IloEnvI* env, IloNumCollectionExprI* left, IloNumCollectionExprI* right);
  virtual ~IloNumCollectionInterI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloNumCollectionExprI* getLeft() const { return _left;}
  IloNumCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloSymbolCollectionInterI : public IloSymbolCollectionExprI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _left;
  IloSymbolCollectionExprI* _right;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloSymbolCollectionInterI(IloEnvI* env, IloSymbolCollectionExprI* left, IloSymbolCollectionExprI* right);
  virtual ~IloSymbolCollectionInterI();
  IloSymbolCollectionExprI* getLeft() const { return _left;}
  IloSymbolCollectionExprI* getRight() const { return _right;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloIntCollectionAsNumCollectionI : public IloNumCollectionExprI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _expr;
  virtual void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
public:
  IloIntCollectionAsNumCollectionI(IloEnvI* env, IloIntCollectionExprI* left);
  virtual ~IloIntCollectionAsNumCollectionI();
  virtual IloNum eval(const IloAlgorithm alg) const ILO_OVERRIDE;
  IloIntCollectionExprI* getExpr() const { return _expr;}
  virtual IloExtractableI* makeClone(IloEnvI*) const ILO_OVERRIDE;
  void display(ILOSTD(ostream)& out) const ILO_OVERRIDE;
};

class IloIntCollectionExprSubsetI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloIntCollectionExprI* _slice;
  IloIntCollectionExprI* _coll;
  IloBool _eq;
public:
  
  IloIntCollectionExprSubsetI(IloEnvI* env, IloIntCollectionExprI* exp, IloIntCollectionExprI* coll, IloBool eq);
  
  virtual ~IloIntCollectionExprSubsetI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloIntCollectionExprI* getSlice() const { return _slice; }
  
  IloIntCollectionExprI* getCollection() const { return _coll; }
  IloBool isSubSetEq() const { return _eq; }
  ILOEXTROTHERDECL
};

class IloNumCollectionExprSubsetI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloNumCollectionExprI* _slice;
  IloNumCollectionExprI* _coll;
  IloBool _eq;
public:
  
  IloNumCollectionExprSubsetI(IloEnvI* env, IloNumCollectionExprI* exp, IloNumCollectionExprI* coll, IloBool eq);
  
  virtual ~IloNumCollectionExprSubsetI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloNumCollectionExprI* getSlice() const { return _slice; }
  
  IloNumCollectionExprI* getCollection() const { return _coll; }
  IloBool isSubSetEq() const { return _eq; }
  ILOEXTROTHERDECL
};

class IloSymbolCollectionExprSubsetI : public IloConstraintI {
  ILOEXTRDECL
private:
  IloSymbolCollectionExprI* _slice;
  IloSymbolCollectionExprI* _coll;
  IloBool _eq;
public:
  
  IloSymbolCollectionExprSubsetI(IloEnvI* env, IloSymbolCollectionExprI* exp, IloSymbolCollectionExprI* coll, IloBool eq);
  virtual ~IloSymbolCollectionExprSubsetI();
  
  void visitSubExtractables(IloExtractableVisitor* v) ILO_OVERRIDE;
  
  IloSymbolCollectionExprI* getSlice() const { return _slice; }
  
  IloSymbolCollectionExprI* getCollection() const { return _coll; }
  IloBool isSubSetEq() const { return _eq; }
  ILOEXTROTHERDECL
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
