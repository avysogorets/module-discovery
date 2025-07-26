// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilcp/modeling.h
// --------------------------------------------------------------------------
//
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2024 All Rights Reserved.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
//
// --------------------------------------------------------------------------

#include <ilcp/cpext.h>
#include <vector>
#include <sstream>

namespace cpoptimizer {

class Model;
class CP;
template <class T> class Opt;

class InvalidOptAccess: public IloCP::Exception {
 public:
  InvalidOptAccess()
    :IloCP::Exception(-1, "Invalid access to Opt class (there is no value).")
  {}
};

class InvalidType: public IloCP::Exception {
 public:
  InvalidType()
    :IloCP::Exception(-1, "Expression has an invalid type.")
  {}
};

class InvalidPrintfFormat: public IloCP::Exception {
 public:
  InvalidPrintfFormat()
    :IloCP::Exception(-1, "Invalid printf format.")
  {}
};

class NoExpressionWithSuchName: public IloCP::Exception {
 public:
  NoExpressionWithSuchName()
    :IloCP::Exception(-1, "Expression with the given name does not exist.")
  {}
};

class NoExpressionWithSuchId: public IloCP::Exception {
 public:
  NoExpressionWithSuchId()
    :IloCP::Exception(-1, "Expression with the given id does not exist.")
  {}
};

class UndefinedMutableExpression: public IloCP::Exception {
 public:
  UndefinedMutableExpression()
    :IloCP::Exception(-1, "Invalid operation on undefined mutable expression.")
  {}
};

class Expr {
 protected:
  void *_impl;
 public:
  enum Type {
      BoolConstantT
    , BoolExprArrayT
    , BoolExprT
    , BoolVarT
    , ConstraintT
    , CumulAtomT
    , CumulExprArrayT
    , CumulExprT
    , FloatArrayT
    , FloatConstantT
    , FloatExprArrayT
    , FloatExprT
    , IntArrayT
    , IntConstantT
    , IntervalVarArrayT
    , IntervalVarT
    , IntExprArrayT
    , IntExprT
    , IntValueChooserT
    , IntValueEvalT
    , IntValueSelectorT
    , IntVarChooserT
    , IntVarEvalT
    , IntVarSelectorT
    , IntVarT
    , MutableBoolExprT
    , MutableIntExprT
    , MutableFloatExprT
    , MutableCumulExprT
    , ObjectiveT
    , SearchPhaseT
    , SegmentedFunctionT
    , SequenceVarArrayT
    , SequenceVarT
    , StateFunctionT
    , StepFunctionT
    , TransitionMatrixT
    , TupleSetT
  };
 protected:
  Expr() { _impl = nullptr; }
  explicit Expr(void* o) { _impl = o; }
  void _addName(const char* name);
  template <class T>
  static void _nameAux(std::stringstream& buffer, T v) {
    buffer << v;
  }
  template <typename T, typename... Ts>
  static void _nameAux(std::stringstream& buffer, T head, Ts... tail) {
    buffer << head;
    _nameAux(buffer, tail...);
  }
  template <typename... Ts>
  void _name(Ts... args) {
    std::stringstream buffer;
    buffer.imbue(std::locale::classic());
    _nameAux(buffer, args...);
    _addName(buffer.str().c_str());
  }
  
  void _setLocation(const char* filename, IlcInt line);
  Model _getModel() const;
  Type _getType() const;
  bool _isInTheSameModel(Expr other);
  void verify() const {
    IlcCPOAssert(_impl, "Empty handle (unitialized class).");
  }
  void verifyModel(Expr other) {
    IlcCPOAssert(_isInTheSameModel(other), "Expressions belong to different Models.");
    (void)other;
  }
  friend class Model;
  friend class CP;
  friend class IntVar;
  friend class IntArrayArg;
  friend class FloatArrayArg;
  friend class IntExprArrayArg;
  friend class BoolExprArrayArg;
  friend class FloatExprArrayArg;
  friend class CumulExprArrayArg;
  friend class IntervalVarArrayArg;
  friend class SequenceVarArrayArg;
  friend class Opt<Expr>;
 public:
  Expr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  Expr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  Expr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  Expr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  inline Model getModel() const;
  Type getType() const {
    verify();
    return _getType();
  }
  template <class T> inline Opt<T> downcast() const;
  void* getImpl() const { return _impl; }
};

template <class T>
class Opt {
 private:
  T _value;
  void _verifyAssigned() const {
    if (!_value._impl)
      throw InvalidOptAccess();
  }
 public:
  Opt() {
    static_assert(std::is_base_of<Expr,T>::value, "Template class Opt<T> can be used only for classes T derived from cpoptimizer::Expr.");
  }
  template <class U = T>
  Opt(const U value) {
    static_assert(std::is_base_of<Expr,T>::value, "Template class Opt<T> can be used only for classes T derived from cpoptimizer::Expr.");
    static_assert(std::is_convertible<U, T>::value, "Only types U that are convertible to T can be used in constructor of Opt<T>.");
    _value = value;
  }
  template <class U = T>
  Opt(const Opt<U> other) {
    static_assert(std::is_base_of<Expr,T>::value, "Template class Opt<T> can be used only for classes T derived from cpoptimizer::Expr.");
    static_assert(std::is_convertible<U, T>::value, "Only types U that are convertible to T can be used in constructor of Opt<T>.");
    _value = other._value;
  }
  template <class U = T>
  Opt& operator=(const U value) {
    static_assert(std::is_base_of<Expr,T>::value, "Template class Opt<T> can be used only for classes T derived from cpoptimizer::Expr.");
    static_assert(std::is_convertible<U, T>::value, "Only types U that are convertible to T can be used in assignment to Opt<T>.");
    _value = value;
    return *this;
  }
  template <class U = T>
  Opt& operator=(Opt<U> other) {
    static_assert(std::is_base_of<Expr,T>::value, "Template class Opt<T> can be used only for classes T derived from cpoptimizer::Expr.");
    static_assert(std::is_convertible<U, T>::value, "Only types U that are convertible to T can be used in assignment to Opt<T>.");
    _value = other._value;
    return *this;
  }
  void reset() {
    _value._impl = nullptr;
  }
  explicit operator bool() const noexcept { return _value._impl != nullptr; }
  bool has_value() const noexcept { return _value._impl != nullptr; }
  T& value() & { _verifyAssigned(); return _value; }
  const T& value() const & { _verifyAssigned(); return _value; }
  const T* operator->() const { _verifyAssigned(); return &_value; }
  T* operator->() { _verifyAssigned(); return &_value; }
  const T& operator*() const& { _verifyAssigned(); return _value; }
  T& operator*() & { _verifyAssigned(); return _value; }
};
class FloatExpr: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit FloatExpr(void* o): Expr(o) {}  FloatExpr(): Expr() {}
  friend class FloatExprArray;
  friend class MutableFloatExpr;
 public:
  typedef class MutableFloatExpr Mutable;
  static constexpr Expr::Type ExprType = FloatExprT;
  FloatExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  FloatExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  FloatExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  FloatExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntExpr: public FloatExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntExpr(void* o): FloatExpr(o) {}  IntExpr(): FloatExpr() {}
  friend class IntExprArray;
  friend class MutableIntExpr;
 public:
  typedef class MutableIntExpr Mutable;
  static constexpr Expr::Type ExprType = IntExprT;
  IntExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class BoolExpr: public IntExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit BoolExpr(void* o): IntExpr(o) {}  BoolExpr(): IntExpr() {}
  friend class BoolExprArray;
  friend class Constraint;
  friend class MutableBoolExpr;
 public:
  typedef class MutableBoolExpr Mutable;
  static constexpr Expr::Type ExprType = BoolExprT;
  BoolExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  BoolExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  BoolExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  BoolExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class Constraint: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit Constraint(void* o): Expr(o) {}  Constraint(): Expr() {}
 public:
  static constexpr Expr::Type ExprType = ConstraintT;
  Constraint& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  Constraint& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  Constraint& name(Ts... args) {
    _name(args...);
    return *this;
  }
  Constraint& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  Constraint(const BoolExpr& src): Expr(src._impl) {}
};

class Objective: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit Objective(void* o): Expr(o) {}  Objective(): Expr() {}
  friend class ObjectiveArray;
 public:
  static constexpr Expr::Type ExprType = ObjectiveT;
  Objective& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  Objective& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  Objective& name(Ts... args) {
    _name(args...);
    return *this;
  }
  Objective& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class SegmentedFunction: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit SegmentedFunction(void* o): Expr(o) {}  SegmentedFunction(): Expr() {}
  friend class SegmentedFunctionArray;
 public:
  static constexpr Expr::Type ExprType = SegmentedFunctionT;
  SegmentedFunction& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  SegmentedFunction& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  SegmentedFunction& name(Ts... args) {
    _name(args...);
    return *this;
  }
  SegmentedFunction& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class StepFunction: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit StepFunction(void* o): Expr(o) {}  StepFunction(): Expr() {}
  friend class StepFunctionArray;
 public:
  static constexpr Expr::Type ExprType = StepFunctionT;
  StepFunction& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  StepFunction& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  StepFunction& name(Ts... args) {
    _name(args...);
    return *this;
  }
  StepFunction& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class SequenceVar: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit SequenceVar(void* o): Expr(o) {}  SequenceVar(): Expr() {}
  friend class SequenceVarArray;
 public:
  static constexpr Expr::Type ExprType = SequenceVarT;
  SequenceVar& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  SequenceVar& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  SequenceVar& name(Ts... args) {
    _name(args...);
    return *this;
  }
  SequenceVar& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class StateFunction: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit StateFunction(void* o): Expr(o) {}  StateFunction(): Expr() {}
  friend class StateFunctionArray;
 public:
  static constexpr Expr::Type ExprType = StateFunctionT;
  StateFunction& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  StateFunction& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  StateFunction& name(Ts... args) {
    _name(args...);
    return *this;
  }
  StateFunction& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class TransitionMatrix: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit TransitionMatrix(void* o): Expr(o) {}  TransitionMatrix(): Expr() {}
  friend class TransitionMatrixArray;
 public:
  static constexpr Expr::Type ExprType = TransitionMatrixT;
  TransitionMatrix& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  TransitionMatrix& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  TransitionMatrix& name(Ts... args) {
    _name(args...);
    return *this;
  }
  TransitionMatrix& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntVarEval: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntVarEval(void* o): Expr(o) {}  IntVarEval(): Expr() {}
  friend class IntVarEvalArray;
 public:
  static constexpr Expr::Type ExprType = IntVarEvalT;
  IntVarEval& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntVarEval& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntVarEval& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntVarEval& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntValueEval: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntValueEval(void* o): Expr(o) {}  IntValueEval(): Expr() {}
  friend class IntValueEvalArray;
 public:
  static constexpr Expr::Type ExprType = IntValueEvalT;
  IntValueEval& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntValueEval& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntValueEval& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntValueEval& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntVarSelector: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntVarSelector(void* o): Expr(o) {}  IntVarSelector(): Expr() {}
  friend class IntVarSelectorArray;
 public:
  static constexpr Expr::Type ExprType = IntVarSelectorT;
  IntVarSelector& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntVarSelector& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntVarSelector& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntVarSelector& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntValueSelector: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntValueSelector(void* o): Expr(o) {}  IntValueSelector(): Expr() {}
  friend class IntValueSelectorArray;
 public:
  static constexpr Expr::Type ExprType = IntValueSelectorT;
  IntValueSelector& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntValueSelector& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntValueSelector& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntValueSelector& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntVarChooser: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntVarChooser(void* o): Expr(o) {}  IntVarChooser(): Expr() {}
  friend class IntVarChooserArray;
 public:
  static constexpr Expr::Type ExprType = IntVarChooserT;
  IntVarChooser& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntVarChooser& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntVarChooser& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntVarChooser& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class IntValueChooser: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntValueChooser(void* o): Expr(o) {}  IntValueChooser(): Expr() {}
  friend class IntValueChooserArray;
 public:
  static constexpr Expr::Type ExprType = IntValueChooserT;
  IntValueChooser& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntValueChooser& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntValueChooser& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntValueChooser& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class SearchPhase: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit SearchPhase(void* o): Expr(o) {}  SearchPhase(): Expr() {}
  friend class SearchPhaseArray;
 public:
  static constexpr Expr::Type ExprType = SearchPhaseT;
  SearchPhase& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  SearchPhase& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  SearchPhase& name(Ts... args) {
    _name(args...);
    return *this;
  }
  SearchPhase& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class CumulAtom: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit CumulAtom(void* o): Expr(o) {}  CumulAtom(): Expr() {}
  friend class CumulAtomArray;
 public:
  static constexpr Expr::Type ExprType = CumulAtomT;
  CumulAtom& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  CumulAtom& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  CumulAtom& name(Ts... args) {
    _name(args...);
    return *this;
  }
  CumulAtom& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
};

class CumulExpr: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit CumulExpr(void* o): Expr(o) {}  CumulExpr(): Expr() {}
  friend class CumulExprArray;
  friend class MutableCumulExpr;
 public:
  static constexpr Expr::Type ExprType = CumulExprT;
  CumulExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  CumulExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  CumulExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  CumulExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  CumulExpr(CumulAtom src): Expr(src.getImpl()) {}
};

class BoolConstant: public BoolExpr {
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit BoolConstant(void* o): BoolExpr(o) {}  BoolConstant(): BoolExpr() {}
  bool _getValue() const;
 public:
  static constexpr Expr::Type ExprType = BoolConstantT;
  BoolConstant& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  BoolConstant& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  BoolConstant& name(Ts... args) {
    _name(args...);
    return *this;
  }
  BoolConstant& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  bool getValue() const {
    verify();
    return _getValue();
  }
};

class IntConstant: public IntExpr {
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntConstant(void* o): IntExpr(o) {}  IntConstant(): IntExpr() {}
  IlcInt _getValue() const;
 public:
  static constexpr Expr::Type ExprType = IntConstantT;
  IntConstant& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntConstant& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntConstant& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntConstant& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getValue() const {
    verify();
    return _getValue();
  }
};

class FloatConstant: public FloatExpr {
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit FloatConstant(void* o): FloatExpr(o) {}  FloatConstant(): FloatExpr() {}
  IlcFloat _getValue() const;
 public:
  static constexpr Expr::Type ExprType = FloatConstantT;
  FloatConstant& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  FloatConstant& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  FloatConstant& name(Ts... args) {
    _name(args...);
    return *this;
  }
  FloatConstant& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcFloat getValue() const {
    verify();
    return _getValue();
  }
};

class IntVar: public IntExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntVar(void* o): IntExpr(o) {}  IntVar(): IntExpr() {}
  void _setDomain(IlcInt size, const IlcInt* values);
  void _setDomain(IlcInt min, IlcInt max);
  void _setDomain(const void*);
  IlcInt _getMin() const;
  IlcInt _getMax() const;
 public:
  static constexpr Expr::Type ExprType = IntVarT;
  IntVar& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntVar& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntVar& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntVar& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IntVar& setDomain(IlcInt min, IlcInt max) {
    verify();
    _setDomain(min, max);
    return *this;
  }
  IntVar& setDomain(class IntArrayArg domain);
  IlcInt getMin() const {
    verify();
    return _getMin();
  }
  IlcInt getMax() const {
    verify();
    return _getMax();
  }
};

class BoolVar: public BoolExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit BoolVar(void* o): BoolExpr(o) {}  BoolVar(): BoolExpr() {}
  void _setTrue();
  void _setFalse();
  bool _isTrue() const;
  bool _isFalse() const;
 public:
  static constexpr Expr::Type ExprType = BoolVarT;
  BoolVar& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  BoolVar& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  BoolVar& name(Ts... args) {
    _name(args...);
    return *this;
  }
  BoolVar& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  BoolVar& setTrue() {
    verify();
    _setTrue();
    return *this;
  }
  BoolVar& setFalse() {
    verify();
    _setFalse();
    return *this;
  }
  bool isTrue() const {
    verify();
    return _isTrue();
  }
  bool isFalse() const {
    verify();
    return _isFalse();
  }
};

class IntervalVar: public Expr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntervalVar(void* o): Expr(o) {}  IntervalVar(): Expr() {}
  friend class IntervalVarArray;
  void _optional();
  void _present();
  void _absent();
  void _start(IlcInt, IlcInt);
  void _end(IlcInt, IlcInt);
  void _size(IlcInt, IlcInt);
  void _length(IlcInt, IlcInt);
  void _length(IlcInt);
  bool _isPresent() const;
  bool _isAbsent() const;
  bool _isOptional() const;
  IlcInt _getStartMin() const;
  IlcInt _getStartMax() const;
  IlcInt _getEndMin() const;
  IlcInt _getEndMax() const;
  IlcInt _getSizeMin() const;
  IlcInt _getSizeMax() const;
  IlcInt _getLengthMin() const;
  IlcInt _getLengthMax() const;
 public:
  static constexpr Expr::Type ExprType = IntervalVarT;
  IntervalVar& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntervalVar& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntervalVar& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntervalVar& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }

  IntervalVar& optional() {
    verify();
    _optional();
    return *this;
  }
  IntervalVar& present() {
    verify();
    _present();
    return *this;
  }
  IntervalVar& absent() {
    verify();
    _absent();
    return *this;
  }
  IntervalVar& start(IlcInt startMin, IlcInt startMax) {
    verify();
    _start(startMin, startMax);
    return *this;
  }
  IntervalVar& start(IlcInt start) {
    verify();
    _start(start, start);
    return *this;
  }
  IntervalVar& end(IlcInt endMin, IlcInt endMax) {
    verify();
    _end(endMin, endMax);
    return *this;
  }
  IntervalVar& end(IlcInt end) {
    verify();
    _end(end, end);
    return *this;
  }
  IntervalVar& size(IlcInt sizeMin, IlcInt sizeMax) {
    verify();
    _size(sizeMin, sizeMax);
    return *this;
  }
  IntervalVar& size(IlcInt size) {
    verify();
    _size(size, size);
    return *this;
  }
  IntervalVar& length(IlcInt lengthMin, IlcInt lengthMax) {
    verify();
    _length(lengthMin, lengthMax);
    return *this;
  }
  IntervalVar& length(IlcInt length) {
    verify();
    _length(length, length);
    return *this;
  }

  bool isPresent() const {
    verify();
    return _isPresent();
  }
  bool isAbsent() const {
    verify();
    return _isAbsent();
  }
  bool isOptional() const {
    verify();
    return _isOptional();
  }
  IlcInt getStartMin() const {
    verify();
    return _getStartMin();
  }
  IlcInt getStartMax() const {
    verify();
    return _getStartMax();
  }
  IlcInt getEndMin() const {
    verify();
    return _getEndMin();
  }
  IlcInt getEndMax() const {
    verify();
    return _getEndMax();
  }
  IlcInt getSizeMin() const {
    verify();
    return _getSizeMin();
  }
  IlcInt getSizeMax() const {
    verify();
    return _getSizeMax();
  }
  IlcInt getLengthMin() const {
    verify();
    return _getLengthMin();
  }
  IlcInt getLengthMax() const {
    verify();
    return _getLengthMax();
  }
};

class MutableBoolExpr: public BoolExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit MutableBoolExpr(void* o): BoolExpr(o) {}  MutableBoolExpr(): BoolExpr() {}
  void* _get();
  void _set(void*);
  bool _isDefined() const;
 public:
  static constexpr Expr::Type ExprType = MutableBoolExprT;
  MutableBoolExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  MutableBoolExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  MutableBoolExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  MutableBoolExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  bool isDefined() const {
    verify();
    return _isDefined();
  }
  BoolExpr get() {
    verify();
    IlcCPOAssert(_isDefined(), "Invalid call MutableBoolExpr::get on undefined mutable expression.");
    return BoolExpr(_get());
  }
  void set(BoolExpr expr) {
    verify();
    expr.verify();
    verifyModel(expr);
    _set(expr._impl);
  }
};

class MutableIntExpr: public IntExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit MutableIntExpr(void* o): IntExpr(o) {}  MutableIntExpr(): IntExpr() {}
  void* _get();
  void _set(void*);
  bool _isDefined() const;
 public:
  static constexpr Expr::Type ExprType = MutableIntExprT;
  MutableIntExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  MutableIntExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  MutableIntExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  MutableIntExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  bool isDefined() const {
    verify();
    return _isDefined();
  }
  IntExpr get() {
    verify();
    IlcCPOAssert(_isDefined(), "Invalid call MutableIntExpr::get on undefined mutable expression.");
    return IntExpr(_get());
  }
  void set(IntExpr expr) {
    verify();
    expr.verify();
    verifyModel(expr);
    _set(expr._impl);
  }
};

class MutableFloatExpr: public FloatExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit MutableFloatExpr(void* o): FloatExpr(o) {}  MutableFloatExpr(): FloatExpr() {}
  void* _get();
  void _set(void*);
  bool _isDefined() const;
 public:
  static constexpr Expr::Type ExprType = MutableFloatExprT;
  MutableFloatExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  MutableFloatExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  MutableFloatExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  MutableFloatExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  bool isDefined() const {
    verify();
    return _isDefined();
  }
  FloatExpr get() {
    verify();
    IlcCPOAssert(_isDefined(), "Invalid call MutableFloatExpr::get on undefined mutable expression.");
    return FloatExpr(_get());
  }
  void set(FloatExpr expr) {
    verify();
    expr.verify();
    verifyModel(expr);
    _set(expr._impl);
  }
};

class MutableCumulExpr: public CumulExpr {
 protected:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit MutableCumulExpr(void* o): CumulExpr(o) {}  MutableCumulExpr(): CumulExpr() {}
  void* _get();
  void _set(void*);
  bool _isDefined() const;
 public:
  static constexpr Expr::Type ExprType = MutableCumulExprT;
  MutableCumulExpr& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  MutableCumulExpr& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  MutableCumulExpr& name(Ts... args) {
    _name(args...);
    return *this;
  }
  MutableCumulExpr& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  bool isDefined() const {
    verify();
    return _isDefined();
  }
  CumulExpr get() {
    verify();
    IlcCPOAssert(_isDefined(), "Invalid call MutableCumulExpr::get on undefined mutable expression.");
    return CumulExpr(_get());
  }
  void set(CumulExpr expr) {
    verify();
    expr.verify();
    verifyModel(expr);
    _set(expr._impl);
  }
};

class IntArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntArray(void* o): Expr(o) {}  IntArray(): Expr() {}
  friend class IntArrayArg;
  friend class TupleSet;
  IlcInt _getSize() const;
  IlcInt _at(IlcInt index) const;
  const IlcInt* _begin() const;
  const IlcInt* _end() const;
 public:
  typedef IlcInt ItemType;
  static constexpr Expr::Type ExprType = IntArrayT;
  IntArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  IlcInt operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return _at(index);
  }
  typedef const IlcInt* const_iterator;
  const_iterator begin() const { verify(); return _begin(); }
  const_iterator cbegin() const { verify(); return _begin(); }
  const_iterator end() const { verify(); return _end(); }
  const_iterator cend() const { verify(); return _end(); }
};

class FloatArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit FloatArray(void* o): Expr(o) {}  FloatArray(): Expr() {}
  friend class FloatArrayArg;
  IlcInt _getSize() const;
  IlcFloat _at(IlcInt index) const;
  const IlcFloat* _begin() const;
  const IlcFloat* _end() const;
 public:
  typedef IlcFloat ItemType;
  static constexpr Expr::Type ExprType = FloatArrayT;
  FloatArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  FloatArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  FloatArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  FloatArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  IlcFloat operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return _at(index);
  }
  typedef const IlcFloat* const_iterator;
  const_iterator begin() const { verify(); return _begin(); }
  const_iterator cbegin() const { verify(); return _begin(); }
  const_iterator end() const { verify(); return _end(); }
  const_iterator cend() const { verify(); return _end(); }
};

class BoolExprArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit BoolExprArray(void* o): Expr(o) {}  BoolExprArray(): Expr() {}
  friend class BoolExprArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef BoolExpr ItemType;
  static constexpr Expr::Type ExprType = BoolExprArrayT;
  BoolExprArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  BoolExprArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  BoolExprArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  BoolExprArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const BoolExpr operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return BoolExpr(_at(index));
  }
  BoolExpr operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return BoolExpr(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const BoolExpr operator*() { return static_cast<const BoolExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = BoolExpr;
    using pointer = const BoolExpr*;
    using reference = const BoolExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    BoolExpr operator*() { return static_cast<BoolExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = BoolExpr;
    using pointer = const BoolExpr*;
    using reference = const BoolExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class IntExprArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntExprArray(void* o): Expr(o) {}  IntExprArray(): Expr() {}
  friend class IntExprArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef IntExpr ItemType;
  static constexpr Expr::Type ExprType = IntExprArrayT;
  IntExprArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntExprArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntExprArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntExprArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IntExprArray(const BoolExprArray src): Expr(src.getImpl()) {}
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const IntExpr operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntExpr(_at(index));
  }
  IntExpr operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntExpr(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const IntExpr operator*() { return static_cast<const IntExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntExpr;
    using pointer = const IntExpr*;
    using reference = const IntExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    IntExpr operator*() { return static_cast<IntExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntExpr;
    using pointer = const IntExpr*;
    using reference = const IntExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class FloatExprArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit FloatExprArray(void* o): Expr(o) {}  FloatExprArray(): Expr() {}
  friend class FloatExprArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef FloatExpr ItemType;
  static constexpr Expr::Type ExprType = FloatExprArrayT;
  FloatExprArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  FloatExprArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  FloatExprArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  FloatExprArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  FloatExprArray(const BoolExprArray src): Expr(src.getImpl()) {}
  FloatExprArray(const IntExprArray src): Expr(src.getImpl()) {}
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const FloatExpr operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return FloatExpr(_at(index));
  }
  FloatExpr operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return FloatExpr(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const FloatExpr operator*() { return static_cast<const FloatExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = FloatExpr;
    using pointer = const FloatExpr*;
    using reference = const FloatExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    FloatExpr operator*() { return static_cast<FloatExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = FloatExpr;
    using pointer = const FloatExpr*;
    using reference = const FloatExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class IntervalVarArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit IntervalVarArray(void* o): Expr(o) {}  IntervalVarArray(): Expr() {}
  friend class IntervalVarArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef IntervalVar ItemType;
  static constexpr Expr::Type ExprType = IntervalVarArrayT;
  IntervalVarArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  IntervalVarArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  IntervalVarArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  IntervalVarArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const IntervalVar operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntervalVar(_at(index));
  }
  IntervalVar operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntervalVar(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const IntervalVar operator*() { return static_cast<const IntervalVar>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntervalVar;
    using pointer = const IntervalVar*;
    using reference = const IntervalVar;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    IntervalVar operator*() { return static_cast<IntervalVar>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntervalVar;
    using pointer = const IntervalVar*;
    using reference = const IntervalVar;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class CumulExprArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit CumulExprArray(void* o): Expr(o) {}  CumulExprArray(): Expr() {}
  friend class CumulExprArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef CumulExpr ItemType;
  static constexpr Expr::Type ExprType = CumulExprArrayT;
  CumulExprArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  CumulExprArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  CumulExprArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  CumulExprArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const CumulExpr operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return CumulExpr(_at(index));
  }
  CumulExpr operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return CumulExpr(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const CumulExpr operator*() { return static_cast<const CumulExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = CumulExpr;
    using pointer = const CumulExpr*;
    using reference = const CumulExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    CumulExpr operator*() { return static_cast<CumulExpr>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = CumulExpr;
    using pointer = const CumulExpr*;
    using reference = const CumulExpr;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class SequenceVarArray: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit SequenceVarArray(void* o): Expr(o) {}  SequenceVarArray(): Expr() {}
  friend class SequenceVarArrayArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef SequenceVar ItemType;
  static constexpr Expr::Type ExprType = SequenceVarArrayT;
  SequenceVarArray& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  SequenceVarArray& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  SequenceVarArray& name(Ts... args) {
    _name(args...);
    return *this;
  }
  SequenceVarArray& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const SequenceVar operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return SequenceVar(_at(index));
  }
  SequenceVar operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return SequenceVar(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const SequenceVar operator*() { return static_cast<const SequenceVar>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = SequenceVar;
    using pointer = const SequenceVar*;
    using reference = const SequenceVar;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    SequenceVar operator*() { return static_cast<SequenceVar>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = SequenceVar;
    using pointer = const SequenceVar*;
    using reference = const SequenceVar;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

class TupleSet: public Expr {
 private:
  friend class Model;
  friend class Expr;
  template <class T> friend class Opt;
  explicit TupleSet(void* o): Expr(o) {}  TupleSet(): Expr() {}
  friend class TupleSetArg;
  IlcInt _getSize() const;
  void* _at(IlcInt index) const;
  void *const* _begin() const;
  void *const* _end() const;
 public:
  typedef IntArray ItemType;
  static constexpr Expr::Type ExprType = TupleSetT;
  TupleSet& name(const char* name) {
    verify();
    _addName(name);
    return *this;
  }
  template<class Allocator>
  TupleSet& name(const std::basic_string<char, std::char_traits<char>, Allocator>& src) {
    verify();
    addName(src.data());
    return *this;
  }
  template <typename... Ts>
  TupleSet& name(Ts... args) {
    _name(args...);
    return *this;
  }
  TupleSet& setLocation(const char* file, IlcInt line) {
    verify();
    _setLocation(file, line);
    return *this;
  }
  IlcInt getSize() const {
    verify();
    return _getSize();
  }
  const IntArray operator[](IlcInt index) const {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntArray(_at(index));
  }
  IntArray operator[](IlcInt index) {
    verify();
    IlcCPOAssert(index >= 0 && index < _getSize(), "Invalid index while accessing an array.");
    return IntArray(_at(index));
  }
  class iterator;
  class const_iterator {
   private:
    void *const* _curr;
    friend class iterator;
   public:
    const_iterator(void *const* curr = nullptr) : _curr(curr) {}
    const_iterator& operator++() { _curr++; return *this;}
    const_iterator& operator--() { _curr--; return *this;}
    const_iterator operator++(int) { const_iterator retval = *this; _curr++; return retval; }
    const_iterator operator--(int) { const_iterator retval = *this; _curr--; return retval; }
    bool operator==(const const_iterator& other) const { return _curr == other._curr; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    const IntArray operator*() { return static_cast<const IntArray>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntArray;
    using pointer = const IntArray*;
    using reference = const IntArray;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  class iterator {
   private:
    void *const* _curr;
   public:
    iterator(void *const* curr = nullptr) : _curr(curr) {}
    iterator(const const_iterator& src): _curr(src._curr) {}
    iterator& operator++() { _curr++; return *this;}
    iterator& operator--() { _curr--; return *this;}
    iterator operator++(int) { iterator retval = *this; _curr++; return retval; }
    iterator operator--(int) { iterator retval = *this; _curr--; return retval; }
    bool operator==(const iterator& other) const { return _curr == other._curr; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    IntArray operator*() { return static_cast<IntArray>(*_curr); }
    using difference_type = std::ptrdiff_t;
    using value_type = IntArray;
    using pointer = const IntArray*;
    using reference = const IntArray;
    using iterator_category = std::bidirectional_iterator_tag;
  };
  iterator begin() const { verify(); return iterator(_begin()); }
  iterator end() const { verify(); return iterator(_end()); }
  const_iterator cbegin() const { verify(); return const_iterator(_begin()); }
  const_iterator cend() const { verify(); return const_iterator(_end()); }
};

typedef Opt<Expr> OptExpr;
typedef Opt<BoolConstant> OptBoolConstant;
typedef Opt<BoolExprArray> OptBoolExprArray;
typedef Opt<BoolExpr> OptBoolExpr;
typedef Opt<BoolVar> OptBoolVar;
typedef Opt<Constraint> OptConstraint;
typedef Opt<CumulAtom> OptCumulAtom;
typedef Opt<CumulExprArray> OptCumulExprArray;
typedef Opt<CumulExpr> OptCumulExpr;
typedef Opt<FloatArray> OptFloatArray;
typedef Opt<FloatConstant> OptFloatConstant;
typedef Opt<FloatExprArray> OptFloatExprArray;
typedef Opt<FloatExpr> OptFloatExpr;
typedef Opt<IntArray> OptIntArray;
typedef Opt<IntConstant> OptIntConstant;
typedef Opt<IntervalVarArray> OptIntervalVarArray;
typedef Opt<IntervalVar> OptIntervalVar;
typedef Opt<IntExprArray> OptIntExprArray;
typedef Opt<IntExpr> OptIntExpr;
typedef Opt<IntValueChooser> OptIntValueChooser;
typedef Opt<IntValueEval> OptIntValueEval;
typedef Opt<IntValueSelector> OptIntValueSelector;
typedef Opt<IntVarChooser> OptIntVarChooser;
typedef Opt<IntVarEval> OptIntVarEval;
typedef Opt<IntVarSelector> OptIntVarSelector;
typedef Opt<IntVar> OptIntVar;
typedef Opt<MutableBoolExpr> OptMutableBoolExpr;
typedef Opt<MutableIntExpr> OptMutableIntExpr;
typedef Opt<MutableFloatExpr> OptMutableFloatExpr;
typedef Opt<MutableCumulExpr> OptMutableCumulExpr;
typedef Opt<Objective> OptObjective;
typedef Opt<SearchPhase> OptSearchPhase;
typedef Opt<SegmentedFunction> OptSegmentedFunction;
typedef Opt<SequenceVarArray> OptSequenceVarArray;
typedef Opt<SequenceVar> OptSequenceVar;
typedef Opt<StateFunction> OptStateFunction;
typedef Opt<StepFunction> OptStepFunction;
typedef Opt<TransitionMatrix> OptTransitionMatrix;
typedef Opt<TupleSet> OptTupleSet;

class IntArrayArg {
 private:
  enum Type {
    Impl,
    Data
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  friend class IntVar;
  IntArray _getExpr() const { return IntArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const IlcInt* _getItems() const { return static_cast<const IlcInt*>(_data); }
 public:
  IntArrayArg(IntArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  IntArrayArg(const std::vector<IlcInt, Allocator>& src):
    _type(Data),
    _data(src.data()),
    _size(src.size())
  {}
  IntArrayArg(std::initializer_list<IlcInt> src):
    _type(Data),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntArrayArg(IlcInt (&c_array)[N]):
    _type(Data),
    _data(c_array),
    _size(N)
  {}
};

class IntervalVarArrayArg {
  enum Type {
    Impl,
    Data
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  IntervalVarArray _getExpr() const { return IntervalVarArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const IntervalVar* _getItems() const { return static_cast<const IntervalVar*>(_data); }
 public:
  IntervalVarArrayArg(IntervalVarArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  IntervalVarArrayArg(const std::vector<IntervalVar, Allocator>& src):
    _type(Data),
    _data(src.data()),
    _size(src.size())
  {}
  IntervalVarArrayArg(std::initializer_list<IntervalVar> src):
    _type(Data),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntervalVarArrayArg(IntervalVar (&c_array)[N]):
    _type(Data),
    _data(c_array),
    _size(N)
  {}
};

class CumulExprArrayArg {
  enum Type {
    Impl,
    Data
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  CumulExprArray _getExpr() const { return CumulExprArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const CumulExpr* _getItems() const { return static_cast<const CumulExpr*>(_data); }
 public:
  CumulExprArrayArg(CumulExprArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  CumulExprArrayArg(const std::vector<CumulExpr, Allocator>& src):
    _type(Data),
    _data(src.data()),
    _size(src.size())
  {}
  CumulExprArrayArg(std::initializer_list<CumulExpr> src):
    _type(Data),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  CumulExprArrayArg(CumulExpr (&c_array)[N]):
    _type(Data),
    _data(c_array),
    _size(N)
  {}
};

class SequenceVarArrayArg {
  enum Type {
    Impl,
    Data
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  SequenceVarArray _getExpr() const { return SequenceVarArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const SequenceVar* _getItems() const { return static_cast<const SequenceVar*>(_data); }
 public:
  SequenceVarArrayArg(SequenceVarArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  SequenceVarArrayArg(const std::vector<SequenceVar, Allocator>& src):
    _type(Data),
    _data(src.data()),
    _size(src.size())
  {}
  SequenceVarArrayArg(std::initializer_list<SequenceVar> src):
    _type(Data),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  SequenceVarArrayArg(SequenceVar (&c_array)[N]):
    _type(Data),
    _data(c_array),
    _size(N)
  {}
};

class TupleSetArg {
  enum Type {
    Impl,
    Data
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  TupleSet _getExpr() const { return TupleSet(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const IntArray* _getItems() const { return static_cast<const IntArray*>(_data); }
 public:
  TupleSetArg(TupleSet src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  TupleSetArg(const std::vector<IntArray, Allocator>& src):
    _type(Data),
    _data(src.data()),
    _size(src.size())
  {}
  TupleSetArg(std::initializer_list<IntArray> src):
    _type(Data),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  TupleSetArg(IntArray (&c_array)[N]):
    _type(Data),
    _data(c_array),
    _size(N)
  {}
};

class FloatArrayArg {
 private:
  enum Type {
    Impl,
    Floats
  };
  Type        _type;
  const void* _data;
  IlcInt      _size;
  friend class Model;
  FloatArray _getExpr() const { return FloatArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const IlcFloat* _getItems() const { return static_cast<const IlcFloat*>(_data); }
 public:
  
  FloatArrayArg(const FloatArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  FloatArrayArg(const std::vector<IlcFloat, Allocator>& src):
    _type(Floats),
    _data(src.data()),
    _size(src.size())
  {}
  FloatArrayArg(std::initializer_list<IlcFloat> src):
    _type(Floats),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  FloatArrayArg(IlcFloat (&c_array)[N]):
    _type(Floats),
    _data(c_array),
    _size(N)
  {}
};

class BoolExprArrayArg {
 private:
  enum Type {
    Impl,
    Exprs
  };
  Type         _type;
  const void*  _data;
  IlcInt       _size;
  friend class Model;
  BoolExprArray _getExpr() const { return BoolExprArray(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const BoolExpr* _getItems() const { return static_cast<const BoolExpr*>(_data); }
 public:
  
  BoolExprArrayArg(const BoolExprArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  BoolExprArrayArg(const std::vector<BoolExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  BoolExprArrayArg(std::initializer_list<BoolExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  BoolExprArrayArg(BoolExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  BoolExprArrayArg(const std::vector<BoolVar, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  BoolExprArrayArg(std::initializer_list<BoolVar> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  BoolExprArrayArg(BoolVar (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  BoolExprArrayArg(const std::vector<MutableBoolExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  BoolExprArrayArg(std::initializer_list<MutableBoolExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  BoolExprArrayArg(MutableBoolExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
};

class IntExprArrayArg {
 private:
  enum Type {
    Impl,
    Exprs
  };
  Type         _type;
  const void*  _data;
  IlcInt       _size;
  friend class Model;
  Expr _getExpr() const { return Expr(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const Expr* _getItems() const { return static_cast<const Expr*>(_data); }
 public:
  
  IntExprArrayArg(const IntExprArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  IntExprArrayArg(const std::vector<IntExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<IntExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(IntExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  IntExprArrayArg(const std::vector<IntVar, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<IntVar> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(IntVar (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  IntExprArrayArg(const std::vector<MutableIntExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<MutableIntExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(MutableIntExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  IntExprArrayArg(const BoolExprArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  IntExprArrayArg(const std::vector<BoolExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<BoolExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(BoolExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  IntExprArrayArg(const std::vector<BoolVar, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<BoolVar> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(BoolVar (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  IntExprArrayArg(const std::vector<MutableBoolExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  IntExprArrayArg(std::initializer_list<MutableBoolExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  IntExprArrayArg(MutableBoolExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
};

class FloatExprArrayArg {
  enum Type {
    Impl,
    Exprs
  };
  Type         _type;
  const void*  _data;
  IlcInt       _size;
  friend class Model;
  Expr _getExpr() const { return Expr(const_cast<void*>(_data)); }
  IlcInt _getSize() const { return _size; }
  const Expr* _getItems() const { return static_cast<const Expr*>(_data); }
 public:
  
  FloatExprArrayArg(const FloatExprArray src):
    _type(Impl),
    _data(src._impl),
    _size(-1)
  {}
  template <class Allocator>
  FloatExprArrayArg(const std::vector<FloatExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  FloatExprArrayArg(std::initializer_list<FloatExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  FloatExprArrayArg(FloatExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
  
  template <class Allocator>
  FloatExprArrayArg(const std::vector<MutableFloatExpr, Allocator>& src):
    _type(Exprs),
    _data(src.data()),
    _size(src.size())
  {}
  FloatExprArrayArg(std::initializer_list<MutableFloatExpr> src):
    _type(Exprs),
    _data(src.begin()),
    _size(src.size())
  {}
  template <size_t N>
  FloatExprArrayArg(MutableFloatExpr (&c_array)[N]):
    _type(Exprs),
    _data(c_array),
    _size(N)
  {}
};

class Model {
 private:
  void* _impl;

  explicit Model(void* m): _impl(m) {}
  friend class Expr;
  friend class IntVar;
  friend class CP;

  friend IntExpr operator +(IntExpr operand1, IntExpr operand2);
  friend IntExpr operator +(IntExpr operand1, IlcInt operand2);
  friend IntExpr operator +(IlcInt operand1, IntExpr operand2);
  friend IntExpr operator -(IntExpr operand1, IntExpr operand2);
  friend IntExpr operator -(IntExpr operand1, IlcInt operand2);
  friend IntExpr operator -(IlcInt operand1, IntExpr operand2);
  friend IntExpr operator -(IntExpr operand);
  friend IntExpr operator *(IntExpr operand1, IntExpr operand2);
  friend IntExpr operator *(IntExpr operand1, IlcInt operand2);
  friend IntExpr operator *(IlcInt operand1, IntExpr operand2);
  friend IntExpr operator /(IntExpr operand1, IntExpr operand2);
  friend IntExpr operator /(IntExpr operand1, IlcInt operand2);
  friend IntExpr operator /(IlcInt operand1, IntExpr operand2);
  friend IntExpr operator %(IntExpr operand1, IntExpr operand2);
  friend IntExpr operator %(IntExpr operand1, IlcInt operand2);
  friend IntExpr operator %(IlcInt operand1, IntExpr operand2);
  friend FloatExpr operator +(FloatExpr operand1, FloatExpr operand2);
  friend FloatExpr operator +(FloatExpr operand1, IlcFloat operand2);
  friend FloatExpr operator +(IlcFloat operand1, FloatExpr operand2);
  friend FloatExpr operator -(FloatExpr operand1, FloatExpr operand2);
  friend FloatExpr operator -(FloatExpr operand1, IlcFloat operand2);
  friend FloatExpr operator -(IlcFloat operand1, FloatExpr operand2);
  friend FloatExpr operator -(FloatExpr operand);
  friend FloatExpr operator *(FloatExpr operand1, FloatExpr operand2);
  friend FloatExpr operator *(FloatExpr operand1, IlcFloat operand2);
  friend FloatExpr operator *(IlcFloat operand1, FloatExpr operand2);
  friend FloatExpr operator /(FloatExpr operand1, FloatExpr operand2);
  friend FloatExpr operator /(FloatExpr operand1, IlcFloat operand2);
  friend FloatExpr operator /(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator ==(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator ==(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator ==(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator ==(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator ==(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator ==(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator !=(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator !=(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator !=(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator !=(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator !=(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator !=(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator >=(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator >=(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator >=(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator >=(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator >=(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator >=(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator <=(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator <=(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator <=(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator <=(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator <=(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator <=(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator >(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator >(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator >(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator >(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator >(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator >(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator <(IntExpr operand1, IntExpr operand2);
  friend BoolExpr operator <(IntExpr operand1, IlcInt operand2);
  friend BoolExpr operator <(IlcInt operand1, IntExpr operand2);
  friend BoolExpr operator <(FloatExpr operand1, FloatExpr operand2);
  friend BoolExpr operator <(FloatExpr operand1, IlcFloat operand2);
  friend BoolExpr operator <(IlcFloat operand1, FloatExpr operand2);
  friend BoolExpr operator ||(BoolExpr operand1, BoolExpr operand2);
  friend BoolExpr operator &&(BoolExpr operand1, BoolExpr operand2);
  friend BoolExpr operator !(BoolExpr operand);
  friend CumulExpr operator +(CumulExpr operand1, CumulExpr operand2);
  friend CumulExpr operator -(CumulExpr operand1, CumulExpr operand2);
  friend CumulExpr operator -(CumulExpr operand);
  friend Constraint operator >=(IlcInt max, CumulExpr function);
  friend Constraint operator <=(CumulExpr function, IlcInt max);
  friend Constraint operator >=(CumulExpr function, IlcInt min);
  friend Constraint operator <=(IlcInt min, CumulExpr function);
  friend Constraint operator <=(CumulExpr function, IntExpr max);
  friend Constraint operator >=(CumulExpr function, IntExpr min);
  friend Constraint operator >=(IntExpr max, CumulExpr function);
  friend Constraint operator <=(IntExpr min, CumulExpr function);

  void verify() const {
    IlcCPOAssert(_impl, "Empty handle (unitialized class).");
  }
  void verify(Expr e) const {
    e.verify();
    IlcCPOAssert(e.getModel()._impl == _impl, "Expression belongs to other Model.");
  }
  static void Verify(Expr e) {
    e.verify();
  }
  static void Verify(Expr e1, Expr e2) {
    e1.verify();
    e2.verify();
    e1.verifyModel(e2);
  }

  void _add(void*);
  void _remove(void*);

  void _dispose(void*);

  void* _intExpr(IlcInt);
  void* _floatExpr(IlcFloat);

  void* _intVar();
  void* _boolVar();
  void* _intervalVar();

  void* _mutableBoolExpr();
  void* _mutableIntExpr();
  void* _mutableFloatExpr();
  void* _mutableCumulExpr();

  void* _inferred(void*);

  void* _intArray(IlcInt size, const IlcInt* data);
  void* _floatArray(IlcInt size, const IlcFloat* data);
  void* _boolExprArray(IlcInt size, const void* data);
  void* _intExprArray(IlcInt size, const void* data);
  void* _intExprArray(IlcInt size, const IlcInt* data);
  void* _floatExprArray(IlcInt size, const void* data);
  void* _floatExprArray(IlcInt size, const IlcFloat* data);
  void* _intervalVarArray(IlcInt size, const void* data);
  void* _cumulExprArray(IlcInt size, const void* data);
  void* _sequenceVarArray(IlcInt size, const void* data);
  void* _tupleSet(IlcInt size, const void* data);

  void* fromArg(FloatExpr arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntExpr arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(BoolExpr arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(Constraint arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntervalVar arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(CumulExpr arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(Objective arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(SegmentedFunction arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(StepFunction arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(SequenceVar arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(CumulAtom arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(StateFunction arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(TransitionMatrix arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntVarEval arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntValueEval arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntVarSelector arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntValueSelector arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntVarChooser arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntValueChooser arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(SearchPhase arg) {
    verify(arg);
    return arg._impl;
  }
  void* fromArg(IntArrayArg arg) {
    if (arg._type == IntArrayArg::Impl) {
      Expr theArg = arg._getExpr();
      verify(theArg);
      return theArg._impl;
    }
    return _intArray(arg._getSize(), arg._getItems());
  }
  void* fromArg(FloatArrayArg arg) {
    if (arg._type == FloatArrayArg::Impl) {
      Expr theArg = arg._getExpr();
      verify(theArg);
      return theArg._impl;
    }
    return _floatArray(arg._getSize(), arg._getItems());
  }
  void* fromArg(IntExprArrayArg arg) {
    if (arg._type == IntExprArrayArg::Impl) {
      Expr theArg = arg._getExpr();
      verify(theArg);
      return theArg._impl;
    }
    IlcInt size = arg._getSize();
    const Expr* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _intExprArray(size, items);
  }
  void* fromArg(FloatExprArrayArg arg) {
    if (arg._type == FloatExprArrayArg::Impl) {
      Expr theArg = arg._getExpr();
      verify(theArg);
      return theArg._impl;
    }
    IlcInt size = arg._getSize();
    const Expr* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _floatExprArray(size, items);
  }
  void* fromArg(BoolExprArrayArg arg) {
    if (arg._type == BoolExprArrayArg::Impl) {
       BoolExprArray result = arg._getExpr();
       verify(result);
       return result._impl;
    }
    IlcInt size = arg._getSize();
    const BoolExpr* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _boolExprArray(size, items);
  }
  void* fromArg(IntervalVarArrayArg arg) {
    if (arg._type == IntervalVarArrayArg::Impl) {
       IntervalVarArray result = arg._getExpr();
       verify(result);
       return result._impl;
    }
    IlcInt size = arg._getSize();
    const IntervalVar* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _intervalVarArray(size, items);
  }
  void* fromArg(CumulExprArrayArg arg) {
    if (arg._type == CumulExprArrayArg::Impl) {
       CumulExprArray result = arg._getExpr();
       verify(result);
       return result._impl;
    }
    IlcInt size = arg._getSize();
    const CumulExpr* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _cumulExprArray(size, items);
  }
  void* fromArg(SequenceVarArrayArg arg) {
    if (arg._type == SequenceVarArrayArg::Impl) {
       SequenceVarArray result = arg._getExpr();
       verify(result);
       return result._impl;
    }
    IlcInt size = arg._getSize();
    const SequenceVar* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _sequenceVarArray(size, items);
  }
  void* fromArg(TupleSetArg arg) {
    if (arg._type == TupleSetArg::Impl) {
       TupleSet result = arg._getExpr();
       verify(result);
       return result._impl;
    }
    IlcInt size = arg._getSize();
    const IntArray* items = arg._getItems();
    for (IlcInt i = 0; i < size; i++)
      verify(items[i]);
    return _tupleSet(size, items);
  }

  void* _getExprByName(const char* name) const;
  void* _getExprById(IlcInt id) const;

  BoolExpr _true1();
  BoolExpr _false1();
  static IntExpr _Plus1(void* operand1, void* operand2);
  static IntExpr _Plus2(void* operand1, IlcInt operand2);
  static IntExpr _Plus3(IlcInt operand1, void* operand2);
  static IntExpr _Minus1(void* operand1, void* operand2);
  static IntExpr _Minus2(void* operand1, IlcInt operand2);
  static IntExpr _Minus3(IlcInt operand1, void* operand2);
  static IntExpr _UnaryMinus1(void* operand);
  static IntExpr _Times1(void* operand1, void* operand2);
  static IntExpr _Times2(void* operand1, IlcInt operand2);
  static IntExpr _Times3(IlcInt operand1, void* operand2);
  static IntExpr _IntDiv1(void* operand1, void* operand2);
  static IntExpr _IntDiv2(void* operand1, IlcInt operand2);
  static IntExpr _IntDiv3(IlcInt operand1, void* operand2);
  static IntExpr _Mod1(void* operand1, void* operand2);
  static IntExpr _Mod2(void* operand1, IlcInt operand2);
  static IntExpr _Mod3(IlcInt operand1, void* operand2);
  static FloatExpr _Plus4(void* operand1, void* operand2);
  static FloatExpr _Plus5(void* operand1, IlcFloat operand2);
  static FloatExpr _Plus6(IlcFloat operand1, void* operand2);
  static FloatExpr _Minus4(void* operand1, void* operand2);
  static FloatExpr _Minus5(void* operand1, IlcFloat operand2);
  static FloatExpr _Minus6(IlcFloat operand1, void* operand2);
  static FloatExpr _UnaryMinus2(void* operand);
  static FloatExpr _Times4(void* operand1, void* operand2);
  static FloatExpr _Times5(void* operand1, IlcFloat operand2);
  static FloatExpr _Times6(IlcFloat operand1, void* operand2);
  static FloatExpr _FloatDiv1(void* operand1, void* operand2);
  static FloatExpr _FloatDiv2(void* operand1, IlcFloat operand2);
  static FloatExpr _FloatDiv3(IlcFloat operand1, void* operand2);
  FloatExpr _exp1(void* operand1, void* operand2);
  FloatExpr _exp2(void* operand1, IlcFloat operand2);
  FloatExpr _exp3(IlcFloat operand1, void* operand2);
  static BoolExpr _Equal1(void* operand1, void* operand2);
  static BoolExpr _Equal2(void* operand1, IlcInt operand2);
  static BoolExpr _Equal3(IlcInt operand1, void* operand2);
  static BoolExpr _Equal4(void* operand1, void* operand2);
  static BoolExpr _Equal5(void* operand1, IlcFloat operand2);
  static BoolExpr _Equal6(IlcFloat operand1, void* operand2);
  static BoolExpr _Diff1(void* operand1, void* operand2);
  static BoolExpr _Diff2(void* operand1, IlcInt operand2);
  static BoolExpr _Diff3(IlcInt operand1, void* operand2);
  static BoolExpr _Diff4(void* operand1, void* operand2);
  static BoolExpr _Diff5(void* operand1, IlcFloat operand2);
  static BoolExpr _Diff6(IlcFloat operand1, void* operand2);
  static BoolExpr _GreaterOrEqual1(void* operand1, void* operand2);
  static BoolExpr _GreaterOrEqual2(void* operand1, IlcInt operand2);
  static BoolExpr _GreaterOrEqual3(IlcInt operand1, void* operand2);
  static BoolExpr _GreaterOrEqual4(void* operand1, void* operand2);
  static BoolExpr _GreaterOrEqual5(void* operand1, IlcFloat operand2);
  static BoolExpr _GreaterOrEqual6(IlcFloat operand1, void* operand2);
  static BoolExpr _LessOrEqual1(void* operand1, void* operand2);
  static BoolExpr _LessOrEqual2(void* operand1, IlcInt operand2);
  static BoolExpr _LessOrEqual3(IlcInt operand1, void* operand2);
  static BoolExpr _LessOrEqual4(void* operand1, void* operand2);
  static BoolExpr _LessOrEqual5(void* operand1, IlcFloat operand2);
  static BoolExpr _LessOrEqual6(IlcFloat operand1, void* operand2);
  static BoolExpr _Greater1(void* operand1, void* operand2);
  static BoolExpr _Greater2(void* operand1, IlcInt operand2);
  static BoolExpr _Greater3(IlcInt operand1, void* operand2);
  static BoolExpr _Greater4(void* operand1, void* operand2);
  static BoolExpr _Greater5(void* operand1, IlcFloat operand2);
  static BoolExpr _Greater6(IlcFloat operand1, void* operand2);
  static BoolExpr _Less1(void* operand1, void* operand2);
  static BoolExpr _Less2(void* operand1, IlcInt operand2);
  static BoolExpr _Less3(IlcInt operand1, void* operand2);
  static BoolExpr _Less4(void* operand1, void* operand2);
  static BoolExpr _Less5(void* operand1, IlcFloat operand2);
  static BoolExpr _Less6(IlcFloat operand1, void* operand2);
  static BoolExpr _Or1(void* operand1, void* operand2);
  static BoolExpr _And1(void* operand1, void* operand2);
  BoolExpr _ifThen1(void* operand1, void* operand2);
  static BoolExpr _Not1(void* operand);
  BoolExpr _conjunction1(void* a);
  BoolExpr _disjunction1(void* a);
  IntExpr _abs1(void* x);
  IntExpr _square1(void* x);
  IntExpr _min1(void* x, void* y);
  IntExpr _min2(void* x, IlcInt y);
  IntExpr _min3(IlcInt x, void* y);
  IntExpr _max1(void* x, void* y);
  IntExpr _max2(void* x, IlcInt y);
  IntExpr _max3(IlcInt x, void* y);
  IntExpr _min4(void* a);
  IntExpr _max4(void* a);
  IntExpr _sum1(void* x);
  IntExpr _scalProd1(void* x, void* y);
  IntExpr _scalProd2(void* x, void* y);
  IntExpr _scalProd3(void* x, void* y);
  IntExpr _count1(void* exprs, IlcInt value);
  IntExpr _element1(void* array, void* subscript);
  IntExpr _element2(void* array, void* subscript);
  IntExpr _element3(void* array, IlcInt subscript);
  IntExpr _countDifferent1(void* array);
  IntExpr _floatToInt1(void* x);
  FloatExpr _abs2(void* x);
  FloatExpr _exponent1(void* x);
  FloatExpr _log1(void* x);
  FloatExpr _square2(void* x);
  FloatExpr _min5(void* x, void* y);
  FloatExpr _min6(void* x, IlcFloat y);
  FloatExpr _min7(IlcFloat x, void* y);
  FloatExpr _max5(void* x, void* y);
  FloatExpr _max6(void* x, IlcFloat y);
  FloatExpr _max7(IlcFloat x, void* y);
  FloatExpr _min8(void* a);
  FloatExpr _max8(void* a);
  FloatExpr _sum2(void* x);
  FloatExpr _scalProd4(void* x, void* y);
  FloatExpr _scalProd5(void* x, void* y);
  FloatExpr _scalProd6(void* x, void* y);
  FloatExpr _scalProd7(void* x, void* y);
  FloatExpr _scalProd8(void* x, void* y);
  FloatExpr _scalProd9(void* x, void* y);
  FloatExpr _scalProd10(void* x, void* y);
  FloatExpr _element4(void* array, void* subscript);
  FloatExpr _standardDeviation1(void* x, IlcFloat meanLB, IlcFloat meanUB);
  FloatExpr _slopePiecewiseLinear1(void* x, void* points, void* slopes, IlcFloat refX, IlcFloat refY);
  FloatExpr _coordinatePiecewiseLinear1(void* x, IlcFloat firstSlope, void* points, void* values, IlcFloat lastSlope);
  BoolExpr _allowedAssignments1(void* expr, void* values);
  BoolExpr _forbiddenAssignments1(void* expr, void* values);
  BoolExpr _allowedAssignments2(void* exprs, void* tuples);
  BoolExpr _forbiddenAssignments2(void* exprs, void* tuples);
  Constraint _inverse1(void* f, void* invf);
  BoolExpr _allDiff1(void* x);
  BoolExpr _allMinDistance1(void* exprs, IlcInt distance);
  Constraint _distribute1(void* counts, void* values, void* exprs);
  Constraint _distribute2(void* counts, void* exprs);
  Constraint _pack1(void* load, void* where, void* size, void* used);
  Constraint _pack2(void* load, void* where, void* size, IlcInt used);
  BoolExpr _lexicographic1(void* x, void* y);
  BoolExpr _strictLexicographic1(void* x, void* y);
  Constraint _sequence1(IlcInt min, IlcInt max, IlcInt width, void* exprs, void* values, void* cards);
  Constraint _abstraction1(void* y, void* x, void* values, IlcInt abstractValue);
  Constraint _boolAbstraction1(void* y, void* x, void* values);
  Constraint _spread1(void* foo1, void* foo2, void* foo3);
  Constraint _spread2(void* foo1, void* foo2, IlcFloat foo3);
  Constraint _spread3(void* foo1, IlcFloat foo2, void* foo3);
  Constraint _spread4(void* foo1, IlcFloat foo2, IlcFloat foo3);
  Constraint _equalOrEscape1(void* foo1, void* foo2, IlcInt escapeValue);
  Constraint _equalOrEscape2(void* foo1, IlcInt foo2, IlcInt escapeValue);
  Constraint _equalOrEscape3(IlcInt foo1, void* foo2, IlcInt escapeValue);
  Constraint _strong1(void* x);
  BoolExpr _range1(void* x, IlcFloat lB, IlcFloat uB);
  Constraint _subCircuit1(void* array);
  Objective _minimize1(void* expr);
  Objective _maximize1(void* expr);
  Objective _minimizeStaticLex1(void* exprs);
  Objective _minimizeStaticLex2(void* exprs);
  Objective _maximizeStaticLex1(void* exprs);
  Objective _maximizeStaticLex2(void* exprs);
  Constraint _exoticObject1();
  BoolExpr _presenceOf1(void* interval);
  IntExpr _startOf1(void* interval, IlcInt absentValue);
  IntExpr _endOf1(void* interval, IlcInt absentValue);
  IntExpr _sizeOf1(void* interval, IlcInt absentValue);
  IntExpr _lengthOf1(void* interval, IlcInt absentValue);
  IntExpr _overlapLength1(void* interval1, void* interval2, IlcInt absentValue);
  IntExpr _overlapLength2(void* interval, IlcInt start, IlcInt end, IlcInt absentValue);
  FloatExpr _startEval1(void* interval, void* function, IlcFloat absentValue);
  FloatExpr _endEval1(void* interval, void* function, IlcFloat absentValue);
  FloatExpr _sizeEval1(void* interval, void* function, IlcFloat absentValue);
  FloatExpr _lengthEval1(void* interval, void* function, IlcFloat absentValue);
  Constraint _alternative1(void* interval, void* array, void* cardinality);
  Constraint _alternative2(void* interval, void* array, IlcInt cardinality);
  Constraint _span1(void* interval, void* array);
  Constraint _synchronize1(void* interval, void* array);
  Constraint _endBeforeEnd1(void* predecessor, void* successor, void* minDelay);
  Constraint _endBeforeEnd2(void* predecessor, void* successor, IlcInt minDelay);
  Constraint _endBeforeStart1(void* predecessor, void* successor, void* minDelay);
  Constraint _endBeforeStart2(void* predecessor, void* successor, IlcInt minDelay);
  Constraint _endAtEnd1(void* a, void* b, void* delay);
  Constraint _endAtEnd2(void* a, void* b, IlcInt delay);
  Constraint _endAtStart1(void* a, void* b, void* delay);
  Constraint _endAtStart2(void* a, void* b, IlcInt delay);
  Constraint _startBeforeEnd1(void* predecessor, void* successor, void* minDelay);
  Constraint _startBeforeEnd2(void* predecessor, void* successor, IlcInt minDelay);
  Constraint _startBeforeStart1(void* predecessor, void* successor, void* minDelay);
  Constraint _startBeforeStart2(void* predecessor, void* successor, IlcInt minDelay);
  Constraint _startAtEnd1(void* a, void* b, void* delay);
  Constraint _startAtEnd2(void* a, void* b, IlcInt delay);
  Constraint _startAtStart1(void* a, void* b, void* delay);
  Constraint _startAtStart2(void* a, void* b, IlcInt delay);
  Constraint _forbidStart1(void* interval, void* function);
  Constraint _forbidEnd1(void* interval, void* function);
  Constraint _forbidExtent1(void* interval, void* function);
  Constraint _isomorphism1(void* array1, void* array2, void* map, IlcInt absentValue);
  SequenceVar _sequenceVar1(void* intervals, void* types);
  IntExpr _typeOfNext1(void* sequence, void* interval, IlcInt lastValue, IlcInt absentValue);
  IntExpr _startOfNext1(void* sequence, void* interval, IlcInt lastValue, IlcInt absentValue);
  IntExpr _endOfNext1(void* sequence, void* interval, IlcInt lastValue, IlcInt absentValue);
  IntExpr _sizeOfNext1(void* sequence, void* interval, IlcInt lastValue, IlcInt absentValue);
  IntExpr _lengthOfNext1(void* sequence, void* interval, IlcInt lastValue, IlcInt absentValue);
  IntExpr _typeOfPrev1(void* sequence, void* interval, IlcInt firstValue, IlcInt absentValue);
  IntExpr _startOfPrev1(void* sequence, void* interval, IlcInt firstValue, IlcInt absentValue);
  IntExpr _endOfPrev1(void* sequence, void* interval, IlcInt firstValue, IlcInt absentValue);
  IntExpr _sizeOfPrev1(void* sequence, void* interval, IlcInt firstValue, IlcInt absentValue);
  IntExpr _lengthOfPrev1(void* sequence, void* interval, IlcInt firstValue, IlcInt absentValue);
  Constraint _noOverlap1(void* sequence, void* distanceMatrix, bool isDirect);
  Constraint _noOverlap2(void* intervals);
  Constraint _first1(void* sequence, void* interval);
  Constraint _last1(void* sequence, void* interval);
  Constraint _previous1(void* sequence, void* interval1, void* interval2);
  Constraint _before1(void* sequence, void* interval1, void* interval2);
  Constraint _sameSequence1(void* seq1, void* seq2);
  Constraint _sameSequence2(void* seq1, void* seq2, void* array1, void* array2);
  Constraint _sameCommonSubsequence1(void* seq1, void* seq2);
  Constraint _sameCommonSubsequence2(void* seq1, void* seq2, void* array1, void* array2);
  CumulAtom _pulse1(IlcInt start, IlcInt end, IlcInt height);
  CumulAtom _stepAt1(IlcInt time, IlcInt height);
  CumulAtom _pulse2(void* interval, IlcInt heightMin);
  CumulAtom _pulse3(void* interval, IlcInt heightMin, IlcInt heightMax);
  CumulAtom _stepAtStart1(void* interval, IlcInt heightMin);
  CumulAtom _stepAtStart2(void* interval, IlcInt heightMin, IlcInt heightMax);
  CumulAtom _stepAtEnd1(void* interval, IlcInt heightMin);
  CumulAtom _stepAtEnd2(void* interval, IlcInt heightMin, IlcInt heightMax);
  static CumulExpr _Plus7(void* operand1, void* operand2);
  static CumulExpr _Minus7(void* operand1, void* operand2);
  static CumulExpr _UnaryMinus3(void* operand);
  CumulExpr _sum3(void* x);
  IntExpr _heightAtStart1(void* interval, void* function, IlcInt absentValue);
  IntExpr _heightAtEnd1(void* interval, void* function, IlcInt absentValue);
  Constraint _alwaysIn1(void* function, void* interval, IlcInt min, IlcInt max);
  Constraint _alwaysIn2(void* function, IlcInt start, IlcInt end, IlcInt min, IlcInt max);
  static Constraint _GreaterOrEqual7(IlcInt max, void* function);
  static Constraint _LessOrEqual7(void* function, IlcInt max);
  static Constraint _GreaterOrEqual8(void* function, IlcInt min);
  static Constraint _LessOrEqual8(IlcInt min, void* function);
  Constraint _cumulRange1(void* function, void* min, void* max);
  Constraint _cumulRange2(void* function, void* min, IlcInt max);
  Constraint _cumulRange3(void* function, IlcInt min, void* max);
  Constraint _cumulRange4(void* function, IlcInt min, IlcInt max);
  static Constraint _LessOrEqual9(void* function, void* max);
  static Constraint _GreaterOrEqual9(void* function, void* min);
  static Constraint _GreaterOrEqual10(void* max, void* function);
  static Constraint _LessOrEqual10(void* min, void* function);
  StateFunction _stateFunction1(void* distanceMatrix);
  Constraint _alwaysNoState1(void* function, void* interval);
  Constraint _alwaysNoState2(void* function, IlcInt start, IlcInt end);
  Constraint _alwaysIn3(void* function, void* interval, IlcInt min, IlcInt max);
  Constraint _alwaysIn4(void* function, IlcInt start, IlcInt end, IlcInt min, IlcInt max);
  Constraint _alwaysConstant1(void* function, void* interval, bool isStartAligned, bool isEndAligned);
  Constraint _alwaysConstant2(void* function, IlcInt start, IlcInt end, bool isStartAligned, bool isEndAligned);
  Constraint _alwaysEqual1(void* function, void* interval, IlcInt min, bool isStartAligned, bool isEndAligned);
  Constraint _alwaysEqual2(void* function, IlcInt start, IlcInt end, IlcInt min, bool isStartAligned, bool isEndAligned);
  IntVarEval _domainSize1();
  IntVarEval _domainMax1();
  IntVarEval _domainMin1();
  IntVarEval _varImpact1();
  IntVarEval _varSuccessRate1();
  IntVarEval _impactOfLastBranch1();
  IntVarEval _varIndex1(void* x, IlcFloat defaultEval);
  IntVarEval _varLocalImpact1(IlcInt effort);
  IntVarEval _explicitVarEval1(void* x, void* evalArray, IlcFloat defaultEval);
  IntValueEval _value1();
  IntValueEval _valueImpact1();
  IntValueEval _valueSuccessRate1();
  IntValueEval _valueIndex1(void* valueEval, IlcFloat defaultEval);
  IntValueEval _explicitValueEval1(void* valueArray, void* evalArray, IlcFloat defaultEval);
  IntVarSelector _selectSmallest1(IlcFloat tolerance, void* eval);
  IntVarSelector _selectSmallest2(void* eval, IlcFloat tolerance);
  IntVarSelector _selectLargest1(IlcFloat tolerance, void* eval);
  IntVarSelector _selectLargest2(void* eval, IlcFloat tolerance);
  IntVarSelector _selectRandomVar1();
  IntValueSelector _selectSmallest3(IlcFloat tolerance, void* eval);
  IntValueSelector _selectSmallest4(void* eval, IlcFloat tolerance);
  IntValueSelector _selectLargest3(IlcFloat tolerance, void* eval);
  IntValueSelector _selectLargest4(void* eval, IlcFloat tolerance);
  IntValueSelector _selectRandomValue1();
  SearchPhase _searchPhase1(void* variables);
  SearchPhase _searchPhase2(void* variableChooser, void* valueChooser);
  SearchPhase _searchPhase3(void* variables, void* variableChooser, void* valueChooser);
  SearchPhase _searchPhase4(void* variables);
  SearchPhase _searchPhase5(void* variables);

 public:
  Model(CP cp);

  IntExpr   intExpr(IlcInt v)     { verify(); return IntExpr(_intExpr(v)); }
  FloatExpr floatExpr(IlcFloat v) { verify(); return FloatExpr(_floatExpr(v)); }
  BoolExpr  boolExpr(bool b)      { verify(); return b ? _true1() : _false1(); }

  IntVar intVar() {
    verify();
    return IntVar(_intVar());
  }
  Constraint inferred(IntVar intVar) {
    verify();
    return Constraint(_inferred(intVar.getImpl()));
  }
  BoolVar boolVar() {
    verify();
    return BoolVar(_boolVar());
  }
  IntervalVar intervalVar() {
    verify();
    return IntervalVar(_intervalVar());
  }

  std::vector<IntVar> intVarVector(IlcInt n) {
    verify();
    IlcCPOAssert(n >= 0, "Specified size of the array is negative.");
    std::vector<IntVar> result;
    result.reserve(n);
    for (IlcInt i = 0; i < n; i++)
      result.push_back(IntVar(_intVar()));
    return result;
  }
  std::vector<BoolVar> boolVarVector(IlcInt n) {
    verify();
    IlcCPOAssert(n >= 0, "Specified size of the array is negative.");
    std::vector<BoolVar> result;
    result.reserve(n);
    for (IlcInt i = 0; i < n; i++)
      result.push_back(BoolVar(_boolVar()));
    return result;
  }

  MutableBoolExpr mutableBoolExpr() {
    verify();
    return MutableBoolExpr(_mutableBoolExpr());
  }
  MutableIntExpr mutableIntExpr() {
    verify();
    return MutableIntExpr(_mutableIntExpr());
  }
  MutableFloatExpr mutableFloatExpr() {
    verify();
    return MutableFloatExpr(_mutableFloatExpr());
  }
  MutableCumulExpr mutableCumulExpr() {
    verify();
    return MutableCumulExpr(_mutableCumulExpr());
  }

  IntArray intArray(IntArrayArg items) {
    verify();
    return IntArray(fromArg(items));
  }
  FloatArray floatArray(FloatArrayArg items) {
    verify();
    return FloatArray(fromArg(items));
  }
  FloatArray floatArray(IntArrayArg items) {
    verify();
    const IlcInt* array;
    IlcInt size;
    if (items._type == IntArrayArg::Impl) {
      IntArray theArg = items._getExpr();
      verify(theArg);
      array = theArg.begin();
      size = theArg.getSize();
    } else {
      array = items._getItems();
      size = items._getSize();
    }
    std::vector<IlcFloat> tmp;
    tmp.reserve(size);
    for (IlcInt i = 0; i < size; i++)
      tmp.push_back(array[i]);
    return FloatArray(_floatArray(size, tmp.data()));
  }
  BoolExprArray boolExprArray(BoolExprArrayArg items) {
    verify();
    return BoolExprArray(fromArg(items));
  }
  IntExprArray intExprArray(IntExprArrayArg items) {
    verify();
    return IntExprArray(fromArg(items));
  }
  IntExprArray intExprArray(IntArrayArg items) {
    verify();
    const IlcInt* array;
    IlcInt size;
    if (items._type == IntArrayArg::Impl) {
      IntArray theArg = items._getExpr();
      verify(theArg);
      array = theArg.begin();
      size = theArg.getSize();
    } else {
      array = items._getItems();
      size = items._getSize();
    }
    return IntExprArray(_intExprArray(size, array));
  }
  FloatExprArray floatExprArray(FloatExprArrayArg items) {
    verify();
    return FloatExprArray(fromArg(items));
  }
  FloatExprArray floatExprArray(IntExprArrayArg items) {
    verify();
    return IntExprArray(fromArg(items));
  }
  FloatExprArray floatExprArray(FloatArrayArg items) {
    verify();
    const IlcFloat* array;
    IlcInt size;
    if (items._type == FloatArrayArg::Impl) {
      FloatArray theArg = items._getExpr();
      verify(theArg);
      array = theArg.begin();
      size = theArg.getSize();
    } else {
      array = items._getItems();
      size = items._getSize();
    }
    return FloatExprArray(_floatExprArray(size, array));
  }
  FloatExprArray floatExprArray(IntArrayArg items) {
    return intExprArray(items);
  }
  TupleSet tupleSet(TupleSetArg tuples) {
    verify();
    return TupleSet(fromArg(tuples));
  }

  Constraint add(Constraint c) {
    verify();
    _add(fromArg(c));
    return c;
  }
  BoolExpr add(BoolExpr c) {
    verify();
    _add(fromArg(c));
    return c;
  }
  Objective add(Objective o) {
    verify();
    _add(fromArg(o));
    return o;
  }
  Constraint remove(Constraint c) {
    verify();
    _remove(fromArg(c));
    return c;
  }
  BoolExpr remove(BoolExpr c) {
    verify();
    _remove(fromArg(c));
    return c;
  }
  Objective remove(Objective o) {
    verify();
    _remove(fromArg(o));
    return o;
  }

  OptExpr getExprByName(const char* name) const {
    verify();
    return Expr(_getExprByName(name));
  }
  template <class Allocator>
  OptExpr getExprByName(const std::basic_string<char, std::char_traits<char>, Allocator>& name) const {
    verify();
    return Expr(_getExprByName(name.data()));
  }
  template <typename... Ts>
  OptExpr getExprByName(Ts... nameArgs) const {
    verify();
    std::stringstream buffer;
    buffer.imbue(std::locale::classic());
    Expr::_nameAux(buffer, nameArgs...);
    return OptExpr(Expr(_getExprByName(buffer.str().c_str())));
  }
  template <class T>
  T getByName(const char* name) const {
    verify();
    OptExpr expr = Expr(_getExprByName(name));
    if (!expr)
      throw NoExpressionWithSuchName();
    Opt<T> result = expr->downcast<T>();
    if (!result)
      throw InvalidType();
    return *result;
  }
  template <class T, class Allocator>
  T getByName(const std::basic_string<char, std::char_traits<char>, Allocator>& name) const {
    return getByName(name.data());
  }
  template <class T, typename... Ts>
  T getByName(Ts... nameArgs) const {
    OptExpr expr = getExprByName(nameArgs...);
    if (!expr)
      throw NoExpressionWithSuchName();
    Opt<T> result = expr->downcast<T>();
    if (!result)
      throw InvalidType();
    return *result;
  }

  
  
  
  
  
  
  
  
  void dispose(Expr expr) {
    verify();
    verify(expr);
    _dispose(expr._impl);
  }

  FloatExpr exp(FloatExpr operand1, FloatExpr operand2) {
    verify();
    void* pOperand1 = fromArg(operand1);
    void* pOperand2 = fromArg(operand2);
    return _exp1(pOperand1, pOperand2);
  }

  FloatExpr exp(FloatExpr operand1, IlcFloat operand2) {
    verify();
    void* pOperand1 = fromArg(operand1);
    return _exp2(pOperand1, operand2);
  }

  FloatExpr exp(IlcFloat operand1, FloatExpr operand2) {
    verify();
    void* pOperand2 = fromArg(operand2);
    return _exp3(operand1, pOperand2);
  }

  BoolExpr ifThen(BoolExpr operand1, BoolExpr operand2) {
    verify();
    void* pOperand1 = fromArg(operand1);
    void* pOperand2 = fromArg(operand2);
    return _ifThen1(pOperand1, pOperand2);
  }

  BoolExpr conjunction(BoolExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _conjunction1(pA);
  }

  BoolExpr disjunction(BoolExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _disjunction1(pA);
  }

  IntExpr abs(IntExpr x) {
    verify();
    void* pX = fromArg(x);
    return _abs1(pX);
  }

  IntExpr square(IntExpr x) {
    verify();
    void* pX = fromArg(x);
    return _square1(pX);
  }

  IntExpr min(IntExpr x, IntExpr y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _min1(pX, pY);
  }

  IntExpr min(IntExpr x, IlcInt y) {
    verify();
    void* pX = fromArg(x);
    return _min2(pX, y);
  }

  IntExpr min(IlcInt x, IntExpr y) {
    verify();
    void* pY = fromArg(y);
    return _min3(x, pY);
  }

  IntExpr max(IntExpr x, IntExpr y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _max1(pX, pY);
  }

  IntExpr max(IntExpr x, IlcInt y) {
    verify();
    void* pX = fromArg(x);
    return _max2(pX, y);
  }

  IntExpr max(IlcInt x, IntExpr y) {
    verify();
    void* pY = fromArg(y);
    return _max3(x, pY);
  }

  IntExpr min(IntExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _min4(pA);
  }

  IntExpr max(IntExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _max4(pA);
  }

  IntExpr sum(IntExprArrayArg x) {
    verify();
    void* pX = fromArg(x);
    return _sum1(pX);
  }

  IntExpr scalProd(IntArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd1(pX, pY);
  }

  IntExpr scalProd(IntExprArrayArg x, IntArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd2(pX, pY);
  }

  IntExpr scalProd(IntExprArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd3(pX, pY);
  }

  IntExpr count(IntExprArrayArg exprs, IlcInt value) {
    verify();
    void* pExprs = fromArg(exprs);
    return _count1(pExprs, value);
  }

  IntExpr element(IntArrayArg array, IntExpr subscript) {
    verify();
    void* pArray = fromArg(array);
    void* pSubscript = fromArg(subscript);
    return _element1(pArray, pSubscript);
  }

  IntExpr element(IntExprArrayArg array, IntExpr subscript) {
    verify();
    void* pArray = fromArg(array);
    void* pSubscript = fromArg(subscript);
    return _element2(pArray, pSubscript);
  }

  IntExpr element(IntExprArrayArg array, IlcInt subscript) {
    verify();
    void* pArray = fromArg(array);
    return _element3(pArray, subscript);
  }

  IntExpr countDifferent(IntExprArrayArg array) {
    verify();
    void* pArray = fromArg(array);
    return _countDifferent1(pArray);
  }

  IntExpr floatToInt(FloatExpr x) {
    verify();
    void* pX = fromArg(x);
    return _floatToInt1(pX);
  }

  FloatExpr abs(FloatExpr x) {
    verify();
    void* pX = fromArg(x);
    return _abs2(pX);
  }

  FloatExpr exponent(FloatExpr x) {
    verify();
    void* pX = fromArg(x);
    return _exponent1(pX);
  }

  FloatExpr log(FloatExpr x) {
    verify();
    void* pX = fromArg(x);
    return _log1(pX);
  }

  FloatExpr square(FloatExpr x) {
    verify();
    void* pX = fromArg(x);
    return _square2(pX);
  }

  FloatExpr min(FloatExpr x, FloatExpr y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _min5(pX, pY);
  }

  FloatExpr min(FloatExpr x, IlcFloat y) {
    verify();
    void* pX = fromArg(x);
    return _min6(pX, y);
  }

  FloatExpr min(IlcFloat x, FloatExpr y) {
    verify();
    void* pY = fromArg(y);
    return _min7(x, pY);
  }

  FloatExpr max(FloatExpr x, FloatExpr y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _max5(pX, pY);
  }

  FloatExpr max(FloatExpr x, IlcFloat y) {
    verify();
    void* pX = fromArg(x);
    return _max6(pX, y);
  }

  FloatExpr max(IlcFloat x, FloatExpr y) {
    verify();
    void* pY = fromArg(y);
    return _max7(x, pY);
  }

  FloatExpr min(FloatExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _min8(pA);
  }

  FloatExpr max(FloatExprArrayArg a) {
    verify();
    void* pA = fromArg(a);
    return _max8(pA);
  }

  FloatExpr sum(FloatExprArrayArg x) {
    verify();
    void* pX = fromArg(x);
    return _sum2(pX);
  }

  FloatExpr scalProd(FloatArrayArg x, FloatExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd4(pX, pY);
  }

  FloatExpr scalProd(FloatArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd5(pX, pY);
  }

  FloatExpr scalProd(FloatExprArrayArg x, FloatArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd6(pX, pY);
  }

  FloatExpr scalProd(IntExprArrayArg x, FloatArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd7(pX, pY);
  }

  FloatExpr scalProd(FloatExprArrayArg x, FloatExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd8(pX, pY);
  }

  FloatExpr scalProd(FloatExprArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd9(pX, pY);
  }

  FloatExpr scalProd(IntExprArrayArg x, FloatExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _scalProd10(pX, pY);
  }

  FloatExpr element(FloatArrayArg array, IntExpr subscript) {
    verify();
    void* pArray = fromArg(array);
    void* pSubscript = fromArg(subscript);
    return _element4(pArray, pSubscript);
  }

  FloatExpr standardDeviation(IntExprArrayArg x, IlcFloat meanLB = -IlcInfinity, IlcFloat meanUB = IlcInfinity) {
    verify();
    void* pX = fromArg(x);
    return _standardDeviation1(pX, meanLB, meanUB);
  }

  FloatExpr slopePiecewiseLinear(FloatExpr x, FloatArrayArg points, FloatArrayArg slopes, IlcFloat refX, IlcFloat refY) {
    verify();
    void* pX = fromArg(x);
    void* pPoints = fromArg(points);
    void* pSlopes = fromArg(slopes);
    return _slopePiecewiseLinear1(pX, pPoints, pSlopes, refX, refY);
  }

  FloatExpr coordinatePiecewiseLinear(FloatExpr x, IlcFloat firstSlope, FloatArrayArg points, FloatArrayArg values, IlcFloat lastSlope) {
    verify();
    void* pX = fromArg(x);
    void* pPoints = fromArg(points);
    void* pValues = fromArg(values);
    return _coordinatePiecewiseLinear1(pX, firstSlope, pPoints, pValues, lastSlope);
  }

  BoolExpr allowedAssignments(IntExpr expr, IntArrayArg values) {
    verify();
    void* pExpr = fromArg(expr);
    void* pValues = fromArg(values);
    return _allowedAssignments1(pExpr, pValues);
  }

  BoolExpr forbiddenAssignments(IntExpr expr, IntArrayArg values) {
    verify();
    void* pExpr = fromArg(expr);
    void* pValues = fromArg(values);
    return _forbiddenAssignments1(pExpr, pValues);
  }

  BoolExpr allowedAssignments(IntExprArrayArg exprs, TupleSetArg tuples) {
    verify();
    void* pExprs = fromArg(exprs);
    void* pTuples = fromArg(tuples);
    return _allowedAssignments2(pExprs, pTuples);
  }

  BoolExpr forbiddenAssignments(IntExprArrayArg exprs, TupleSetArg tuples) {
    verify();
    void* pExprs = fromArg(exprs);
    void* pTuples = fromArg(tuples);
    return _forbiddenAssignments2(pExprs, pTuples);
  }

  Constraint inverse(IntExprArrayArg f, IntExprArrayArg invf) {
    verify();
    void* pF = fromArg(f);
    void* pInvf = fromArg(invf);
    return _inverse1(pF, pInvf);
  }

  BoolExpr allDiff(IntExprArrayArg x) {
    verify();
    void* pX = fromArg(x);
    return _allDiff1(pX);
  }

  BoolExpr allMinDistance(IntExprArrayArg exprs, IlcInt distance) {
    verify();
    void* pExprs = fromArg(exprs);
    return _allMinDistance1(pExprs, distance);
  }

  Constraint distribute(IntExprArrayArg counts, IntArrayArg values, IntExprArrayArg exprs) {
    verify();
    void* pCounts = fromArg(counts);
    void* pValues = fromArg(values);
    void* pExprs = fromArg(exprs);
    return _distribute1(pCounts, pValues, pExprs);
  }

  Constraint distribute(IntExprArrayArg counts, IntExprArrayArg exprs) {
    verify();
    void* pCounts = fromArg(counts);
    void* pExprs = fromArg(exprs);
    return _distribute2(pCounts, pExprs);
  }

  Constraint pack(IntExprArrayArg load, IntExprArrayArg where, IntArrayArg size, IntExpr used) {
    verify();
    void* pLoad = fromArg(load);
    void* pWhere = fromArg(where);
    void* pSize = fromArg(size);
    void* pUsed = fromArg(used);
    return _pack1(pLoad, pWhere, pSize, pUsed);
  }

  Constraint pack(IntExprArrayArg load, IntExprArrayArg where, IntArrayArg size, IlcInt used = 0) {
    verify();
    void* pLoad = fromArg(load);
    void* pWhere = fromArg(where);
    void* pSize = fromArg(size);
    return _pack2(pLoad, pWhere, pSize, used);
  }

  BoolExpr lexicographic(IntExprArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _lexicographic1(pX, pY);
  }

  BoolExpr strictLexicographic(IntExprArrayArg x, IntExprArrayArg y) {
    verify();
    void* pX = fromArg(x);
    void* pY = fromArg(y);
    return _strictLexicographic1(pX, pY);
  }

  Constraint sequence(IlcInt min, IlcInt max, IlcInt width, IntExprArrayArg exprs, IntArrayArg values, IntExprArrayArg cards) {
    verify();
    void* pExprs = fromArg(exprs);
    void* pValues = fromArg(values);
    void* pCards = fromArg(cards);
    return _sequence1(min, max, width, pExprs, pValues, pCards);
  }

  Constraint abstraction(IntExprArrayArg y, IntExprArrayArg x, IntArrayArg values, IlcInt abstractValue) {
    verify();
    void* pY = fromArg(y);
    void* pX = fromArg(x);
    void* pValues = fromArg(values);
    return _abstraction1(pY, pX, pValues, abstractValue);
  }

  Constraint boolAbstraction(IntExprArrayArg y, IntExprArrayArg x, IntArrayArg values) {
    verify();
    void* pY = fromArg(y);
    void* pX = fromArg(x);
    void* pValues = fromArg(values);
    return _boolAbstraction1(pY, pX, pValues);
  }

  Constraint spread(IntExprArrayArg foo1, FloatExpr foo2, FloatExpr foo3) {
    verify();
    void* pFoo1 = fromArg(foo1);
    void* pFoo2 = fromArg(foo2);
    void* pFoo3 = fromArg(foo3);
    return _spread1(pFoo1, pFoo2, pFoo3);
  }

  Constraint spread(IntExprArrayArg foo1, FloatExpr foo2, IlcFloat foo3) {
    verify();
    void* pFoo1 = fromArg(foo1);
    void* pFoo2 = fromArg(foo2);
    return _spread2(pFoo1, pFoo2, foo3);
  }

  Constraint spread(IntExprArrayArg foo1, IlcFloat foo2, FloatExpr foo3) {
    verify();
    void* pFoo1 = fromArg(foo1);
    void* pFoo3 = fromArg(foo3);
    return _spread3(pFoo1, foo2, pFoo3);
  }

  Constraint spread(IntExprArrayArg foo1, IlcFloat foo2, IlcFloat foo3) {
    verify();
    void* pFoo1 = fromArg(foo1);
    return _spread4(pFoo1, foo2, foo3);
  }

  Constraint equalOrEscape(IntExpr foo1, IntExpr foo2, IlcInt escapeValue) {
    verify();
    void* pFoo1 = fromArg(foo1);
    void* pFoo2 = fromArg(foo2);
    return _equalOrEscape1(pFoo1, pFoo2, escapeValue);
  }

  Constraint equalOrEscape(IntExpr foo1, IlcInt foo2, IlcInt escapeValue) {
    verify();
    void* pFoo1 = fromArg(foo1);
    return _equalOrEscape2(pFoo1, foo2, escapeValue);
  }

  Constraint equalOrEscape(IlcInt foo1, IntExpr foo2, IlcInt escapeValue) {
    verify();
    void* pFoo2 = fromArg(foo2);
    return _equalOrEscape3(foo1, pFoo2, escapeValue);
  }

  Constraint strong(IntExprArrayArg x) {
    verify();
    void* pX = fromArg(x);
    return _strong1(pX);
  }

  BoolExpr range(FloatExpr x, IlcFloat lB, IlcFloat uB) {
    verify();
    void* pX = fromArg(x);
    return _range1(pX, lB, uB);
  }

  Constraint subCircuit(IntExprArrayArg array) {
    verify();
    void* pArray = fromArg(array);
    return _subCircuit1(pArray);
  }

  Objective minimize(FloatExpr expr) {
    verify();
    void* pExpr = fromArg(expr);
    return _minimize1(pExpr);
  }

  Objective maximize(FloatExpr expr) {
    verify();
    void* pExpr = fromArg(expr);
    return _maximize1(pExpr);
  }

  Objective minimizeStaticLex(FloatExprArrayArg exprs) {
    verify();
    void* pExprs = fromArg(exprs);
    return _minimizeStaticLex1(pExprs);
  }

  Objective minimizeStaticLex(IntExprArrayArg exprs) {
    verify();
    void* pExprs = fromArg(exprs);
    return _minimizeStaticLex2(pExprs);
  }

  Objective maximizeStaticLex(FloatExprArrayArg exprs) {
    verify();
    void* pExprs = fromArg(exprs);
    return _maximizeStaticLex1(pExprs);
  }

  Objective maximizeStaticLex(IntExprArrayArg exprs) {
    verify();
    void* pExprs = fromArg(exprs);
    return _maximizeStaticLex2(pExprs);
  }

  Constraint exoticObject() {
    verify();
    return _exoticObject1();
  }

  BoolExpr presenceOf(IntervalVar interval) {
    verify();
    void* pInterval = fromArg(interval);
    return _presenceOf1(pInterval);
  }

  IntExpr startOf(IntervalVar interval, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    return _startOf1(pInterval, absentValue);
  }

  IntExpr endOf(IntervalVar interval, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    return _endOf1(pInterval, absentValue);
  }

  IntExpr sizeOf(IntervalVar interval, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    return _sizeOf1(pInterval, absentValue);
  }

  IntExpr lengthOf(IntervalVar interval, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    return _lengthOf1(pInterval, absentValue);
  }

  IntExpr overlapLength(IntervalVar interval1, IntervalVar interval2, IlcInt absentValue = 0) {
    verify();
    void* pInterval1 = fromArg(interval1);
    void* pInterval2 = fromArg(interval2);
    return _overlapLength1(pInterval1, pInterval2, absentValue);
  }

  IntExpr overlapLength(IntervalVar interval, IlcInt start, IlcInt end, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    return _overlapLength2(pInterval, start, end, absentValue);
  }

  FloatExpr startEval(IntervalVar interval, SegmentedFunction function, IlcFloat absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _startEval1(pInterval, pFunction, absentValue);
  }

  FloatExpr endEval(IntervalVar interval, SegmentedFunction function, IlcFloat absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _endEval1(pInterval, pFunction, absentValue);
  }

  FloatExpr sizeEval(IntervalVar interval, SegmentedFunction function, IlcFloat absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _sizeEval1(pInterval, pFunction, absentValue);
  }

  FloatExpr lengthEval(IntervalVar interval, SegmentedFunction function, IlcFloat absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _lengthEval1(pInterval, pFunction, absentValue);
  }

  Constraint alternative(IntervalVar interval, IntervalVarArrayArg array, IntExpr cardinality) {
    verify();
    void* pInterval = fromArg(interval);
    void* pArray = fromArg(array);
    void* pCardinality = fromArg(cardinality);
    return _alternative1(pInterval, pArray, pCardinality);
  }

  Constraint alternative(IntervalVar interval, IntervalVarArrayArg array, IlcInt cardinality = 1) {
    verify();
    void* pInterval = fromArg(interval);
    void* pArray = fromArg(array);
    return _alternative2(pInterval, pArray, cardinality);
  }

  Constraint span(IntervalVar interval, IntervalVarArrayArg array) {
    verify();
    void* pInterval = fromArg(interval);
    void* pArray = fromArg(array);
    return _span1(pInterval, pArray);
  }

  Constraint synchronize(IntervalVar interval, IntervalVarArrayArg array) {
    verify();
    void* pInterval = fromArg(interval);
    void* pArray = fromArg(array);
    return _synchronize1(pInterval, pArray);
  }

  Constraint endBeforeEnd(IntervalVar predecessor, IntervalVar successor, IntExpr minDelay) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    void* pMinDelay = fromArg(minDelay);
    return _endBeforeEnd1(pPredecessor, pSuccessor, pMinDelay);
  }

  Constraint endBeforeEnd(IntervalVar predecessor, IntervalVar successor, IlcInt minDelay = 0) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    return _endBeforeEnd2(pPredecessor, pSuccessor, minDelay);
  }

  Constraint endBeforeStart(IntervalVar predecessor, IntervalVar successor, IntExpr minDelay) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    void* pMinDelay = fromArg(minDelay);
    return _endBeforeStart1(pPredecessor, pSuccessor, pMinDelay);
  }

  Constraint endBeforeStart(IntervalVar predecessor, IntervalVar successor, IlcInt minDelay = 0) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    return _endBeforeStart2(pPredecessor, pSuccessor, minDelay);
  }

  Constraint endAtEnd(IntervalVar a, IntervalVar b, IntExpr delay) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    void* pDelay = fromArg(delay);
    return _endAtEnd1(pA, pB, pDelay);
  }

  Constraint endAtEnd(IntervalVar a, IntervalVar b, IlcInt delay = 0) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    return _endAtEnd2(pA, pB, delay);
  }

  Constraint endAtStart(IntervalVar a, IntervalVar b, IntExpr delay) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    void* pDelay = fromArg(delay);
    return _endAtStart1(pA, pB, pDelay);
  }

  Constraint endAtStart(IntervalVar a, IntervalVar b, IlcInt delay = 0) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    return _endAtStart2(pA, pB, delay);
  }

  Constraint startBeforeEnd(IntervalVar predecessor, IntervalVar successor, IntExpr minDelay) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    void* pMinDelay = fromArg(minDelay);
    return _startBeforeEnd1(pPredecessor, pSuccessor, pMinDelay);
  }

  Constraint startBeforeEnd(IntervalVar predecessor, IntervalVar successor, IlcInt minDelay = 0) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    return _startBeforeEnd2(pPredecessor, pSuccessor, minDelay);
  }

  Constraint startBeforeStart(IntervalVar predecessor, IntervalVar successor, IntExpr minDelay) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    void* pMinDelay = fromArg(minDelay);
    return _startBeforeStart1(pPredecessor, pSuccessor, pMinDelay);
  }

  Constraint startBeforeStart(IntervalVar predecessor, IntervalVar successor, IlcInt minDelay = 0) {
    verify();
    void* pPredecessor = fromArg(predecessor);
    void* pSuccessor = fromArg(successor);
    return _startBeforeStart2(pPredecessor, pSuccessor, minDelay);
  }

  Constraint startAtEnd(IntervalVar a, IntervalVar b, IntExpr delay) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    void* pDelay = fromArg(delay);
    return _startAtEnd1(pA, pB, pDelay);
  }

  Constraint startAtEnd(IntervalVar a, IntervalVar b, IlcInt delay = 0) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    return _startAtEnd2(pA, pB, delay);
  }

  Constraint startAtStart(IntervalVar a, IntervalVar b, IntExpr delay) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    void* pDelay = fromArg(delay);
    return _startAtStart1(pA, pB, pDelay);
  }

  Constraint startAtStart(IntervalVar a, IntervalVar b, IlcInt delay = 0) {
    verify();
    void* pA = fromArg(a);
    void* pB = fromArg(b);
    return _startAtStart2(pA, pB, delay);
  }

  Constraint forbidStart(IntervalVar interval, StepFunction function) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _forbidStart1(pInterval, pFunction);
  }

  Constraint forbidEnd(IntervalVar interval, StepFunction function) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _forbidEnd1(pInterval, pFunction);
  }

  Constraint forbidExtent(IntervalVar interval, StepFunction function) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _forbidExtent1(pInterval, pFunction);
  }

  Constraint isomorphism(IntervalVarArrayArg array1, IntervalVarArrayArg array2, IntExprArrayArg map, IlcInt absentValue = 0) {
    verify();
    void* pArray1 = fromArg(array1);
    void* pArray2 = fromArg(array2);
    void* pMap = fromArg(map);
    return _isomorphism1(pArray1, pArray2, pMap, absentValue);
  }

  SequenceVar sequenceVar(IntervalVarArrayArg intervals, IntArrayArg types) {
    verify();
    void* pIntervals = fromArg(intervals);
    void* pTypes = fromArg(types);
    return _sequenceVar1(pIntervals, pTypes);
  }

  IntExpr typeOfNext(SequenceVar sequence, IntervalVar interval, IlcInt lastValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _typeOfNext1(pSequence, pInterval, lastValue, absentValue);
  }

  IntExpr startOfNext(SequenceVar sequence, IntervalVar interval, IlcInt lastValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _startOfNext1(pSequence, pInterval, lastValue, absentValue);
  }

  IntExpr endOfNext(SequenceVar sequence, IntervalVar interval, IlcInt lastValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _endOfNext1(pSequence, pInterval, lastValue, absentValue);
  }

  IntExpr sizeOfNext(SequenceVar sequence, IntervalVar interval, IlcInt lastValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _sizeOfNext1(pSequence, pInterval, lastValue, absentValue);
  }

  IntExpr lengthOfNext(SequenceVar sequence, IntervalVar interval, IlcInt lastValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _lengthOfNext1(pSequence, pInterval, lastValue, absentValue);
  }

  IntExpr typeOfPrev(SequenceVar sequence, IntervalVar interval, IlcInt firstValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _typeOfPrev1(pSequence, pInterval, firstValue, absentValue);
  }

  IntExpr startOfPrev(SequenceVar sequence, IntervalVar interval, IlcInt firstValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _startOfPrev1(pSequence, pInterval, firstValue, absentValue);
  }

  IntExpr endOfPrev(SequenceVar sequence, IntervalVar interval, IlcInt firstValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _endOfPrev1(pSequence, pInterval, firstValue, absentValue);
  }

  IntExpr sizeOfPrev(SequenceVar sequence, IntervalVar interval, IlcInt firstValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _sizeOfPrev1(pSequence, pInterval, firstValue, absentValue);
  }

  IntExpr lengthOfPrev(SequenceVar sequence, IntervalVar interval, IlcInt firstValue = 0, IlcInt absentValue = 0) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _lengthOfPrev1(pSequence, pInterval, firstValue, absentValue);
  }

  Constraint noOverlap(SequenceVar sequence, TransitionMatrix distanceMatrix, bool isDirect = false) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pDistanceMatrix = fromArg(distanceMatrix);
    return _noOverlap1(pSequence, pDistanceMatrix, isDirect);
  }

  Constraint noOverlap(IntervalVarArrayArg intervals) {
    verify();
    void* pIntervals = fromArg(intervals);
    return _noOverlap2(pIntervals);
  }

  Constraint first(SequenceVar sequence, IntervalVar interval) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _first1(pSequence, pInterval);
  }

  Constraint last(SequenceVar sequence, IntervalVar interval) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval = fromArg(interval);
    return _last1(pSequence, pInterval);
  }

  Constraint previous(SequenceVar sequence, IntervalVar interval1, IntervalVar interval2) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval1 = fromArg(interval1);
    void* pInterval2 = fromArg(interval2);
    return _previous1(pSequence, pInterval1, pInterval2);
  }

  Constraint before(SequenceVar sequence, IntervalVar interval1, IntervalVar interval2) {
    verify();
    void* pSequence = fromArg(sequence);
    void* pInterval1 = fromArg(interval1);
    void* pInterval2 = fromArg(interval2);
    return _before1(pSequence, pInterval1, pInterval2);
  }

  Constraint sameSequence(SequenceVar seq1, SequenceVar seq2) {
    verify();
    void* pSeq1 = fromArg(seq1);
    void* pSeq2 = fromArg(seq2);
    return _sameSequence1(pSeq1, pSeq2);
  }

  Constraint sameSequence(SequenceVar seq1, SequenceVar seq2, IntervalVarArrayArg array1, IntervalVarArrayArg array2) {
    verify();
    void* pSeq1 = fromArg(seq1);
    void* pSeq2 = fromArg(seq2);
    void* pArray1 = fromArg(array1);
    void* pArray2 = fromArg(array2);
    return _sameSequence2(pSeq1, pSeq2, pArray1, pArray2);
  }

  Constraint sameCommonSubsequence(SequenceVar seq1, SequenceVar seq2) {
    verify();
    void* pSeq1 = fromArg(seq1);
    void* pSeq2 = fromArg(seq2);
    return _sameCommonSubsequence1(pSeq1, pSeq2);
  }

  Constraint sameCommonSubsequence(SequenceVar seq1, SequenceVar seq2, IntervalVarArrayArg array1, IntervalVarArrayArg array2) {
    verify();
    void* pSeq1 = fromArg(seq1);
    void* pSeq2 = fromArg(seq2);
    void* pArray1 = fromArg(array1);
    void* pArray2 = fromArg(array2);
    return _sameCommonSubsequence2(pSeq1, pSeq2, pArray1, pArray2);
  }

  CumulAtom pulse(IlcInt start, IlcInt end, IlcInt height) {
    verify();
    return _pulse1(start, end, height);
  }

  CumulAtom stepAt(IlcInt time, IlcInt height) {
    verify();
    return _stepAt1(time, height);
  }

  CumulAtom pulse(IntervalVar interval, IlcInt heightMin) {
    verify();
    void* pInterval = fromArg(interval);
    return _pulse2(pInterval, heightMin);
  }

  CumulAtom pulse(IntervalVar interval, IlcInt heightMin, IlcInt heightMax) {
    verify();
    void* pInterval = fromArg(interval);
    return _pulse3(pInterval, heightMin, heightMax);
  }

  CumulAtom stepAtStart(IntervalVar interval, IlcInt heightMin) {
    verify();
    void* pInterval = fromArg(interval);
    return _stepAtStart1(pInterval, heightMin);
  }

  CumulAtom stepAtStart(IntervalVar interval, IlcInt heightMin, IlcInt heightMax) {
    verify();
    void* pInterval = fromArg(interval);
    return _stepAtStart2(pInterval, heightMin, heightMax);
  }

  CumulAtom stepAtEnd(IntervalVar interval, IlcInt heightMin) {
    verify();
    void* pInterval = fromArg(interval);
    return _stepAtEnd1(pInterval, heightMin);
  }

  CumulAtom stepAtEnd(IntervalVar interval, IlcInt heightMin, IlcInt heightMax) {
    verify();
    void* pInterval = fromArg(interval);
    return _stepAtEnd2(pInterval, heightMin, heightMax);
  }

  CumulExpr sum(CumulExprArrayArg x) {
    verify();
    void* pX = fromArg(x);
    return _sum3(pX);
  }

  IntExpr heightAtStart(IntervalVar interval, CumulExpr function, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _heightAtStart1(pInterval, pFunction, absentValue);
  }

  IntExpr heightAtEnd(IntervalVar interval, CumulExpr function, IlcInt absentValue = 0) {
    verify();
    void* pInterval = fromArg(interval);
    void* pFunction = fromArg(function);
    return _heightAtEnd1(pInterval, pFunction, absentValue);
  }

  Constraint alwaysIn(CumulExpr function, IntervalVar interval, IlcInt min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    void* pInterval = fromArg(interval);
    return _alwaysIn1(pFunction, pInterval, min, max);
  }

  Constraint alwaysIn(CumulExpr function, IlcInt start, IlcInt end, IlcInt min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    return _alwaysIn2(pFunction, start, end, min, max);
  }

  Constraint cumulRange(CumulExpr function, IntExpr min, IntExpr max) {
    verify();
    void* pFunction = fromArg(function);
    void* pMin = fromArg(min);
    void* pMax = fromArg(max);
    return _cumulRange1(pFunction, pMin, pMax);
  }

  Constraint cumulRange(CumulExpr function, IntExpr min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    void* pMin = fromArg(min);
    return _cumulRange2(pFunction, pMin, max);
  }

  Constraint cumulRange(CumulExpr function, IlcInt min, IntExpr max) {
    verify();
    void* pFunction = fromArg(function);
    void* pMax = fromArg(max);
    return _cumulRange3(pFunction, min, pMax);
  }

  Constraint cumulRange(CumulExpr function, IlcInt min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    return _cumulRange4(pFunction, min, max);
  }

  StateFunction stateFunction(TransitionMatrix distanceMatrix) {
    verify();
    void* pDistanceMatrix = fromArg(distanceMatrix);
    return _stateFunction1(pDistanceMatrix);
  }

  Constraint alwaysNoState(StateFunction function, IntervalVar interval) {
    verify();
    void* pFunction = fromArg(function);
    void* pInterval = fromArg(interval);
    return _alwaysNoState1(pFunction, pInterval);
  }

  Constraint alwaysNoState(StateFunction function, IlcInt start, IlcInt end) {
    verify();
    void* pFunction = fromArg(function);
    return _alwaysNoState2(pFunction, start, end);
  }

  Constraint alwaysIn(StateFunction function, IntervalVar interval, IlcInt min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    void* pInterval = fromArg(interval);
    return _alwaysIn3(pFunction, pInterval, min, max);
  }

  Constraint alwaysIn(StateFunction function, IlcInt start, IlcInt end, IlcInt min, IlcInt max) {
    verify();
    void* pFunction = fromArg(function);
    return _alwaysIn4(pFunction, start, end, min, max);
  }

  Constraint alwaysConstant(StateFunction function, IntervalVar interval, bool isStartAligned = false, bool isEndAligned = false) {
    verify();
    void* pFunction = fromArg(function);
    void* pInterval = fromArg(interval);
    return _alwaysConstant1(pFunction, pInterval, isStartAligned, isEndAligned);
  }

  Constraint alwaysConstant(StateFunction function, IlcInt start, IlcInt end, bool isStartAligned = false, bool isEndAligned = false) {
    verify();
    void* pFunction = fromArg(function);
    return _alwaysConstant2(pFunction, start, end, isStartAligned, isEndAligned);
  }

  Constraint alwaysEqual(StateFunction function, IntervalVar interval, IlcInt min, bool isStartAligned = false, bool isEndAligned = false) {
    verify();
    void* pFunction = fromArg(function);
    void* pInterval = fromArg(interval);
    return _alwaysEqual1(pFunction, pInterval, min, isStartAligned, isEndAligned);
  }

  Constraint alwaysEqual(StateFunction function, IlcInt start, IlcInt end, IlcInt min, bool isStartAligned = false, bool isEndAligned = false) {
    verify();
    void* pFunction = fromArg(function);
    return _alwaysEqual2(pFunction, start, end, min, isStartAligned, isEndAligned);
  }

  IntVarEval domainSize() {
    verify();
    return _domainSize1();
  }

  IntVarEval domainMax() {
    verify();
    return _domainMax1();
  }

  IntVarEval domainMin() {
    verify();
    return _domainMin1();
  }

  IntVarEval varImpact() {
    verify();
    return _varImpact1();
  }

  IntVarEval varSuccessRate() {
    verify();
    return _varSuccessRate1();
  }

  IntVarEval impactOfLastBranch() {
    verify();
    return _impactOfLastBranch1();
  }

  IntVarEval varIndex(IntExprArrayArg x, IlcFloat defaultEval = -1) {
    verify();
    void* pX = fromArg(x);
    return _varIndex1(pX, defaultEval);
  }

  IntVarEval varLocalImpact(IlcInt effort = -1) {
    verify();
    return _varLocalImpact1(effort);
  }

  IntVarEval explicitVarEval(IntExprArrayArg x, FloatArrayArg evalArray, IlcFloat defaultEval = 0) {
    verify();
    void* pX = fromArg(x);
    void* pEvalArray = fromArg(evalArray);
    return _explicitVarEval1(pX, pEvalArray, defaultEval);
  }

  IntValueEval value() {
    verify();
    return _value1();
  }

  IntValueEval valueImpact() {
    verify();
    return _valueImpact1();
  }

  IntValueEval valueSuccessRate() {
    verify();
    return _valueSuccessRate1();
  }

  IntValueEval valueIndex(IntArrayArg valueEval, IlcFloat defaultEval = -1) {
    verify();
    void* pValueEval = fromArg(valueEval);
    return _valueIndex1(pValueEval, defaultEval);
  }

  IntValueEval explicitValueEval(IntArrayArg valueArray, FloatArrayArg evalArray, IlcFloat defaultEval = 0) {
    verify();
    void* pValueArray = fromArg(valueArray);
    void* pEvalArray = fromArg(evalArray);
    return _explicitValueEval1(pValueArray, pEvalArray, defaultEval);
  }

  IntVarSelector selectSmallest(IlcFloat tolerance, IntVarEval eval) {
    verify();
    void* pEval = fromArg(eval);
    return _selectSmallest1(tolerance, pEval);
  }

  IntVarSelector selectSmallest(IntVarEval eval, IlcFloat tolerance = 0) {
    verify();
    void* pEval = fromArg(eval);
    return _selectSmallest2(pEval, tolerance);
  }

  IntVarSelector selectLargest(IlcFloat tolerance, IntVarEval eval) {
    verify();
    void* pEval = fromArg(eval);
    return _selectLargest1(tolerance, pEval);
  }

  IntVarSelector selectLargest(IntVarEval eval, IlcFloat tolerance = 0) {
    verify();
    void* pEval = fromArg(eval);
    return _selectLargest2(pEval, tolerance);
  }

  IntVarSelector selectRandomVar() {
    verify();
    return _selectRandomVar1();
  }

  IntValueSelector selectSmallest(IlcFloat tolerance, IntValueEval eval) {
    verify();
    void* pEval = fromArg(eval);
    return _selectSmallest3(tolerance, pEval);
  }

  IntValueSelector selectSmallest(IntValueEval eval, IlcFloat tolerance = 0) {
    verify();
    void* pEval = fromArg(eval);
    return _selectSmallest4(pEval, tolerance);
  }

  IntValueSelector selectLargest(IlcFloat tolerance, IntValueEval eval) {
    verify();
    void* pEval = fromArg(eval);
    return _selectLargest3(tolerance, pEval);
  }

  IntValueSelector selectLargest(IntValueEval eval, IlcFloat tolerance = 0) {
    verify();
    void* pEval = fromArg(eval);
    return _selectLargest4(pEval, tolerance);
  }

  IntValueSelector selectRandomValue() {
    verify();
    return _selectRandomValue1();
  }

  SearchPhase searchPhase(IntExprArrayArg variables) {
    verify();
    void* pVariables = fromArg(variables);
    return _searchPhase1(pVariables);
  }

  SearchPhase searchPhase(IntVarChooser variableChooser, IntValueChooser valueChooser) {
    verify();
    void* pVariableChooser = fromArg(variableChooser);
    void* pValueChooser = fromArg(valueChooser);
    return _searchPhase2(pVariableChooser, pValueChooser);
  }

  SearchPhase searchPhase(IntExprArrayArg variables, IntVarChooser variableChooser, IntValueChooser valueChooser) {
    verify();
    void* pVariables = fromArg(variables);
    void* pVariableChooser = fromArg(variableChooser);
    void* pValueChooser = fromArg(valueChooser);
    return _searchPhase3(pVariables, pVariableChooser, pValueChooser);
  }

  SearchPhase searchPhase(IntervalVarArrayArg variables) {
    verify();
    void* pVariables = fromArg(variables);
    return _searchPhase4(pVariables);
  }

  SearchPhase searchPhase(SequenceVarArrayArg variables) {
    verify();
    void* pVariables = fromArg(variables);
    return _searchPhase5(pVariables);
  }

};

template <typename ExprType, class Allocator, class BaseNameType>
void NameItems(std::vector<ExprType, Allocator>& expressions, const BaseNameType& baseName, IlcInt firstIndex) {
  static_assert(std::is_base_of<Expr, ExprType>::value, "Function NameItems<ExprType> can work only for ExprType derived from cpoptimzier::Expr.");
  IlcInt i = firstIndex;
  for (ExprType& expr: expressions) {
    expr.name(baseName, "[", i, "]");
    i++;
  }
}

template <class Allocator = std::allocator<IntVar> >
void SetDomain(std::vector<IntVar, Allocator>& variables, IlcInt domainMin, IlcInt domainMax) {
  for (IntVar& var: variables)
    var.setDomain(domainMin, domainMax);
}

template <class Allocator = std::allocator<IntVar> >
void SetDomain(std::vector<IntVar, Allocator>& variables, IntArrayArg domain) {
  for (IntVar& var: variables)
    var.setDomain(domain);
}

inline IntExpr operator +(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Plus1(operand1.getImpl(), operand2.getImpl());
}

inline IntExpr operator +(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Plus2(operand1.getImpl(), operand2);
}

inline IntExpr operator +(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Plus3(operand1, operand2.getImpl());
}

inline IntExpr operator -(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Minus1(operand1.getImpl(), operand2.getImpl());
}

inline IntExpr operator -(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Minus2(operand1.getImpl(), operand2);
}

inline IntExpr operator -(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Minus3(operand1, operand2.getImpl());
}

inline IntExpr operator -(IntExpr operand) {
  Model::Verify(operand);
  return Model::_UnaryMinus1(operand.getImpl());
}

inline IntExpr operator *(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Times1(operand1.getImpl(), operand2.getImpl());
}

inline IntExpr operator *(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Times2(operand1.getImpl(), operand2);
}

inline IntExpr operator *(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Times3(operand1, operand2.getImpl());
}

inline IntExpr operator /(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_IntDiv1(operand1.getImpl(), operand2.getImpl());
}

inline IntExpr operator /(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_IntDiv2(operand1.getImpl(), operand2);
}

inline IntExpr operator /(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_IntDiv3(operand1, operand2.getImpl());
}

inline IntExpr operator %(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Mod1(operand1.getImpl(), operand2.getImpl());
}

inline IntExpr operator %(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Mod2(operand1.getImpl(), operand2);
}

inline IntExpr operator %(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Mod3(operand1, operand2.getImpl());
}

inline FloatExpr operator +(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Plus4(operand1.getImpl(), operand2.getImpl());
}

inline FloatExpr operator +(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Plus5(operand1.getImpl(), operand2);
}

inline FloatExpr operator +(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Plus6(operand1, operand2.getImpl());
}

inline FloatExpr operator -(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Minus4(operand1.getImpl(), operand2.getImpl());
}

inline FloatExpr operator -(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Minus5(operand1.getImpl(), operand2);
}

inline FloatExpr operator -(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Minus6(operand1, operand2.getImpl());
}

inline FloatExpr operator -(FloatExpr operand) {
  Model::Verify(operand);
  return Model::_UnaryMinus2(operand.getImpl());
}

inline FloatExpr operator *(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Times4(operand1.getImpl(), operand2.getImpl());
}

inline FloatExpr operator *(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Times5(operand1.getImpl(), operand2);
}

inline FloatExpr operator *(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Times6(operand1, operand2.getImpl());
}

inline FloatExpr operator /(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_FloatDiv1(operand1.getImpl(), operand2.getImpl());
}

inline FloatExpr operator /(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_FloatDiv2(operand1.getImpl(), operand2);
}

inline FloatExpr operator /(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_FloatDiv3(operand1, operand2.getImpl());
}

inline BoolExpr operator ==(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Equal1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator ==(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Equal2(operand1.getImpl(), operand2);
}

inline BoolExpr operator ==(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Equal3(operand1, operand2.getImpl());
}

inline BoolExpr operator ==(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Equal4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator ==(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Equal5(operand1.getImpl(), operand2);
}

inline BoolExpr operator ==(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Equal6(operand1, operand2.getImpl());
}

inline BoolExpr operator !=(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Diff1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator !=(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Diff2(operand1.getImpl(), operand2);
}

inline BoolExpr operator !=(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Diff3(operand1, operand2.getImpl());
}

inline BoolExpr operator !=(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Diff4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator !=(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Diff5(operand1.getImpl(), operand2);
}

inline BoolExpr operator !=(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Diff6(operand1, operand2.getImpl());
}

inline BoolExpr operator >=(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_GreaterOrEqual1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator >=(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_GreaterOrEqual2(operand1.getImpl(), operand2);
}

inline BoolExpr operator >=(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_GreaterOrEqual3(operand1, operand2.getImpl());
}

inline BoolExpr operator >=(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_GreaterOrEqual4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator >=(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_GreaterOrEqual5(operand1.getImpl(), operand2);
}

inline BoolExpr operator >=(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_GreaterOrEqual6(operand1, operand2.getImpl());
}

inline BoolExpr operator <=(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_LessOrEqual1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator <=(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_LessOrEqual2(operand1.getImpl(), operand2);
}

inline BoolExpr operator <=(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_LessOrEqual3(operand1, operand2.getImpl());
}

inline BoolExpr operator <=(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_LessOrEqual4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator <=(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_LessOrEqual5(operand1.getImpl(), operand2);
}

inline BoolExpr operator <=(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_LessOrEqual6(operand1, operand2.getImpl());
}

inline BoolExpr operator >(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Greater1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator >(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Greater2(operand1.getImpl(), operand2);
}

inline BoolExpr operator >(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Greater3(operand1, operand2.getImpl());
}

inline BoolExpr operator >(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Greater4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator >(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Greater5(operand1.getImpl(), operand2);
}

inline BoolExpr operator >(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Greater6(operand1, operand2.getImpl());
}

inline BoolExpr operator <(IntExpr operand1, IntExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Less1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator <(IntExpr operand1, IlcInt operand2) {
  Model::Verify(operand1);
  return Model::_Less2(operand1.getImpl(), operand2);
}

inline BoolExpr operator <(IlcInt operand1, IntExpr operand2) {
  Model::Verify(operand2);
  return Model::_Less3(operand1, operand2.getImpl());
}

inline BoolExpr operator <(FloatExpr operand1, FloatExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Less4(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator <(FloatExpr operand1, IlcFloat operand2) {
  Model::Verify(operand1);
  return Model::_Less5(operand1.getImpl(), operand2);
}

inline BoolExpr operator <(IlcFloat operand1, FloatExpr operand2) {
  Model::Verify(operand2);
  return Model::_Less6(operand1, operand2.getImpl());
}

inline BoolExpr operator ||(BoolExpr operand1, BoolExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Or1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator &&(BoolExpr operand1, BoolExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_And1(operand1.getImpl(), operand2.getImpl());
}

inline BoolExpr operator !(BoolExpr operand) {
  Model::Verify(operand);
  return Model::_Not1(operand.getImpl());
}

inline CumulExpr operator +(CumulExpr operand1, CumulExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Plus7(operand1.getImpl(), operand2.getImpl());
}

inline CumulExpr operator -(CumulExpr operand1, CumulExpr operand2) {
  Model::Verify(operand1, operand2);
  return Model::_Minus7(operand1.getImpl(), operand2.getImpl());
}

inline CumulExpr operator -(CumulExpr operand) {
  Model::Verify(operand);
  return Model::_UnaryMinus3(operand.getImpl());
}

inline Constraint operator >=(IlcInt max, CumulExpr function) {
  Model::Verify(function);
  return Model::_GreaterOrEqual7(max, function.getImpl());
}

inline Constraint operator <=(CumulExpr function, IlcInt max) {
  Model::Verify(function);
  return Model::_LessOrEqual7(function.getImpl(), max);
}

inline Constraint operator >=(CumulExpr function, IlcInt min) {
  Model::Verify(function);
  return Model::_GreaterOrEqual8(function.getImpl(), min);
}

inline Constraint operator <=(IlcInt min, CumulExpr function) {
  Model::Verify(function);
  return Model::_LessOrEqual8(min, function.getImpl());
}

inline Constraint operator <=(CumulExpr function, IntExpr max) {
  Model::Verify(function, max);
  return Model::_LessOrEqual9(function.getImpl(), max.getImpl());
}

inline Constraint operator >=(CumulExpr function, IntExpr min) {
  Model::Verify(function, min);
  return Model::_GreaterOrEqual9(function.getImpl(), min.getImpl());
}

inline Constraint operator >=(IntExpr max, CumulExpr function) {
  Model::Verify(max, function);
  return Model::_GreaterOrEqual10(max.getImpl(), function.getImpl());
}

inline Constraint operator <=(IntExpr min, CumulExpr function) {
  Model::Verify(min, function);
  return Model::_LessOrEqual10(min.getImpl(), function.getImpl());
}

class CP: public IloCP {
 private:
  IlcInt _getIntIncumbentValue(const void*) const;
  IlcFloat _getFloatIncumbentValue(const void*) const;
  bool _getBoolIncumbentValue(const void*) const;
  void* _getModel() const;
  bool _isInOurModel(const void*) const;
  void verify() const {
    IlcCPOAssert(getImpl(), "Empty handle (unitialized class).");
  }
  void verify(const Expr expr) const {
    expr.verify();
    IlcCPOAssert(_isInOurModel(expr.getImpl()), "Expression belongs to other Model.");
  }
 public:
  CP(IloEnv env):
    IloCP(env)
  {}
  Model getModel() {
    verify();
    return Model(_getModel());
  }
  const Model getModel() const {
    verify();
    return Model(_getModel());
  }
  IlcInt getIncubmentValue(const IntExpr expr) const {
    verify();
    verify(expr);
    return _getIntIncumbentValue(expr.getImpl());
  }
  IlcFloat getIncumbentValue(const FloatExpr expr) const {
    verify();
    verify(expr);
    return _getFloatIncumbentValue(expr.getImpl());
  }
  bool getIncumbentValue(const BoolExpr expr) const {
    verify();
    verify(expr);
    return _getBoolIncumbentValue(expr.getImpl());
  }
  void hackBeforeSolve();
};

inline Model Expr::getModel() const {
  verify();
  return _getModel();
}

inline IntVar& IntVar::setDomain(IntArrayArg domain) {
  verify();
  if (domain._type == IntArrayArg::Data)
    _setDomain(domain._getSize(), domain._getItems());
  else {
    IntArray arg = domain._getExpr();
    arg.verify();
    verifyModel(arg);
    _setDomain(arg._impl);
  }
  return *this;
}

template <class T> inline Opt<T> Expr::downcast() const {
  verify();
  Type t = _getType();
  bool ok = T::ExprType == t;
  if (!ok) {
    switch (T::ExprType) {
      case CumulExprT:
        ok = (t == CumulAtomT || t == MutableCumulExprT);
        break;
      case FloatExprArrayT:
        ok = (t == IntExprArrayT || t == BoolExprArrayT);
        break;
      case FloatExprT:
        ok = (t == MutableFloatExprT || t == FloatConstantT);
        
      case IntExprT:
        ok = ok || (t == IntExprT || t == IntVarT || t == MutableIntExprT || t == IntConstantT);
        
      case BoolExprT:
      case ConstraintT:
        ok = ok || (t == BoolExprT || t == BoolVarT || t == MutableBoolExprT || t == BoolConstantT);
        break;
      default:
        ;
    }
  }
  if (ok)
    return T(_impl);
  return {};
}

} 

namespace std {
  template<> struct hash<cpoptimizer::Expr> {
    std::size_t operator()(cpoptimizer::Expr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::Expr> {
    bool operator()(const cpoptimizer::Expr &lhs, const cpoptimizer::Expr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::BoolConstant> {
    std::size_t operator()(cpoptimizer::BoolConstant const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::BoolConstant> {
    bool operator()(const cpoptimizer::BoolConstant &lhs, const cpoptimizer::BoolConstant &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::BoolExprArray> {
    std::size_t operator()(cpoptimizer::BoolExprArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::BoolExprArray> {
    bool operator()(const cpoptimizer::BoolExprArray &lhs, const cpoptimizer::BoolExprArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::BoolExpr> {
    std::size_t operator()(cpoptimizer::BoolExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::BoolExpr> {
    bool operator()(const cpoptimizer::BoolExpr &lhs, const cpoptimizer::BoolExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::BoolVar> {
    std::size_t operator()(cpoptimizer::BoolVar const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::BoolVar> {
    bool operator()(const cpoptimizer::BoolVar &lhs, const cpoptimizer::BoolVar &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::Constraint> {
    std::size_t operator()(cpoptimizer::Constraint const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::Constraint> {
    bool operator()(const cpoptimizer::Constraint &lhs, const cpoptimizer::Constraint &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::CumulAtom> {
    std::size_t operator()(cpoptimizer::CumulAtom const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::CumulAtom> {
    bool operator()(const cpoptimizer::CumulAtom &lhs, const cpoptimizer::CumulAtom &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::CumulExprArray> {
    std::size_t operator()(cpoptimizer::CumulExprArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::CumulExprArray> {
    bool operator()(const cpoptimizer::CumulExprArray &lhs, const cpoptimizer::CumulExprArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::CumulExpr> {
    std::size_t operator()(cpoptimizer::CumulExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::CumulExpr> {
    bool operator()(const cpoptimizer::CumulExpr &lhs, const cpoptimizer::CumulExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::FloatArray> {
    std::size_t operator()(cpoptimizer::FloatArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::FloatArray> {
    bool operator()(const cpoptimizer::FloatArray &lhs, const cpoptimizer::FloatArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::FloatConstant> {
    std::size_t operator()(cpoptimizer::FloatConstant const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::FloatConstant> {
    bool operator()(const cpoptimizer::FloatConstant &lhs, const cpoptimizer::FloatConstant &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::FloatExprArray> {
    std::size_t operator()(cpoptimizer::FloatExprArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::FloatExprArray> {
    bool operator()(const cpoptimizer::FloatExprArray &lhs, const cpoptimizer::FloatExprArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::FloatExpr> {
    std::size_t operator()(cpoptimizer::FloatExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::FloatExpr> {
    bool operator()(const cpoptimizer::FloatExpr &lhs, const cpoptimizer::FloatExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntArray> {
    std::size_t operator()(cpoptimizer::IntArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntArray> {
    bool operator()(const cpoptimizer::IntArray &lhs, const cpoptimizer::IntArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntConstant> {
    std::size_t operator()(cpoptimizer::IntConstant const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntConstant> {
    bool operator()(const cpoptimizer::IntConstant &lhs, const cpoptimizer::IntConstant &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntervalVarArray> {
    std::size_t operator()(cpoptimizer::IntervalVarArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntervalVarArray> {
    bool operator()(const cpoptimizer::IntervalVarArray &lhs, const cpoptimizer::IntervalVarArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntervalVar> {
    std::size_t operator()(cpoptimizer::IntervalVar const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntervalVar> {
    bool operator()(const cpoptimizer::IntervalVar &lhs, const cpoptimizer::IntervalVar &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntExprArray> {
    std::size_t operator()(cpoptimizer::IntExprArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntExprArray> {
    bool operator()(const cpoptimizer::IntExprArray &lhs, const cpoptimizer::IntExprArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntExpr> {
    std::size_t operator()(cpoptimizer::IntExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntExpr> {
    bool operator()(const cpoptimizer::IntExpr &lhs, const cpoptimizer::IntExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntValueChooser> {
    std::size_t operator()(cpoptimizer::IntValueChooser const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntValueChooser> {
    bool operator()(const cpoptimizer::IntValueChooser &lhs, const cpoptimizer::IntValueChooser &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntValueEval> {
    std::size_t operator()(cpoptimizer::IntValueEval const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntValueEval> {
    bool operator()(const cpoptimizer::IntValueEval &lhs, const cpoptimizer::IntValueEval &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntValueSelector> {
    std::size_t operator()(cpoptimizer::IntValueSelector const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntValueSelector> {
    bool operator()(const cpoptimizer::IntValueSelector &lhs, const cpoptimizer::IntValueSelector &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntVarChooser> {
    std::size_t operator()(cpoptimizer::IntVarChooser const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntVarChooser> {
    bool operator()(const cpoptimizer::IntVarChooser &lhs, const cpoptimizer::IntVarChooser &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntVarEval> {
    std::size_t operator()(cpoptimizer::IntVarEval const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntVarEval> {
    bool operator()(const cpoptimizer::IntVarEval &lhs, const cpoptimizer::IntVarEval &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntVarSelector> {
    std::size_t operator()(cpoptimizer::IntVarSelector const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntVarSelector> {
    bool operator()(const cpoptimizer::IntVarSelector &lhs, const cpoptimizer::IntVarSelector &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::IntVar> {
    std::size_t operator()(cpoptimizer::IntVar const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::IntVar> {
    bool operator()(const cpoptimizer::IntVar &lhs, const cpoptimizer::IntVar &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::MutableBoolExpr> {
    std::size_t operator()(cpoptimizer::MutableBoolExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::MutableBoolExpr> {
    bool operator()(const cpoptimizer::MutableBoolExpr &lhs, const cpoptimizer::MutableBoolExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::MutableIntExpr> {
    std::size_t operator()(cpoptimizer::MutableIntExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::MutableIntExpr> {
    bool operator()(const cpoptimizer::MutableIntExpr &lhs, const cpoptimizer::MutableIntExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::MutableFloatExpr> {
    std::size_t operator()(cpoptimizer::MutableFloatExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::MutableFloatExpr> {
    bool operator()(const cpoptimizer::MutableFloatExpr &lhs, const cpoptimizer::MutableFloatExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::MutableCumulExpr> {
    std::size_t operator()(cpoptimizer::MutableCumulExpr const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::MutableCumulExpr> {
    bool operator()(const cpoptimizer::MutableCumulExpr &lhs, const cpoptimizer::MutableCumulExpr &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::Objective> {
    std::size_t operator()(cpoptimizer::Objective const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::Objective> {
    bool operator()(const cpoptimizer::Objective &lhs, const cpoptimizer::Objective &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::SearchPhase> {
    std::size_t operator()(cpoptimizer::SearchPhase const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::SearchPhase> {
    bool operator()(const cpoptimizer::SearchPhase &lhs, const cpoptimizer::SearchPhase &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::SegmentedFunction> {
    std::size_t operator()(cpoptimizer::SegmentedFunction const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::SegmentedFunction> {
    bool operator()(const cpoptimizer::SegmentedFunction &lhs, const cpoptimizer::SegmentedFunction &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::SequenceVarArray> {
    std::size_t operator()(cpoptimizer::SequenceVarArray const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::SequenceVarArray> {
    bool operator()(const cpoptimizer::SequenceVarArray &lhs, const cpoptimizer::SequenceVarArray &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::SequenceVar> {
    std::size_t operator()(cpoptimizer::SequenceVar const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::SequenceVar> {
    bool operator()(const cpoptimizer::SequenceVar &lhs, const cpoptimizer::SequenceVar &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::StateFunction> {
    std::size_t operator()(cpoptimizer::StateFunction const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::StateFunction> {
    bool operator()(const cpoptimizer::StateFunction &lhs, const cpoptimizer::StateFunction &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::StepFunction> {
    std::size_t operator()(cpoptimizer::StepFunction const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::StepFunction> {
    bool operator()(const cpoptimizer::StepFunction &lhs, const cpoptimizer::StepFunction &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::TransitionMatrix> {
    std::size_t operator()(cpoptimizer::TransitionMatrix const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::TransitionMatrix> {
    bool operator()(const cpoptimizer::TransitionMatrix &lhs, const cpoptimizer::TransitionMatrix &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
  template<> struct hash<cpoptimizer::TupleSet> {
    std::size_t operator()(cpoptimizer::TupleSet const& expr) const noexcept {
      return std::hash<const void*>{}(expr.getImpl());
    }
  };
  template<> struct equal_to<cpoptimizer::TupleSet> {
    bool operator()(const cpoptimizer::TupleSet &lhs, const cpoptimizer::TupleSet &rhs) const {
      return lhs.getImpl() == rhs.getImpl();
    }
  };
}
