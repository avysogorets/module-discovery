
// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_truck_drone.cpp
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

The Truck with Drone problem involves the collaboration between a truck and
a drone to deliver customers. The drone can pick up packages from the truck
and deliver them to the customer (but it cannot serve more than one location
at a time) while the truck is serving other customers.
The drone takes off or returns to the truck only at a customer location.
The problem consists in allocating delivery locations to either the truck
or the drone, while minimizing the total time to serve all customers, and
return to the depot.

------------------------------------------------------------ */

#include <ilcp/cp.h>

#define TIME_SCALE 1000000

IloInt IntegerTime(IloNum t) { return IloInt(t * TIME_SCALE); }
IloNum RealTime(IloInt t) { return t / (IloNum)TIME_SCALE; }
IloNumExpr RealTime(IloIntExpr t) { return t / (IloNum)TIME_SCALE; }


class ProblemData {
  IloNumArray _xco;
  IloNumArray _yco;
  IloNum      _truckSpeed;
  IloNum      _droneSpeed;
  IloInt      _numCustomers;

  void ignore(std::istream&in, IloInt n) {
    std::string s;
    for (IloInt i = 0; i < n; i++) in >> s;
  }

public:
  ProblemData(IloEnv env, std::istream& in);

  IloInt getNumCustomers() const { return _numCustomers; }
  IloInt getNumNodes() const { return _numCustomers + 1; }
  double getX(IloInt i) const { return _xco[i]; }
  double getY(IloInt i) const { return _yco[i]; }
  double getDistance(IloInt from, IloInt to) const {
    double dx = getX(from) - getX(to);
    double dy = getY(from) - getY(to);
    return IloPower(dx*dx + dy*dy, 0.5);
  }
  IloInt getDroneTime(IloInt from, IloInt to) const {
    return IntegerTime(getDistance(from, to) * _droneSpeed);
  }
  IloInt getTruckTime(IloInt from, IloInt to) const {
    return IntegerTime(getDistance(from, to) * _truckSpeed);
  }
};

ProblemData::ProblemData(IloEnv env, std::istream& in)
  : _xco(env)
  , _yco(env)
  , _truckSpeed(-1) 
  , _droneSpeed(-1) 
  , _numCustomers(-1) {
    ignore(in, 5); // ignore /*The speed of the Truck*/
    in >> _truckSpeed;
    ignore(in, 5); // ignore /*The speedo of the Drone*/
    in >> _droneSpeed;
    ignore(in, 3); // ignore /*Number of Nodes*/
    in >> _numCustomers;
    _numCustomers--;
    ignore(in, 2); // ignore /*The Depot*/
    IloNum x, y;
    in >> x >> y; ignore(in, 1); // ignore depot name
    _xco.add(x); _yco.add(y);
    ignore(in, 5); // ignore /*The Locations (x_coor y_coor name)*/
    for (IloInt i = 0; i < _numCustomers; i++) {
      in >> x >> y; ignore(in, 1); // ignore customer name
      _xco.add(x); _yco.add(y);
    }
}

const char * MakeName(IloEnv env, const char * base, IloInt i) {
  char * buf = new (env) char[32 + strlen(base)];
  snprintf(buf, 32 + strlen(base), "%s[%ld]", base, (long) i);
  return buf;
}

const char * MakeName(IloEnv env, const char * base, IloInt i, IloInt j) {
  char * buf = new (env) char[64 + strlen(base)];
  snprintf(buf, 64 + strlen(base), "%s[%ld][%ld]", base, (long)i, (long)j);
  return buf;
}


IloInt GetIndex(IloIntervalVarArray x, IloIntervalVar itv) {
  for (IloInt i = 0; i < x.getSize(); i++) {
    if (x[i].getImpl() == itv.getImpl()) {
      return i;
    }
  }
  return -1;
}

void GetBeforeAfter(IloCP cp,
                    IloInt dvIndex,
                    IloArray<IloIntervalVarArray> tdVisit,
                    IloArray<IloIntervalVarArray> dtVisit,
                    IloInt& dvBefore,
                    IloInt& dvAfter) {
  IloInt n = tdVisit.getSize();
  dvBefore = -1;
  dvAfter = -1;
  assert(dvIndex == -1 || (dvIndex >= 1 && dvIndex < n - 1));
  if (dvIndex >= 0) {
    for (IloInt j = 0; j < n; j++) {
      if (cp.isPresent(tdVisit[dvIndex][j])) {
        assert(dvBefore < 0);
        dvBefore = j;
      }
      if (cp.isPresent(dtVisit[dvIndex][j])) {
        assert(dvAfter < 0);
        dvAfter = j;
      }
    }
  }
  assert(dvIndex == -1 || dvBefore >= 0);
  assert(dvIndex == -1 || dvAfter >= 0);
}

void Solve(IloEnv env, const ProblemData& pd, IloNum tlim) {
  // Model variables
  IloInt n = pd.getNumNodes() + 1;
  IloIntervalVarArray    visit(env, n);
  IloIntervalVarArray    tVisit(env, n);
  IloIntervalVarArray    dVisit(env, n);
  IloIntervalVarArray    dVisitBefore(env, n);
  IloIntervalVarArray    dVisitAfter(env, n);
  IloIntArray            tVisitTypes(env, n);

  IloArray<IloIntervalVarArray> tdVisit(env, n);
  IloArray<IloIntervalVarArray> dtVisit(env, n);
  for (IloInt i = 0; i < n; i++) {
    visit[i] = IloIntervalVar(env, MakeName(env, "visit", i));

    tVisit[i] = IloIntervalVar(env, MakeName(env, "tVisit", i));
    tVisit[i].setOptional();

    dVisit[i] = IloIntervalVar(env, MakeName(env, "dVisit", i));
    dVisit[i].setOptional();

    dVisitBefore[i] = IloIntervalVar(env, MakeName(env, "dVisitBefore", i));
    dVisitBefore[i].setOptional();

    dVisitAfter[i] = IloIntervalVar(env, MakeName(env, "dVisitAfter", i));
    dVisitAfter[i].setOptional();

    tVisitTypes[i] = i;

    tdVisit[i] = IloIntervalVarArray(env, n);
    dtVisit[i] = IloIntervalVarArray(env, n);

    for (IloInt j = 0; j < n; j++) {
      tdVisit[i][j] = IloIntervalVar(env, MakeName(env, "tdVisit", i, j));
      tdVisit[i][j].setOptional();

      dtVisit[i][j] = IloIntervalVar(env, MakeName(env, "dtVisit", i, j));
      dtVisit[i][j].setOptional();

    }
  }
  tVisitTypes[n-1] = 0;
  IloIntervalSequenceVar tVisitSeq(env, tVisit, tVisitTypes);
  IloIntervalSequenceVar dVisitSeq(env, dVisit);

  IloArray<IloIntArray> truckTime(env, n - 1);
  for (IloInt i = 0; i < n - 1; i++) {
    truckTime[i] = IloIntArray(env, n - 1);
    for (IloInt j = 0; j < n - 1; j++)
      truckTime[i][j] = pd.getTruckTime(i, j);
  }

  // Constraints
  IloModel mdl(env);

  // Truck's depot visits must be present and bookend the customer visits
  tVisit[0].setPresent();
  tVisit[0].setStartMin(0);
  tVisit[0].setStartMax(0);
  tVisit[n-1].setPresent();
  mdl.add(IloFirst(env, tVisitSeq, tVisit[0]));
  mdl.add(IloLast(env, tVisitSeq, tVisit[n-1]));
  for (IloInt i = 1; i < n - 1; i++) {
    mdl.add(IloStartBeforeStart(env, tVisit[0], visit[i]));
    mdl.add(IloEndBeforeEnd(env, visit[i], tVisit[n-1]));
  }

  // Truck and drone can only do one thing at once.
  // Truck respects travel times.
  mdl.add(IloNoOverlap(env, tVisitSeq, truckTime));
  mdl.add(IloNoOverlap(env, dVisitSeq));

  for (IloInt i = 0; i < n; i++) {
    IloIntervalVarArray visitAlt(env);
    visitAlt.add(tVisit[i]); visitAlt.add(dVisit[i]);
    mdl.add(IloAlternative(env, visit[i], visitAlt));
    mdl.add(IloAlternative(env, dVisitBefore[i], tdVisit[i]));
    mdl.add(IloAlternative(env, dVisitAfter[i], dtVisit[i]));

    IloIntervalVarArray droneOutIn(env);
    droneOutIn.add(dVisitBefore[i]); droneOutIn.add(dVisitAfter[i]);
    mdl.add(IloSpan(env, dVisit[i], droneOutIn));
    mdl.add(IloEndAtStart(env, dVisitBefore[i], dVisitAfter[i]));
    mdl.add(IloPresenceOf(env, dVisit[i]) == IloPresenceOf(env, dVisitBefore[i]));
    mdl.add(IloPresenceOf(env, dVisit[i]) == IloPresenceOf(env, dVisitAfter[i]));
  }

  for (IloInt i = 1; i < n - 1; i++) { // Customer visits
    for (IloInt j = 0; j < n; j++) {
      IloInt outTime = pd.getDroneTime(tVisitTypes[j], tVisitTypes[i]);
      IloInt inTime = pd.getDroneTime(tVisitTypes[i], tVisitTypes[j]);
#if 1
      mdl.add(IloLengthOf(tdVisit[i][j], IloIntervalMax) >= outTime);
      mdl.add(IloLengthOf(dtVisit[i][j], IloIntervalMax) >= inTime);
#else
      tdVisit[i][j].setLengthMin(outTime); tdVisit[i][j].setLengthMax(outTime);
      mdl.add(IloLengthOf(dtVisit[i][j], IloIntervalMax) >= inTime);
#endif
      if (j >= 1 && j < n-1) {
        mdl.add(IloIfThen(env, IloPresenceOf(env, tdVisit[i][j]), IloPresenceOf(env, tVisit[j])));
        mdl.add(IloIfThen(env, IloPresenceOf(env, dtVisit[i][j]), IloPresenceOf(env, tVisit[j])));
      }
      mdl.add(IloStartBeforeStart(env, tVisit[j], tdVisit[i][j]));
      mdl.add(IloStartBeforeEnd(env, tdVisit[i][j], tVisit[j]));
      mdl.add(IloStartBeforeEnd(env, tVisit[j], dtVisit[i][j]));
      mdl.add(IloEndBeforeEnd(env, dtVisit[i][j], tVisit[j]));
    }
  }
  mdl.add(IloMinimize(env, RealTime(IloEndOf(visit[n-1]))));

  IloCP cp(mdl);
#if 0
  IloIntervalSequenceVarArray seqs(env); seqs.add(tVisitSeq); seqs.add(dVisitSeq);
  IloSearchPhaseArray phases(env);
  phases.add(IloSearchPhase(env, seqs));
  cp.setSearchPhases(phases);
#endif
  cp.setParameter(IloCP::TimeLimit, tlim);
  cp.setParameter(IloCP::LogPeriod, 100000);
  IloBool ok = cp.solve();

  cp.out() << std::endl;
  if (ok) {
    cp.out() << "Truck visits customers:";
    for (IloIntervalVar v = cp.getFirst(tVisitSeq);
         v.getImpl() != 0;
         v = cp.getNext(tVisitSeq, v)) {
      IloInt idx = GetIndex(tVisit, v);
      if (idx >= 1 && idx < n - 1)
        cp.out() << " " << idx;
    }
    cp.out() << std::endl;

    cp.out() << "Drone visits customers:";
    for (IloIntervalVar v = cp.getFirst(dVisitSeq);
         v.getImpl() != 0;
         v = cp.getNext(dVisitSeq, v)) {
      IloInt idx = GetIndex(dVisit, v);
      if (idx >= 1 && idx < n - 1)
        cp.out() << " " << idx;
    }
    cp.out() << std::endl;

    IloIntervalVar tv = cp.getFirst(tVisitSeq);
    IloIntervalVar dv = cp.getFirst(dVisitSeq);
    IloInt tvi = GetIndex(tVisit, tv);
    IloInt dvi = GetIndex(dVisit, dv);
    IloInt dvBefore = -1;
    IloInt dvAfter = -1;
    GetBeforeAfter(cp, dvi, tdVisit, dtVisit, dvBefore, dvAfter);
    cp.out() << std::endl;
    while (tvi >= 0) {
      // At loop entry, the drone is on the truck or joins it here.
      if (tvi >= 1 && tvi < n - 1)
        cp.out() << std::setw(3) << tvi;
      else
        cp.out() << "DEPOT";

      // Deal with drone flights while the truck does not move
      while (dvBefore == tvi && dvAfter == tvi) {
        cp.out() << " = " << dvi;
        dv = cp.getNext(dVisitSeq, dv);
        dvi = GetIndex(dVisit, dv);
        GetBeforeAfter(cp, dvi, tdVisit, dtVisit, dvBefore, dvAfter);
      }

      cp.out() << std::endl;
      const char * bb = "  | |"; // double bar
      const char * sb1 = "  |";  // single bar (truck)
      const char * sb2 = " |";   // double bar (drone)
      if (dvBefore == tvi) { // Is there a drone flight from here?
        // How many truck legs does the drone jump (1 means land next stop)
        IloInt jumps = 0;
        IloIntervalVar ntv = tv;
        while (ntv.getImpl() != tVisit[dvAfter].getImpl()) {
          ntv = cp.getNext(tVisitSeq, ntv);
          jumps++;
        }
        assert(jumps >= 1);
        cp.out() << sb1 << "\\" << std::endl;
        if (jumps % 2 == 1) { // odd number of jumps
          if (jumps > 1)
            cp.out() << bb << std::endl;
          for (IloInt i = 0; i < (jumps - 1) / 2; i++) {
            tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
            cp.out() << bb << std::endl
                     << std::setw(3) << tvi << sb2 << std::endl
                     << bb << std::endl;
          }
          cp.out() << sb1 << " " << dvi << std::endl;
          for (IloInt i = 0; i < (jumps - 1) / 2; i++) {
            tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
            cp.out() << bb << std::endl
                     << std::setw(3) << tvi << sb2 << std::endl
                     << bb << std::endl;
          }
          if (jumps > 1)
            cp.out() << bb << std::endl;
        }
        else { // even number of jumps
          for (IloInt i = 0; i < jumps / 2 - 1; i++) {
            tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
            cp.out() << bb << std::endl << bb << std::endl
                     << std::setw(3) << tvi << sb2 << std::endl
                     << bb << std::endl;
          }
          tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
          cp.out() << bb << std::endl << bb << std::endl
                   << std::setw(3) << tvi << " " << dvi << std::endl
                   << bb << std::endl << bb << std::endl;
          for (IloInt i = 0; i < jumps / 2 - 1; i++) {
            tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
            cp.out() << bb << std::endl
                     << std::setw(3) << tvi << sb2 << std::endl
                     << bb << std::endl << bb << std::endl;
          }
        }
        cp.out() << sb1 << "/" << std::endl;
        dv = cp.getNext(dVisitSeq, dv); dvi = GetIndex(dVisit, dv);
        GetBeforeAfter(cp, dvi, tdVisit, dtVisit, dvBefore, dvAfter);
      }
      else if (tvi != n-1) { // no flight from here
        cp.out() << sb1 << std::endl << sb1 << std::endl << sb1 << std::endl;
      }
      tv = cp.getNext(tVisitSeq, tv); tvi = GetIndex(tVisit, tv);
    }
  }
  else {
    cp.out() << "No solution found" << std::endl;
  }
}

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    const char * fname = "../../../examples/data/uniform-1-n11.txt";
    IloNum tlim = 10.0;
    if (argc > 1)
      fname = argv[1];
    if (argc > 2)
      tlim = atof(argv[2]);
    std::ifstream in(fname);
    if (!in.good())
      throw IloCP::Exception(0, "Could not open the data file");

    ProblemData pd(env, in);
    in.close();

    Solve(env, pd, tlim);
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
