// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_jobshop_blackbox.cpp
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

This example illustrates how to use blackbox expressions to solve
a job-shop scheduling problem with uncertain operation durations.

It is an alternative approach to the stochastic optimization technique
illustrated in example sched_stochastic_jobshop.cpp.

The problem is an extension of the classical job-shop scheduling
problem (see sched_jobshop.cpp).

In the classical job-shop scheduling problem a finite set of jobs is
processed on a finite set of machines. Each job is characterized by a
fixed order of operations, each of which is to be processed on a
specific machine for a specified duration.  Each machine can process
at most one operation at a time and once an operation initiates
processing on a given machine it must complete processing
uninterrupted.  The objective of the problem is to find a schedule
that minimizes the makespan of the schedule.

In the present version of the problem, the duration of a given operation
is uncertain and supposed to be a uniform random variable varying in an
operation specific range [min, max]. The objective of the problem is to
find an ordering of the operations on the machines that minimizes
the average makespan of the schedule.

The estimation of the average makespan is computed by a blackbox expression
MakespanAVG on the set of sequence variables of the model (see the use of
macro ILOBLACKBOX4). This blackbox expression simulates the execution of the
specified sequences of operations on the machines on a number of samples and
returns an estimation of the average makespan.

The model uses this blackbox expression as objective function to be minimized.
Note that the model uses the average duration of operations (dmin+dmax)/2 as
deterministic duration to guide the search. One can show that the makespan
of the deterministic problem using the average durations is always lesser than
the average makespan, this is why it can be used as a lower bound on the blackbox
objective function.

The simulation implements a class PrecedenceGraphSimulator that simulates
the execution of operations with uncertain durations under precedence constraints
in order to estimate the average makespan. Two simulation techniques are ilustrated
in the example: Monte-Carlo simulation and Descriptive Sampling
(depending on the definition of macro DESCRIPTIVE_SAMPLING).

For each sample, Monte-Carlo simulation simply draws a random value in the
specified range for the duration of the each operation and simulates the execution
of this precedence graph to compute a sample of the makespan.

Descriptive sampling [1] is a more robust technique for sampling operations duration
in the context of simulating a precedence graph. It ensures that for a given operation,
the probability distibution of its duration is efficiently sampled by controling
the input set of sampled values.

Typically, a similar precision on the average makespan is achieved with 30 samples
of Descriptive sampling instead of typically 200 samples in Monte-Carlo simulation.

[1] E. Saliby. "Descriptive Sampling: A Better Approach to Monte Carlo Simulation".
The Journal of the Operational Research Society
Vol. 41, No. 12 (Dec., 1990), pp. 1133-1142

The blackbox expression MakespanAVG is passed as data fields its scope
(the array of sequence variables representing the sequence of operations
on each machines), as well as the ranges of the operation durations (dmin, dmax),
and a pointer to an instance of precedence graph simulator (PrecedenceGraphSimulator)
that will be lazily created when the blackbox expression is evaluated for the first time.

Note that the CP Optimizer engine will create several instances of the blackbox expression
(one for each parallel worker) and the evaluation of these functions will be performed
concurrently. Each worker will create its own instance of precedence graph simulator
so the code will safely run in parallel. The blackbox expression evaluation code uses a
local random generator accessible with functions reSeed(...) and getInt(...).
As these random generators are local to the blackbox expression instance in the engine,
this code is multi-thread safe. Re-seeding the random generator ensures that
calling twice the function evaluation on the same sequences of operations
on the machine will result in the same estimation of the average makespan.

For more information on blackbox expressions, see the concept "Blackbox expressions"
in the CP Optimizer C++ API Reference Manual.

------------------------------------------------------------ */

#include <ilcp/cpext.h>

// A simple two-dimensional array
class Array2 {
public:
  Array2() {}
  Array2(IloInt* ptr, IloInt m)  : _array(ptr), _m(m) { }
  IloInt* operator[](IloInt i) { return _array + (i*_m); }
private:
  IloInt* _array;
  IloInt _m;
};

// A class for simulating the execution of a set of operations constrained
// by some precedence relations
class PrecedenceGraphSimulator {
public:
  // n is the number of operations/nodes, m is the number of samples
  PrecedenceGraphSimulator(IlcBlackbox* alloc, IloInt n, IloInt m, IloInt w);
  ~PrecedenceGraphSimulator() {}
  void reinit(); // Reinit to empty graph
  // Add precedence relation between node i and node j
  void addPrecedence(IloInt i, IloInt j);
  // Set duration of node i in sample s
  void setDuration(IloInt i, IloInt s, IloInt d) { _durations[i][s] = d; }
  void simulate();
  IloNum getAverageMakespan() const;
private:
  IloInt  _n; // Number of nodes
  IloInt  _m; // Number of samples
  Array2  _durations;
  Array2  _ends;
  IloInt* _makespan;
  IloInt* _nbIncoming;
  IloInt* _nbIncoming0;
  IloInt* _head;
  Array2  _outgoing; // _outgoing[i] is the set of nodes after node i
};

PrecedenceGraphSimulator::PrecedenceGraphSimulator(IlcBlackbox* alloc, IloInt n, IloInt m, IloInt w)
  : _n           (n)
  , _m           (m)
  , _durations   ()
  , _ends        ()
  , _makespan    (NULL)
  , _nbIncoming  (NULL)
  , _nbIncoming0 (NULL)
  , _head        (NULL)
  , _outgoing    () {
  // All the data of the structure is allocated in a contiguous array for performance reasons
  IloInt* array = new (alloc) IloInt[2*n*m+m+(4+w)*n];
  _durations   = Array2(array, m);
  _ends        = Array2(array+(n*m), m);
  _makespan    = array+2*(n*m);
  _nbIncoming  = _makespan+m;
  _nbIncoming0 = _nbIncoming+n;
  _head        = _nbIncoming0+n;
  // w is the maximal out-degree of the precedence graph
  // (2 in case of the job-shop scheduling problem)
  _outgoing    = Array2(_head+n, w+1);
}

void PrecedenceGraphSimulator::reinit() {
  memset(_nbIncoming0,  0, sizeof(IloInt)*_n);
  memset(_outgoing[0], -1, sizeof(IloInt)*_n*3);
}

void PrecedenceGraphSimulator::addPrecedence(IloInt i, IloInt j) {
  _nbIncoming0[j]++;
  IloInt k = 0;
  while (0 <= _outgoing[i][k]) { k++; }
  _outgoing[i][k] = j;
}

void PrecedenceGraphSimulator::simulate() {
  memset(_makespan, 0, sizeof(IloInt)*_m );
  memcpy(_nbIncoming, _nbIncoming0, sizeof(IloInt)*_n );
  memset(_ends[0], 0, sizeof(IloInt)*_n*_m);
  IloInt s = 0; // Number of schedulable nodes
  for (IloInt i = 0; i<_n; ++i) {
    if (_nbIncoming[i] == 0) {
      for (IloInt j = 0; j<_m; ++j) {
        _ends[i][j] = _durations[i][j];
        _makespan[j] = IloMax(_makespan[j], _ends[i][j]);
      }
      _head[s++] = i;
    }
  }
  while (s > 0) {
    IloInt h = _head[--s];
    for (IloInt* o = _outgoing[h]; 0 <= *o; ++o) {
      IloInt i = *o;
      for (IloInt j = 0; j<_m; ++j) {
        _ends[i][j]  = IloMax(_ends[i][j], _ends[h][j] + _durations[i][j]);
        _makespan[j] = IloMax(_makespan[j], _ends[i][j]);
      }
      if (--_nbIncoming[i] == 0) {
        _head[s++] = i;
      }
    }
  }
}

IloNum PrecedenceGraphSimulator::getAverageMakespan() const {
  IloNum sum = 0.0;
  for (IloInt j = 0; j<_m; ++j) {
    sum += _makespan[j];
  }
  return sum/IloNum(_m);
}

#define DESCRIPTIVE_SAMPLING

#if defined(DESCRIPTIVE_SAMPLING)
IloInt NbSamples  = 30;
#else
IloInt NbSamples  = 200;
#endif

ILOBLACKBOX4(MakespanAVG,
  IloIntervalSequenceVarArray, mchSeqs,
  IloIntArray2, dmins,
  IloIntArray2, dmaxs,
  PrecedenceGraphSimulator*, simu
) {
  IloInt nbJobs     = mchSeqs[0].getSize();
  IloInt nbMachines = mchSeqs.getSize();

  if (simu == NULL) {
    // Lazily create the simulator at first evaluation
    simu = new (this) PrecedenceGraphSimulator(this, nbJobs*nbMachines, NbSamples, 2);
  }
  // Reinit simulator
  simu->reinit();

  // Add precedences in the simulator
  for (IloInt i = 0; i < nbJobs; ++i)
    for (IloInt j = 1; j < nbMachines; ++j)
      simu->addPrecedence(i*nbMachines+j-1, i*nbMachines+j);

  for (IloInt j = 0; j < nbMachines; ++j)
    for (IloIntervalVar p = 0, c = getFirst(mchSeqs[j]);
         c.getImpl() != 0;
         p = c, c = getNext(mchSeqs[j], c))
      if (p.getImpl() != 0)
        simu->addPrecedence((IloInt)p.getObject(), (IloInt)c.getObject());

  // Sample operation durations
  reSeed(1); // For determinism, uses local random generator

#if defined(DESCRIPTIVE_SAMPLING)
  IloInt* vals = new IloInt[NbSamples];
#endif

  for (IloInt i = 0; i < nbJobs; ++i) {
    for (IloInt j = 0; j < nbMachines; ++j) {
#if defined(DESCRIPTIVE_SAMPLING)
      // Select values to conform as closely as possible to the uniform distribution
      IloNum min = dmins[i][j]-0.5;
      IloNum max = dmaxs[i][j]+0.5;
      IloNum step = (max-min)/IloNum(NbSamples);
      for (IloInt s = 0; s < NbSamples; ++s) {
        vals[s] = (IloInt)IloFloor(min+step*(s+getFloat())+0.5);
      }
      // Shuffle the selected values
      for (IloInt s = 0; s < NbSamples; ++s) {
        // Uses local random generator to randomly shuffle the set of values
        IloInt u = s+getInt(NbSamples-s);
        IloInt tmp = vals[s];
        vals[s] = vals[u];
        vals[u] = tmp;
        simu->setDuration(i*nbMachines+j, s, vals[s]);
      }
#else
      for (IloInt s = 0; s < NbSamples; ++s) {
        // Uses local random generator to draw random duration
        IloInt d = dmins[i][j] + getInt(dmaxs[i][j]-dmins[i][j]+1);
        simu->setDuration(i*nbMachines+j, s, d);
      }
#endif
    }
  }

#if defined(DESCRIPTIVE_SAMPLING)
    delete [] vals;
#endif

  // Simulate execution
  simu->simulate();

  returnValue(simu->getAverageMakespan());
}

class FileError : public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]) {
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/jobshop_blackbox_default.data";
    IloInt failLimit = 250000;
    if (argc > 1) {
      if (strcmp(argv[1], "-") != 0)
        filename = argv[1];
    }
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file) {
      env.out() << "Could not open " << filename << std::endl;
      env.out() << "Usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      env.out() << "       " << "(use '-' to mean the default input file)" << std::endl;
      throw FileError();
    }

    // Data reading
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;

    IloIntArray2 mchs (env, nbJobs);
    IloIntArray2 dmins(env, nbJobs);
    IloIntArray2 dmaxs(env, nbJobs);
    for (IloInt i = 0; i < nbJobs; ++i) {
      mchs [i] = IloIntArray(env, nbMachines);
      dmins[i] = IloIntArray(env, nbMachines);
      dmaxs[i] = IloIntArray(env, nbMachines);
      for (IloInt j = 0; j < nbMachines; ++j) {
        file >> mchs[i][j] >> dmins[i][j] >> dmaxs[i][j];
      }
    }

    // CP Optimizer model
    IloModel model(env);
    IloIntervalVarArray2 jobOps(env, nbJobs);
    IloIntervalVarArray2 mchOps(env, nbMachines);
    IloIntervalSequenceVarArray mchSeqs(env, nbMachines);
    IloIntExprArray ends(env, nbJobs);
    for (IloInt i = 0; i < nbJobs; ++i) {
      jobOps[i] = IloIntervalVarArray(env, nbMachines);
    }
    for (IloInt j = 0; j < nbMachines; ++j) {
      mchOps[j] = IloIntervalVarArray(env, nbJobs);
    }
    char name[100];
    for (IloInt i = 0; i < nbJobs; ++i) {
      for (IloInt j = 0; j < nbMachines; ++j) {
        snprintf(name, sizeof(name), "O_%ld_%ld", (long)i, (long)j);
        // Duration of operations in the model is the average duration
        IloIntervalVar op(env, ((dmins[i][j]+dmaxs[i][j]))/2, name);
        op.setObject(IloAny(i*nbMachines+j));
        jobOps[i][j] = op;
        mchOps[mchs[i][j]][i] = op;
        if (0 < j) {
          // Precedence constraints between successive operations of a job
          model.add(IloEndBeforeStart(env, jobOps[i][j-1], jobOps[i][j]));
        }
        if (j == nbMachines-1) {
          ends[i] = IloEndOf(op);
        }
      }
    }
    IloIntExpr makespanLB = IloMax(ends);
    for (IloInt j = 0; j < nbMachines; ++j) {
      snprintf(name, sizeof(name), "M_%ld", (long)j);
      // Sequence of operations on machine j
      mchSeqs[j] = IloIntervalSequenceVar(env, mchOps[j], name);
      // Operations executed by machine j do not overlap
      model.add(IloNoOverlap(env, mchSeqs[j]));
    }

    // Blackbox expression representing the estimated average makespan
    IloNumExpr makespanAVG = MakespanAVG(env, mchSeqs, dmins, dmaxs, NULL);

    // Lower bound: we exploit the property that makespan of the deterministic problem
    // with average durations is lesser than the average makespan
    model.add(makespanLB <= makespanAVG);

    model.add(IloMinimize(env, makespanAVG));
    //model.add(IloMinimize(env, IloStaticLex(env, makespanLB, makespanAVG)));

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.setParameter(IloCP::LogPeriod, 1000000);

    cp.solve();
    cp.end();
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }

  env.end();
  return 0;
}
