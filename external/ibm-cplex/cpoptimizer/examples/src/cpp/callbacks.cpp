// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/callbacks.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2024. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------

This example demonstrates the use of a callback to deliver real-time
information on the lower and upper bounds of the objective function,
and the gap.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class BoundsCallback : public IloCP::Callback {
private:
  std::ostream& _out;
  IloNum        _lb;
  IloNum        _ub;
  IloNum        _gap;

  // Helper class to restore stream flags cleanly
  class OstreamGuard {
  private:
    std::ostream&           _s;
    std::ios_base::fmtflags _f;
  public:
    OstreamGuard(std::ostream& s) : _s(s), _f(s.flags()) { }
    ~OstreamGuard() { _s.flags(_f); }
  };
public:
  BoundsCallback(std::ostream& out) : _out(out) { init(); }
  void init() {
    _lb  = -IloInfinity;
    _ub  = IloInfinity;
    _gap = IloInfinity;
  }
  void invoke(IloCP cp, IloCP::Callback::Reason reason) {
    OstreamGuard guard(_out);

    // Initialize the local fields at the beginning of search,
    // and print the banner
    if (reason == IloCP::Callback::StartSolve) {
      init();
      _out << "Time\tLB\tUB\tGap" << std::endl;
      _out << "=============================" << std::endl;
    }
    else if (reason == IloCP::Callback::EndSolve) {
      // Finish the line at the end of search
      _out << std::endl;
    }
    else {
      IloBool soln = (reason == IloCP::Callback::Solution);
      IloBool bnd = (reason == IloCP::Callback::ObjBound);
      if (soln || bnd) {
        // Write a newline if a line has already been written
        if (_lb > -IloInfinity && _ub < IloInfinity)
          _out << std::endl;
        if (soln) {
          // Update upper bound and gap (if a bound has been communicated)
          _ub = cp.getObjValue();
          if (_lb > -IloInfinity)
            _gap = cp.getObjGap();
        }
        else if (bnd) {
          // Update lower bound and gap (if a solution has been found)
          _lb = cp.getObjBound();
          if (_ub < IloInfinity)
            _gap = cp.getObjGap();
        }
      }
      if (_lb > -IloInfinity && _ub < IloInfinity) {
        // Write a line when we have both a bound and a solution.
        int p = _out.precision();
        _out << "\r                                                  \r"
             << std::fixed << std::setprecision(1)
             << cp.getInfo(IloCP::SolveTime) << "\t"
             << std::setprecision(0)
             << _lb << "\t" << _ub << "\t"
             << std::setprecision(1)
             << 100 * _gap << "%\t" << std::flush;
        _out.precision(p);
      }
    }
  }
};

class ScopedCallback {
  IloCP            _cp;
  IloCP::Callback& _callback;
public:
  ScopedCallback(IloCP cp, IloCP::Callback& callback)
    : _cp(cp)
    , _callback(callback) {
    _cp.addCallback(&_callback);
  }
  ~ScopedCallback() {
    _cp.removeCallback(&_callback);
  }
  IloBool solve() const {
    return _cp.solve();
  }
};

void SolveWithCallback(IloCP cp) {
  // Add callback, solve, then remove it.
  BoundsCallback        cb(cp.out());
  // make sure the callback is removed if an exception destroys this scope.
  ScopedCallback(cp, cb).solve();
}

int main(int, const char * []) {
  IloEnv env;
  try {
    // Load a model, and solve using a callback.
    IloCP cp(env);
    cp.importModel("../../../examples/data/sched_jobshop.cpo");
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    SolveWithCallback(cp);
    cp.end();
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
