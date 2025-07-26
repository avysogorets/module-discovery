// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corporation 2021, 2024. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------

In the Capacitated Vehicle Routing Problem with Time Windows (CVRPTW),
a fleet of vehicles with identical limited capacity leave a depot to serve
a set of customers with a single commodity and then come back to the depot.
All customers have known demand and must be served by a single visit.
For each delivery, the vehicle must arrive during the customer preferred
time windows. A service time for delivery is also associated with each
customer.

The problem is to find a sequence of customers to be delivered by each
truck such that the total distance traveled by vehicles is minimized.

------------------------------------------------------------ */

using CP;

tuple Node {
  key int id;
  int x;
  int y;
  int demand;
  int begin;
  int end;
  int service;
}

int maxVehicles = ...;
int capacity = ...;
int depotx = ...;
int depoty = ...;
int horizon = ...;
{Node} custNode = ...;

int TIME_FACTOR = 10;
int n = 1 + max ( c in custNode ) c.id;

range C = 0 .. n - 1;
range N = 0 .. n + 2 * maxVehicles - 1;
range V = 0 .. maxVehicles - 1;
range SRCS = n .. n + maxVehicles - 1;
range SINKS = n + maxVehicles .. n + 2 * maxVehicles - 1;
{int} C_AND_SINKS = {i | i in C} union {i | i in SINKS};

int sourceOf[v in V] = n + v;
int sinkOf[v in V] = n + maxVehicles + v;
Node node[i in N] = ( i in C ) ? item ( custNode, < i > ) : < i, depotx, depoty,
   0, 0, TIME_FACTOR * horizon, 0 >;
// Travel time
int ttToNode[toNode in N, fromNode in N] = ftoi ( floor ( TIME_FACTOR * sqrt ( 
  ( node[toNode].x - node[fromNode].x ) ^ 2 + 
  ( node[toNode].y - node[fromNode].y ) ^ 2 ) ) );

dvar int previous[i in N] in N;
dvar int vehicle[i in N] in V;
dvar int load[i in V] in 0 .. capacity;
dvar int startServ[i in N] in TIME_FACTOR * node[i].begin 
   .. TIME_FACTOR * node[i].end;
dvar int endServ[i in N] in TIME_FACTOR * ( node[i].begin + node[i].service )
   .. TIME_FACTOR * ( node[i].end + node[i].service );
dvar int used in 1 .. maxVehicles;

execute {
  cp.addKPI(used, "Used");
  cp.param.TimeLimit = 10;
  cp.param.LogPeriod = 50000;
}

minimize
  ( sum ( i in C_AND_SINKS ) ttToNode[i][previous[i]] ) / TIME_FACTOR;
subject to {
  // All nodes in a big cycle
  subCircuit ( previous );
  forall ( i in N )
    previous[i] != i;
  // Vehicle sources and sinks
  forall ( v in V ) {
    previous[sourceOf[v]] == sinkOf[( v + 1 ) % maxVehicles];
    vehicle[sourceOf[v]] == v;
    vehicle[sinkOf[v]] == v;
    startServ[sourceOf[v]] == 0;
  }

  // Which vehicle that visits each customer.
  forall ( i in C )
    vehicle[i] == vehicle[previous[i]];

  // Load on a vehicle
  pack ( load, vehicle, all ( i in N ) node[i].demand );
  // Arrival time of a vehicle
  forall ( i in N ) {
    endServ[i] == startServ[i] + TIME_FACTOR * node[i].service;
  }
  forall ( i in C_AND_SINKS ) {
    startServ[i] == maxl ( TIME_FACTOR * node[i].begin,
       endServ[previous[i]] + ttToNode[i][previous[i]] );
  }

  // Number of vehicles used
  used == sum ( v in V ) ( previous[sinkOf[v]] != sourceOf[v] );
  // Help the search
  inferred ( vehicle );
  inferred ( load );
  inferred ( startServ );
  inferred ( endServ );
  inferred ( used );
}
