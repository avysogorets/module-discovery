// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_rcpspmm.cpp
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

The MMRCPSP (Multi-Mode Resource-Constrained Project Scheduling
Problem) is a generalization of the Resource-Constrained Project
Scheduling problem (see sched_rcpsp.cpp). In the MMRCPSP, each
activity can be performed in one out of several modes. Each mode of an
activity represents an alternative way of combining different levels
of resource requirements with a related duration. Renewable and
no-renewable resources are distinguished. While renewable resources
have a limited instantaneous availability such as manpower and
machines, non renewable resources are limited for the entire project,
allowing to model, e.g., a budget for the project.  The objective is
to find a mode and a start time for each activity such that the
schedule is makespan minimal and feasible with regard to the
precedence and resource constraints.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]) {
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/rcpspmm_default.data";
    IloInt failLimit = 30000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file) {
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    IloInt nbTasks, nbRenewable, nbNonRenewable;
    file >> nbTasks >> nbRenewable >> nbNonRenewable;
    IloCumulFunctionExprArray renewables(env, nbRenewable);
    IloIntExprArray nonRenewables(env, nbNonRenewable);
    IloIntArray capRenewables   (env, nbRenewable);
    IloIntArray capNonRenewables(env, nbNonRenewable);
    for (IloInt j = 0; j < nbRenewable; j++) {
      renewables[j] = IloCumulFunctionExpr(env);
      file >> capRenewables[j];
    }
    for (IloInt j = 0; j < nbNonRenewable; j++) {
      nonRenewables[j] = IloIntExpr(env);
      file >> capNonRenewables[j];
    }

    IloIntervalVarArray  tasks(env, nbTasks);
    IloIntervalVarArray2 modes(env, nbTasks);
    for (IloInt i = 0; i < nbTasks; i++) {
      tasks[i] = IloIntervalVar(env);
      modes[i] = IloIntervalVarArray(env);
    }
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbTasks; i++) {
      char name[100];
      snprintf(name, sizeof(name), "T[%ld]", (long)i);
      IloIntervalVar task = tasks[i];
      task.setName(name);
      IloInt d, nbModes, nbSucc;
      file >> d >> nbModes >> nbSucc;
      for (IloInt k = 0; k < nbModes; ++k) {
        snprintf(name, sizeof(name), "M[%ld][%ld]", (long)i, (long)k);
        IloIntervalVar alt(env, name);
        alt.setOptional();
        modes[i].add(alt);
      }
      model.add(IloAlternative(env, task, modes[i]));
      ends.add(IloEndOf(task));
      for (IloInt s = 0; s < nbSucc; ++s) {
        IloInt succ;
        file >> succ;
        model.add(IloEndBeforeStart(env, task, tasks[succ]));
      }
    }
    for (IloInt i = 0; i < nbTasks; i++) {
      IloIntervalVarArray imodes = modes[i];
      for (IloInt k = 0; k < imodes.getSize(); ++k) {
        IloInt taskId, modeId, d;
        file >> taskId >> modeId >> d;
        imodes[k].setSizeMin(d);
        imodes[k].setSizeMax(d);
        IloInt q;
        for (IloInt j = 0; j < nbRenewable; j++) {
          file >> q;
          if (0 < q) {
            renewables[j] += IloPulse(imodes[k], q);
          }
        }
        for (IloInt j = 0; j < nbNonRenewable; j++) {
          file >> q;
          if (0 < q) {
            nonRenewables[j] += q * IloPresenceOf(env, imodes[k]);
          }
        }
      }
    }

    for (IloInt j = 0; j < nbRenewable; j++) {
      model.add(renewables[j] <= capRenewables[j]);
    }

    for (IloInt j = 0; j < nbNonRenewable; j++) {
      model.add(nonRenewables[j] <= capNonRenewables[j]);
    }

    IloObjective objective = IloMinimize(env, IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
      cp.out() << "Makespan \t: " << cp.getObjValue() << std::endl;
    }
    else {
      cp.out() << "No solution found."  << std::endl;
    }
    cp.end();
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
