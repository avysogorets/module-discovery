// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Truckfleet.java
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
The truck must be configured in order to handle one, two or three different colors of products.
The cost for configuring the truck from a configuration A to a configuration B depends on A and B.
The configuration of the truck determines its capacity and its loading cost.
A truck can only be loaded with orders for the same customer.
Both the cost (for configuring and loading the truck) and the number of travels needed to deliver all the
orders must be minimized, the cost being the most important criterion.

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Truckfleet {
    public static void main(String[] args) {
        IloCP cp = null;
        try {
            cp = new IloCP();
            //
            // Data on trucks
            //
            int nbTrucks = 15; // max number of travels of the truck
            // Capacity of the truck depends on its config
            int [] truckCap = { 11, 11, 11, 11, 10, 10, 10 };
            // Cost for loading a truck of a given config
            int[] truckCost = { 2, 2, 2, 3, 3, 3, 4 };
            int[][] costMatrix = {
                { 0, 0, 0, 10, 10, 10, 15 }, // 0 -> X
                { 0, 0, 0, 10, 10, 10, 15 }, // 1 -> X
                { 0, 0, 0, 10, 10, 10, 15 }, // 2 -> X
                { 3, 3, 3,  0, 10, 10, 15 }, // 3 -> X
                { 3, 3, 3,  10, 0, 10, 15 }, // 4 -> X
                { 3, 3, 3,  10, 10, 0, 15 }, // 5 -> X
                { 3, 3, 3,  10, 10, 10, 0 }, // 6 -> X
            };
            int nbTruckConfigs = truckCap.length;

            //
            // Data on orders
            //
            int[] volumes = { 3, 4, 3, 2, 5, 4, 11, 4, 5, 2,
                              4, 7, 3, 5, 2, 5, 6, 11, 1, 6, 3 };
            int[] colors = { 1, 2, 0, 1, 1, 1, 0, 0, 0, 0,
                             2, 2, 2, 0, 2, 1, 0, 2, 0, 0, 0 };
            int nbOrders = volumes.length;

            // Color of an order defines which configurations are allowed (4 each)
            int[][] allowedContainerConfigs = {
              new int[] {0, 3, 4, 6},
              new int[] {1, 3, 5, 6},
              new int[] {2, 4, 5, 6}
            };

            //
            // Data on customers (relates to orders)
            //
            int nbCustomers = 3;
            int[] customerOfOrder = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                      1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2 };

            // Decision variables
            IloIntVar[] truckConfigs = cp.intVarArray(nbTrucks, 0, nbTruckConfigs-1, "C"); // Configuration of the truck
            IloIntVar[] where = cp.intVarArray(nbOrders, 0, nbTrucks - 1, "W"); // In which truck is an order
            IloIntVar[] load = cp.intVarArray(nbTrucks, 0, cp.IntMax, "L"); // Load of a truck
            IloIntVar numUsed = cp.intVar(0, nbTrucks, "U"); // Number of trucks used
            IloIntVar[] customerOfTruck = cp.intVarArray(nbTrucks, 0, nbCustomers-1, "CT");

            //
            // Transition costs
            //
            IloIntTupleSet costTuples = cp.intTable(3);
            //int maxCost = 0;
            for (int i = 0; i < nbTruckConfigs; i++) {
                for (int j = 0; j < nbTruckConfigs; j++) {
                    int cst = costMatrix[i][j];
                    cp.addTuple(costTuples, new int[] {i, j, cst});
                }
            }
            IloIntVar[] transitionCost = cp.intVarArray(nbTrucks-1, 0, cp.IntMax, "TC");
            for (int i = 1; i < nbTrucks; i++) {
                cp.add(cp.allowedAssignments(
                    new IloIntVar[] { truckConfigs[i-1], truckConfigs[i], transitionCost[i-1] },
                    costTuples)
                );
            }

            //
            // Respect the truck capacity (maintain the number of trucks used too)
            //
            cp.add(cp.pack(load, where, volumes, numUsed));
            for (int i = 0; i < nbTrucks; i++)
                cp.add(cp.le(load[i], cp.element(truckCap, truckConfigs[i])));

            //
            // Compatibility between the colors of an order and the configuration of its truck
            //
            for (int j = 0; j < nbOrders; j++)
                cp.add(cp.allowedAssignments(
                    cp.element(truckConfigs, where[j]),
                    allowedContainerConfigs[colors[j]]
                ));

            //
            // All orders on a truck are for a single customer
            //
            for (int j = 0; j < nbOrders; j++)
                cp.add(cp.eq(cp.element(customerOfTruck, where[j]), customerOfOrder[j]));

            //
            // Symmetry and dominance constraints
            //
           
            // All the used trucks should have the smaller indices
            IloConstraint[] used = new IloConstraint[nbTrucks];
            for (int i = 0; i < nbTrucks; i++) {
                used[i] = cp.lt(i, numUsed);
                cp.add(cp.eq(used[i], cp.gt(load[i], 0)));
            }

            // Non-used trucks.  The constraints above will not force a
            // customer or a configuration on an unused truck, so we do this here.
            for (int i = 0; i < nbTrucks; i++)
                cp.add(cp.or(used[i], cp.and(cp.eq(truckConfigs[i], 0), cp.eq(customerOfTruck[i], 0))));

            // Dominance. Changing to or from a configuration should be done only once.
            // since this is always better than an alternative.
            for (int i = 1; i < nbTrucks-1; i++) {
                IloConstraint sameHere = cp.eq(truckConfigs[i-1], truckConfigs[i]);
                IloAnd noneRight = cp.and();
                for (int j = i + 1; j < nbTrucks; j++)
                    noneRight.add(cp.neq(truckConfigs[i-1], truckConfigs[j]));
                cp.add(cp.or(sameHere, noneRight));
            }
            // Symmetry of truck slots.  The slots are sensitive to the configuration
            // because of the cost of reconfiguration, but not to the customer or load.
            for (int i = 1; i < nbTrucks; i++) {
                // If we keep the same config, we want to break symmetry on the
                // customer of the truck and then upon the load.  Larger values of
                // customer and load at lower indices is compatible with the customer
                // and load being zero for the empty vehicles (which are at higher indices).
                IloIntExpr[] signature1 = new IloIntExpr[] { customerOfTruck[i-1], load[i-1] };
                IloIntExpr[] signature2 = new IloIntExpr[] { customerOfTruck[i], load[i] };
                cp.add(cp.ifThen(cp.eq(truckConfigs[i-1], truckConfigs[i]),
                                 cp.lexicographic(signature2, signature1)));
            }
            //
            // Objective: first criterion for minimizing the cost for configuring and loading trucks
            //            second criterion for minimizing the number of trucks
            //
            IloIntExpr obj1 = cp.sum(transitionCost);
            for (int i = 0; i < nbTrucks; i++)
                obj1 = cp.sum(obj1, cp.prod(cp.element(truckCost, truckConfigs[i]), used[i]));
            IloIntExpr obj2 = numUsed;
        
            // Multicriteria lexicographic optimization
            cp.add(cp.minimize(cp.staticLex(obj1, obj2)));
            cp.setParameter(IloCP.IntParam.LogPeriod, 50000);
            cp.solve();

            // Display solution
            System.out.println(
                "Configuration cost: " + (int)cp.getValue(obj1)
              + " Number of Trucks: " + (int)cp.getValue(obj2)
            );
            for (int i = 0; i < nbTrucks; i++) {
                if (cp.getValue(load[i]) > 0) {
                    System.out.print(
                        "Truck " +  i + ": Config = " +
                        cp.getIntValue(truckConfigs[i]) + " Items = "
                    );
                    for (int j = 0; j < nbOrders; j++) {
                        if (cp.getValue(where[j]) == i)
                            System.out.print("<" + j + "," + colors[j] + "," + volumes[j] + "> ");
                    }
                    System.out.println();
                }
            }
        }
        catch (IloException e) {
            System.out.println("Error : " + e);
            e.printStackTrace();
        }
        finally {
            if (cp != null)
                cp.end();
        }
    }
}
