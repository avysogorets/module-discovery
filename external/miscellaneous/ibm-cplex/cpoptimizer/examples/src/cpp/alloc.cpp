// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/alloc.cpp
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

Frequency assignment problem
----------------------------

Problem Description

The problem is given here in the form of discrete data; that is,
each frequency is represented by a number that can be called its
channel number.  For practical purposes, the network is divided
into cells (this problem is an actual cellular phone problem).
In each cell, there is a transmitter which uses different
channels.  The shape of the cells have been determined, as well
as the precise location where the transmitters will be
installed.  For each of these cells, traffic requires a number
of frequencies.

Between two cells, the distance between frequencies is given in
the matrix on the next page.

The problem of frequency assignment is to avoid interference.
As a consequence, the distance between the frequencies within a
cell must be greater than 16.  To avoid inter-cell interference,
the distance must vary because of the geography.

------------------------------------------------------------ */
#include <ilcp/cp.h>

// ----------------------------------------------------------------------------
const int nbCell               = 25;
const int nbAvailFreq          = 256;
const int nbChannel[nbCell] =
  { 8, 6, 6, 1, 4, 4, 8, 8, 8, 8, 4, 9, 8, 4, 4, 10, 8, 9, 8, 4, 5, 4, 8, 1, 1 };
const int dist[nbCell][nbCell] = {
  { 16, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1 },
  { 1, 16, 2, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 2, 16, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 16, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 2, 16, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 2, 2, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 16, 2, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 2, 16, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 1, 1 },
  { 1, 2, 2, 0, 0, 0, 0, 0, 16, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
  { 1, 2, 2, 0, 0, 0, 0, 0, 2, 16, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 1, 1, 2, 2, 16, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 0, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 1, 1, 2, 2, 2, 16, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 0, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 1, 1, 2, 2, 2, 2, 16, 2, 2, 2, 2, 1, 1, 2, 1, 1, 0, 1, 1 },
  { 2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 16, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 1, 1, 16, 2, 2, 2, 1, 2, 2, 1, 2, 2 },
  { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 1, 1, 2, 16, 2, 2, 1, 2, 2, 1, 2, 2 },
  { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 16, 2, 2, 1, 1, 0, 2, 2 },
  { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 16, 2, 1, 1, 0, 2, 2 },
  { 0, 0, 0, 1, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 2, 2, 16, 1, 1, 0, 1, 1 },
  { 2, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 16, 2, 1, 2, 2 },
  { 2, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 2, 16, 1, 2, 2 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 16, 1, 1 },
  { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1, 16, 2 },
  { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1, 2, 16 }
};

// ----------------------------------------------------------------------------
// Helper functions

IloInt GetNumTransmitters() {
  IloInt t = 0;
  for (IloInt i = 0; i < nbCell; i++)
    t += nbChannel[i];
  return t;
}

IloInt GetCell(IloInt t) {
  IloInt c = 0;
  while (t >= nbChannel[c]) {
    t -= nbChannel[c];
    c++;
  }
  return c;
}

IloInt GetMinDistance(IloInt t1, IloInt t2) {
  return dist[GetCell(t1)][GetCell(t2)];
}

//
// Find a bound based on a maximum clique problem.
// We create a graph where an edge connects two vertices
// if the two vertices represent transmitters which must
// have different frequencies.  A clique if size k in this
// graph means that at least k different frequencies must
// be used.  Note that any clique provides a valid bound
// and hence the search can be time bounded and still
// provide a usable bound.
//
IloInt MaxCliqueBound() {
  IloEnv env;
  IloInt lb = 1;
  try {
    IloModel model(env);
    IloInt nbTransmitters = GetNumTransmitters();
    IloIntVarArray inClique(env, nbTransmitters, 0, 1);
    for (IloInt t2 = 1; t2 < nbTransmitters; t2++)
      for (IloInt t1 = 0; t1 < t2; t1++) {
        if (GetMinDistance(t1, t2) == 0) {
          model.add(inClique[t1] == 0 || inClique[t2] == 0);
      }
    }
    model.add(IloMaximize(env, IloSum(inClique)));
    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, 10);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    IloBool ok = cp.solve();
    if (ok) lb = (IloInt)cp.getObjValue();
  } catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return lb;
}

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt nbTransmitters = GetNumTransmitters();
    IloIntVarArray freq(env, nbTransmitters, 0, nbAvailFreq - 1);
    freq.setNames("freq");
    for (IloInt t2 = 1; t2 < nbTransmitters; t2++) {
      for (IloInt t1 = 0; t1 < t2; t1++) {
        IloInt d = GetMinDistance(t1, t2);
        if (d > 0)
          model.add(IloAbs(freq[t1] - freq[t2]) >= GetMinDistance(t1, t2));
      }
    }
    // Minimize the total number of frequencies, using the clique bound
    IloIntExpr nbFreq = IloCountDifferent(freq);
    IloInt lb = MaxCliqueBound();
    model.add(nbFreq >= lb);
    model.add(IloMinimize(env, nbFreq));

    IloCP cp(model);
    if (cp.solve()) {
      for (IloInt t = 0; t < nbTransmitters; t++) {
        IloInt c = GetCell(t);
        if (c > 0 && GetCell(t-1) != c)
          std::cout << std::endl;
        std::cout << cp.getValue(freq[t]) << " ";
      }
      std::cout << std::endl;
      cp.out() << "Total # of sites       " << nbTransmitters << std::endl;
      cp.out() << "Total # of frequencies " << cp.getValue(nbFreq) << std::endl;
    }
    else
      cp.out() << "No solution found."  << std::endl;
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
