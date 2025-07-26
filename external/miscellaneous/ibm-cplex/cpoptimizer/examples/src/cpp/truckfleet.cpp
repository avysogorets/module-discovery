// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/truckfleet.cpp
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
The problem is to deliver some orders to several clients with a single truck.
Each order consists of a given quantity of a product of a certain type (called
its color).
The truck must be configured in order to handle one, two or three different
colors of products.  The cost for configuring the truck from a configuration A
to a configuration B depends on A and B.  The configuration of the truck
determines its capacity and its loading cost.  A truck can only be loaded with
orders for the same customer.  Both the cost (for configuring and loading the truck)
and the number of travels needed to deliver all the orders must be minimized, the
cost being the most important criterion.

------------------------------------------------------------ */

#include <ilcp/cp.h>
#include "util.h"

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);
    //
    // Data on trucks
    //
    const IloInt nbTrucks       = 15; // max number of travels of the truck
    const IloInt nbTruckConfigs = 7; // number of possible configs of the truck
    // Capacity of the truck depends on its config
    IloIntArray truckCap(env, nbTruckConfigs, 11, 11, 11, 11, 10, 10, 10);
    // Cost for loading a truck of a given config
    IloIntArray truckCost(env, nbTruckConfigs, 2, 2, 2, 3, 3, 3, 4);
    // Transition costs between truck configurations
    IloInt costMatrix[][nbTruckConfigs] = {
      { 0, 0, 0, 10, 10, 10, 15 }, // 0 -> X
      { 0, 0, 0, 10, 10, 10, 15 }, // 1 -> X
      { 0, 0, 0, 10, 10, 10, 15 }, // 2 -> X
      { 3, 3, 3,  0, 10, 10, 15 }, // 3 -> X
      { 3, 3, 3,  10, 0, 10, 15 }, // 4 -> X
      { 3, 3, 3,  10, 10, 0, 15 }, // 5 -> X
      { 3, 3, 3,  10, 10, 10, 0 }, // 6 -> X
    };

    //
    // Data on orders
    //
    const IloInt nbOrders = 21;
    IloIntArray volumes(env, nbOrders, 3, 4, 3, 2, 5, 4, 11, 4, 5, 2,
                                       4, 7, 3, 5, 2, 5, 6, 11, 1, 6, 3);
    IloIntArray colors(env, nbOrders, 1, 2, 0, 1, 1, 1, 0, 0, 0, 0,
                                      2, 2, 2, 0, 2, 1, 0, 2, 0, 0, 0);

    // Color of an order defines which configurations are allowed (4 each)
    IloArray<IloIntArray> allowedContainerConfigs(env, 3);
    allowedContainerConfigs[0] = IloIntArray(env, 4, 0, 3, 4, 6);
    allowedContainerConfigs[1] = IloIntArray(env, 4, 1, 3, 5, 6);
    allowedContainerConfigs[2] = IloIntArray(env, 4, 2, 4, 5, 6);

    //
    // Data on customers (relates to orders)
    //
    IloIntArray customerOfOrder(env, nbOrders, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                               1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2);

    //
    // Decision variables
    //
    IloIntVarArray truckConfigs(env, nbTrucks, 0, nbTruckConfigs-1); //configuration of the truck
    NameVars(truckConfigs, "C");
    IloIntVarArray where(env, nbOrders, 0, nbTrucks-1); //In which truck is an order
    NameVars(where, "W");
    IloIntVarArray load(env, nbTrucks, 0, IloMax(truckCap)); //load of a truck
    NameVars(load, "L");
    IloIntVar numUsed(env, 0, nbTrucks, "Used"); // number of trucks used
    IloIntVarArray customerOfTruck(env, nbTrucks, 0, IloMax(customerOfOrder));
    NameVars(customerOfTruck, "CT");

    //
    // Transition costs
    //
    IloIntTupleSet costTuples(env, 3);
    IloInt maxCost = 0;
    for (IloInt i = 0; i < nbTruckConfigs; i++) {
      for (IloInt j = 0; j < nbTruckConfigs; j++) {
        costTuples.add(IloIntArray(env, 3, i, j, costMatrix[i][j]));
        maxCost = IloMax(maxCost, costMatrix[i][j]);
      }
    }
    IloIntVarArray transitionCost(env, nbTrucks-1, 0, maxCost);
    NameVars(transitionCost, "TC");
    for (IloInt i = 1; i < nbTrucks; i++) {
      model.add(IloAllowedAssignments(
        env, truckConfigs[i-1], truckConfigs[i], transitionCost[i-1], costTuples
      ));
    }

    //
    // Respect the truck capacity (maintain the number of trucks used too)
    //
    model.add(IloPack(env, load, where, volumes, numUsed));
    for (IloInt i = 0; i < nbTrucks; i++)
      model.add(load[i] <= truckCap[truckConfigs[i]]);

    //
    // Compatibility between the colors of an order and the configuration of its truck
    //
    for (IloInt j = 0; j < nbOrders; j++)
      model.add(IloAllowedAssignments(
        env, truckConfigs[where[j]], allowedContainerConfigs[colors[j]])
      );

    //
    // All orders on a truck are for a single customer
    //
    for (IloInt j = 0; j < nbOrders; j++)
      model.add(customerOfTruck[where[j]] == customerOfOrder[j]);

    //
    // Symmetry and dominance constraints
    //

    // All the used trucks should have the smaller indices
    IloConstraintArray used(env);
    for (IloInt i = 0; i < nbTrucks; i++) {
      used.add(i < numUsed);
      model.add(used[i] == (load[i] > 0));
    }

    // Non-used trucks.  The constraints above will not force a
    // customer or a configuration on an unused truck, so we do this here.
    for (IloInt i = 0; i < nbTrucks; i++)
      model.add(used[i] || (truckConfigs[i] == 0 && customerOfTruck[i] == 0));

    // Dominance. Changing to or from a configuration should be done only once.
    // since this is always better than an alternative.
    for (IloInt i = 1; i < nbTrucks-1; i++) {
      IloConstraint sameHere = truckConfigs[i-1] == truckConfigs[i];
      IloAnd noneRight(env);
      for (IloInt j = i + 1; j < nbTrucks; j++)
        noneRight.add(truckConfigs[i-1] != truckConfigs[j]);
      model.add(sameHere || noneRight);
    }

    // Symmetry of truck slots.  The slots are sensitive to the configuration
    // because of the cost of reconfiguration, but not to the customer or load.
    for (IloInt i = 1; i < nbTrucks; i++) {
      // If we keep the same config, we want to break symmetry on the
      // customer of the truck and then upon the load.  Larger values of
      // customer and load at lower indices is compatible with the customer
      // and load being zero for the empty vehicles (which are at higher indices).
      IloIntExprArray signature1(env); signature1.add(customerOfTruck[i-1]); signature1.add(load[i-1]);
      IloIntExprArray signature2(env); signature2.add(customerOfTruck[i]); signature2.add(load[i]);
      model.add(IloIfThen(env, truckConfigs[i-1] == truckConfigs[i],
                               IloLexicographic(env, signature2, signature1)));
    }

    //
    // Objective: first criterion for minimizing the cost for configuring and loading trucks
    //            second criterion for minimizing the number of trucks
    //
    IloExpr obj1 = IloSum(transitionCost);
    for (IloInt i = 0; i < nbTrucks; i++)
      obj1 += truckCost[truckConfigs[i]] * used[i];

    IloIntExpr obj2 = numUsed;
    IloNumExprArray objArray(env);
    objArray.add(obj1);
    objArray.add(obj2);

    // Multicriteria lexicographic optimization
    model.add(IloMinimize(env, IloStaticLex(env, objArray)));
    IloCP cp(model);
    cp.setParameter(IloCP::LogPeriod, 50000);
    cp.solve();

    // Display solution
    cp.out() << "Configuration cost: " << cp.getValue(obj1)
             << " Number of Trucks: " << cp.getValue(obj2) <<std::endl;
    for (IloInt i = 0; i < nbTrucks; i++) {
      if (cp.getValue(load[i]) > 0) {
        cp.out() << "Truck " << i
                 << ": Config = " << cp.getValue(truckConfigs[i])
                 << " Items = ";
        for (IloInt j = 0; j < nbOrders; j++) {
          if (cp.getValue(where[j]) == i)
            cp.out() << "<" << j << "," << colors[j] << "," << volumes[j] << "> ";
        }
        cp.out() << std::endl;
      }
    }
    cp.end();
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}

