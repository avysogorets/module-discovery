// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/util.h
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
//
#ifndef __CPO_EXAMPLES_UTIL_H

#define __CPO_EXAMPLES_UTIL_H

#include <ilconcert/ilomodel.h>

void NameVars(IloIntVarArray a, const char * base) {
  for (IloInt i = 0; i < a.getSize(); i++) {
    char name[100];
    snprintf(name, sizeof(name), "%s[%ld]", base, (long)i);
    a[i].setName(name);
  }
}

void NameVars(IloIntervalVarArray a, const char * base) {
  for (IloInt i = 0; i < a.getSize(); i++) {
    char name[100];
    snprintf(name, sizeof(name), "%s[%ld]", base, (long)i);
    a[i].setName(name);
  }
}

void NameVars(IloArray<IloIntVarArray> a, const char * base) {
  for (IloInt i = 0; i < a.getSize(); i++) {
    for (IloInt j = 0; j < a[i].getSize(); j++) {
      char name[100];
      snprintf(name,sizeof(name), "%s[%ld][%ld]", base, (long)i, long(j));
      a[i][j].setName(name);
    }
  }
}

void NameVars(IloArray<IloIntervalVarArray> a, const char * base) {
  for (IloInt i = 0; i < a.getSize(); i++) {
    for (IloInt j = 0; j < a[i].getSize(); j++) {
      char name[100];
      snprintf(name, sizeof(name), "%s[%ld][%ld]", base, (long)i, long(j));
      a[i][j].setName(name);
    }
  }
}

// Interval [s, e)
class DisplayInterval {
private:
  IloInt _s;
  IloInt _e;
  void displayTime(std::ostream& out, IloInt t) const {
    if (t == IloIntervalMin)      out << "IntervalMin";
    else if (t == IloIntervalMax) out << "IntervalMax";
    else                          out << t;
  }
public:
  DisplayInterval(IloInt s, IloInt e) : _s(s), _e(e) { }
  virtual void display(std::ostream& out) const {
    out << "[";
    displayTime(out, _s);
    out << ", ";
    displayTime(out, _e);
    out << ")";
  }
};
std::ostream& operator << (std::ostream& out, const DisplayInterval& itv) {
  itv.display(out);
  return out;
}

class DisplayCumulSegment : public DisplayInterval {
public:
  DisplayCumulSegment(IloCP cp, IloCumulFunctionExpr sf, IloInt seg)
    : DisplayInterval(cp.getSegmentStart(sf, seg), cp.getSegmentEnd(sf, seg)) { }
};

class DisplayStateSegment : public DisplayInterval {
public:
  DisplayStateSegment(IloCP cp, IloStateFunction sf, IloInt seg)
    : DisplayInterval(cp.getSegmentStart(sf, seg), cp.getSegmentEnd(sf, seg)) { }
};

#endif
