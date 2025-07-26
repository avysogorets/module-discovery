// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/cvrptw.cpp
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


------------------------------------------------------------ */

#include <ilcp/cp.h>

// Holds a representation of the raw data of the problem. Indices in
// the getter API are customer indicies from 0 to getNbCustomers()-1
class CVRPTWProblem {
private:
  class Node {
  public:
    IloInt x;
    IloInt y;
    IloInt demand;
    IloInt earliestStart;
    IloInt latestStart;
    IloInt serviceTime;
  };

  IloEnv         _env;
  IloInt         _nbVehicles;
  IloInt         _capacity;
  IloInt         _maxHorizon;
  IloInt         _depot;
  IloArray<Node> _data;

  IloInt custId(IloInt i) const { return i + (i >= _depot); }
  void read(std::istream& in);
  void skip(std::istream& in, IloInt n = 1) {
    std::string dummy;
    for (IloInt i = 0; i < n; i++)
      in >> dummy;
  }
public:
  CVRPTWProblem(IloEnv env, std::istream &in) : _env(env), _data(env) { read(in); }
  ~CVRPTWProblem() { _data.end(); }
  IloEnv getEnv() const { return _env; }
  IloInt getX(IloInt i) const { return _data[custId(i)].x; }
  IloInt getY(IloInt i) const { return _data[custId(i)].y; }
  IloInt getDemand(IloInt i) const { return _data[custId(i)].demand; }
  IloInt getEarliestStart(IloInt i) const { return _data[custId(i)].earliestStart; }
  IloInt getLatestStart(IloInt i) const { return _data[custId(i)].latestStart; }
  IloInt getServiceTime(IloInt i) const { return _data[custId(i)].serviceTime; }
  IloInt getDepotX() const { return _data[_depot].x; }
  IloInt getDepotY() const { return _data[_depot].y; }
  IloInt getMaxHorizon() const { return _data[_depot].latestStart; }
  IloInt getCapacity() const { return _capacity; }
  IloInt getNbVehicles() const { return _nbVehicles; }
  IloInt getNbCustomers() const { return _data.getSize() - 1; }
};

void CVRPTWProblem::read(std::istream& in) {
  skip(in, 4);
  in >> _nbVehicles;
  in >> _capacity;
  skip(in, 12);
  do {
    IloInt number;
    Node nd;
    in >> number >> nd.x >> nd.y >> nd.demand >> nd.earliestStart >> nd.latestStart >> nd.serviceTime;
    if (in.good()) {
      if (number == 0)
        _depot = _data.getSize();
      _data.add(nd);
    }
  } while (in.good());
}

// Represents the data structures of the _model.  There are
// 2*numVehicles + numCustomers nodes.  The customer nodes are
// guaranteed to correspond to the CVRPTW class (0..nbCustomers-1).
// Indices of the start and end nodes are found
// using getFirst(vehicle), getLast(vehicle).
class VRP {
private:
  // Scaling factor for distances before rounding down
  const IloInt TIME_FACTOR = 10;

  const CVRPTWProblem & _problem;
  IloEnv                _env;
  IloModel              _model;

  IloArray<IloIntArray> _distance;

  IloIntVarArray        _prev;
  IloIntVarArray        _vehicle;
  IloIntVarArray        _start;
  IloIntVarArray        _load;
  IloIntVar             _used;

  IloInt getNbCustomers() const { return _problem.getNbCustomers(); }
  IloInt getNbVehicles() const { return _problem.getNbVehicles(); }
  IloInt getNbNodes() const { return 2 * getNbVehicles() + getNbCustomers(); }

  void getXY(IloInt i, IloInt& x, IloInt& y) const {
    if (isCustomer(i)) { x = _problem.getX(i);     y = _problem.getY(i); }
    else               { x = _problem.getDepotX(); y = _problem.getDepotY(); }
  }
  IloInt getServiceTime(IloInt i) const {
    return isCustomer(i) ? TIME_FACTOR * _problem.getServiceTime(i) : 0;
  }
  IloInt getEarliestStart(IloInt i) const {
    assert(isCustomer(i));
    return TIME_FACTOR * _problem.getEarliestStart(i);
  }
  IloInt getLatestStart(IloInt i) const {
    assert(isCustomer(i));
    return TIME_FACTOR * _problem.getLatestStart(i);
  }
  IloInt getMaxHorizon() const { return TIME_FACTOR * _problem.getMaxHorizon(); }

  IloInt getFirst(IloInt veh) const { return veh + _problem.getNbCustomers(); }
  IloInt getLast(IloInt veh) const { return veh + _problem.getNbVehicles() + _problem.getNbCustomers(); }
  IloBool isCustomer(IloInt i) const { return i < _problem.getNbCustomers(); }

  IloIntExpr arrivalTime(IloInt to) const {
    IloIntExprArray all(_model.getEnv());
    for (IloInt from = 0; from < _start.getSize(); from++)
      all.add(_start[from] + (getServiceTime(from) + _distance[to][from]));
    return all[_prev[to]];
  }

  void buildDistance();
  void buildVariables();
  void buildStructure();
  void enforceLoad();
  void enforceTimes();
  IloNumExpr getTotalDistance();

  void NameVars(IloIntVarArray x, const char * fmt) {
    for (IloInt i = 0; i < x.getSize(); i++) {
      char name[100];
      snprintf(name, sizeof(name), fmt, i);
      x[i].setName(name);
    }
  }


public:
  VRP(IloEnv env, const CVRPTWProblem& prob);
  IloNum solve(IloNum tlim);
};

VRP::VRP(IloEnv env, const CVRPTWProblem& prob)
  : _problem(prob)
  , _env(env)
  , _model(env) {
  buildVariables();
  buildStructure();
  enforceLoad();
  buildDistance();
  enforceTimes();
  IloNumExpr obj = getTotalDistance();
  _model.add(IloMinimize(env, obj));
}

void VRP::buildDistance() {
  IloInt n = getNbNodes();
  _distance = IloArray<IloIntArray>(_env, n);
  for (IloInt to = 0; to < n; to++) {
    _distance[to] = IloIntArray(_env, n);
    IloInt tox, toy; getXY(to, tox, toy);
    for (IloInt from = 0; from < n; from++) {
      IloInt fromx, fromy; getXY(from, fromx, fromy);
      IloInt dx = fromx - tox;
      IloInt dy = fromy - toy;
      IloInt d = (IloInt)floor(TIME_FACTOR * sqrt(dx*dx + dy*dy));
      _distance[to][from] = d;
    }
  }
}

void VRP::buildVariables() {
  IloInt n = getNbNodes();
  IloInt nVeh = getNbVehicles();
  IloInt nCust = getNbCustomers();
  _vehicle = IloIntVarArray(_env, n, 0, nVeh - 1);
  _start = IloIntVarArray(_env, n, 0, getMaxHorizon());
  _load = IloIntVarArray(_env, nVeh, 0, _problem.getCapacity());
  _used = IloIntVar(_env, 0, nVeh, "Used");
  _prev = IloIntVarArray(_env, n, 0, n - 1);
  NameVars(_vehicle, "V_%ld");
  NameVars(_start, "ST_%ld");
  NameVars(_load, "L_%ld");
  NameVars(_prev, "P_%ld");

  // All values inferred except 'prev'
  for (IloInt c = 0; c < nCust; c++) 
    _start[c].setUB(getLatestStart(c));
  _model.add(IloInferred(_env, _vehicle));
  _model.add(IloInferred(_env, _start));
  _model.add(IloInferred(_env, _load));
  _model.add(IloInferred(_env, _used));
}

void VRP::buildStructure() {
  IloInt nVeh = getNbVehicles();
  IloInt nCust = getNbCustomers();
  IloIntArray domain(_env);
  for (IloInt c = 0; c < nCust; c++)
    domain.add(c);
  for (IloInt v = 0; v < nVeh; v++) {
    IloInt f = getFirst(v);
    IloInt l = getLast(v);
    // First points to last of previous vehicle
    _model.add(_prev[f] == getLast((v + nVeh - 1) % nVeh));

    // Last can point it its first or any customer
    domain.add(f);
    _model.add(IloAllowedAssignments(_env, _prev[l], domain));
    domain.remove(nCust);

    // Vehicles of first, last or just before last
    _model.add(_vehicle[f] == v);
    _model.add(_vehicle[l] == v);
    _model.add(_vehicle[_prev[l]] == v);
  }
  // Make domain consist of all customers and all first nodes
  for (IloInt v = 0; v < nVeh; v++)
    domain.add(getFirst(v));

  for (IloInt c = 0; c < nCust; c++) {
    _model.add(IloAllowedAssignments(_env, _prev[c], domain));
    _model.add(_prev[c] != c); // no customers are optional
    _model.add(_vehicle[c] == _vehicle[_prev[c]]);
  }
  _model.add(IloSubCircuit(_env, _prev));
}

void VRP::enforceLoad() {
  IloInt nCust = getNbCustomers();
  IloIntArray demand(_env);
  IloIntVarArray custVeh(_env);
  for (IloInt c = 0; c < nCust; c++) {
    demand.add(_problem.getDemand(c));
    custVeh.add(_vehicle[c]);
  }
  _model.add(IloPack(_env, _load, custVeh, demand, _used));
}

IloNumExpr VRP::getTotalDistance() {
  IloInt nVeh = getNbVehicles();
  IloInt nCust = getNbCustomers();
  IloIntExpr totalDistance(_env);
  for (IloInt c = 0; c < nCust; c++)
    totalDistance += _distance[c][_prev[c]];
  for (IloInt v = 0; v < nVeh; v++) {
    IloInt l = getLast(v);
    totalDistance += _distance[l][_prev[l]];
  }
  return totalDistance / TIME_FACTOR;
}

void VRP::enforceTimes() {
  IloInt nVeh = getNbVehicles();
  IloInt nCust = getNbCustomers();
  for (IloInt c = 0; c < nCust; c++)
    _model.add(_start[c] == IloMax(arrivalTime(c), getEarliestStart(c)));
  for (IloInt v = 0; v < nVeh; v++) {
    IloInt f = getFirst(v);
    IloInt l = getLast(v);
    _model.add(_start[f] == 0);
    _model.add(_start[l] == arrivalTime(l));
  }
}

IloNum VRP::solve(IloNum tlim) {
  IloCP cp(_model);
  cp.addKPI(_used, "Used");
  if (tlim > 0) {
    cp.setParameter(IloCP::TimeLimit, tlim);
  }
  cp.setParameter(IloCP::LogPeriod, 1000000);
  cp.setParameter(IloCP::SearchType, IloCP::Restart);
  IloNum obj = IloInfinity;
  if (cp.solve())
    obj = cp.getObjValue();
  cp.end();
  return obj;
}

int main(int argc, const char * argv[]) {
  const char * fname = "../../../examples/data/cvrptw_C101_25.data";
  IloNum tlim = 5.0;
  if (argc > 1)
    fname = argv[1];
  if (argc > 2)
    tlim = atof(argv[2]);

  std::ifstream in(fname);
  if (!in.good()) {
    std::cout << "Could not open " << fname << std::endl;
  }
  else {
    IloEnv env;
    try {
      CVRPTWProblem problem(env, in);
      in.close();
      VRP vrp(env, problem);
      IloNum obj = vrp.solve(tlim);
      if (obj != IloInfinity)
        std::cout << "Found a solution of distance = " << obj << std::endl;
      else
        std::cout << "No solution found" << std::endl;
    } catch (IloException &ex) {
      std::cout << "Caught: " << ex << std::endl;
    }
    env.end();
  }
  return 0;
}
