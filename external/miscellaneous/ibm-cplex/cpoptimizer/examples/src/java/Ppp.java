// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Ppp.java
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

For a description of the problem and resolution methods:

        The Progressive Party Problem: Integer Linear Programming
        and Constraint Programming Compared

        Proceedings of the First International Conference on Principles
        and Practice of Constraint Programming table of contents

        Lecture Notes In Computer Science; Vol. 976, pages 36-52, 1995
        ISBN:3-540-60299-2

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Ppp {
    public static void main(String[] args) {
        IloCP cp = null;
        try {
            cp = new IloCP();

            //
            // Data
            //
            int numBoats = 42;
            int[] boatSize = {
                7, 8, 12, 12, 12, 12, 12, 10, 10, 10,
                10, 10, 8, 8, 8, 12, 8, 8, 8, 8,
                8, 8, 7, 7, 7, 7, 7, 7, 6, 6,
                6, 6, 6, 6, 6, 6, 6, 6, 9, 2,
                3, 4
            };
            int[] crewSize = {
                2, 2, 2, 2, 4, 4, 4, 1, 2, 2,
                2, 3, 4, 2, 3, 6, 2, 2, 4, 2,
                4, 5, 4, 4, 2, 2, 4, 5, 2, 4,
                2, 2, 2, 2, 2, 2, 4, 5, 7, 2,
                3, 4
            };
            int numPeriods = 6;
            if (args.length > 0)
                numPeriods = Integer.parseInt(args[0]);

            //
            // Variables
            //

            // Host boat choice
            IloIntVar[] host = cp.intVarArray(numBoats, 0, 1, "H");

            // Who is where each time period
            IloIntVar[][] visits = new IloIntVar[numBoats][];
            for (int i = 0; i < numBoats; i++) {
                visits[i] = cp.intVarArray(
                    numPeriods, 0, numBoats - 1, cp.arrayEltName("V", i)
                );
            }

            //
            // Objective
            //
            IloIntExpr numHosts = cp.sum(host);
            cp.add(cp.minimize(numHosts));

            //
            // Constraints
            //

            // Stay in my boat (host) or only visit other boats (guest)
            for (int i = 0; i < numBoats; i++)
                cp.add(cp.eq(
                    cp.count(visits[i], i),
                    cp.prod(host[i], numPeriods)
                ));

            // Capacity constraints: only hosts have capacity
            for (int p = 0; p < numPeriods; p++) {
                IloIntVar[] load = new IloIntVar[numBoats];
                IloIntVar[] timePeriod = new IloIntVar[numBoats];
                for (int i = 0; i < numBoats; i++) {
                    load[i] = cp.intVar(
                        0, boatSize[i], "L[" + p + "][" + i + "]"
                    );
                    timePeriod[i] = visits[i][p];
                    cp.add(cp.le(load[i], cp.prod(host[i], boatSize[i])));
                }
                cp.add(cp.pack(load, timePeriod, crewSize, numHosts));
            }

            // No two crews meet more than once
            for (int i = 0; i < numBoats; i++) {
                for (int j = i + 1; j < numBoats; j++) {
                    IloIntExpr timesMet = cp.constant(0);
                    for (int p = 0; p < numPeriods; p++)
                        timesMet = cp.sum(
                            timesMet,
                            cp.eq(visits[i][p], visits[j][p])
                        );
                    cp.add(cp.le(timesMet, 1));
                }
            }

            // Host and guest boat constraints: given in problem spec
            cp.add(cp.eq(host[0], 1));
            cp.add(cp.eq(host[1], 1));
            cp.add(cp.eq(host[2], 1));
            cp.add(cp.eq(host[39], 0));
            cp.add(cp.eq(host[40], 0));
            cp.add(cp.eq(host[41], 0));

            //
            // Solving
            //
            if (cp.solve()) {
                System.out.println(
                    "Solution at cost = " + (int)cp.getValue(numHosts)
                );
                System.out.print("Hosts: ");
                for (int i = 0; i < numBoats; i++)
                    System.out.print((int)cp.getValue(host[i]));
                System.out.println();
                for (int i = 0; i < numBoats; i++) {
                    System.out.print(
                        "Boat " + i + " (size = " + crewSize[i] + "):\t"
                    );
                    for (int p = 0; p < numPeriods; p++)
                        System.out.print((int)cp.getValue(visits[i][p]) + "\t");
                    System.out.println();
                }
                for (int p = 0; p < numPeriods; p++) {
                    System.out.println("Period " + p);
                    for (int h = 0; h < numBoats; h++) {
                        if (cp.getValue(host[h]) == 1) {
                            System.out.print("\tHost " + h + " : ");
                            int load = 0;
                            for (int i = 0; i < numBoats; i++) {
                                if (cp.getValue(visits[i][p]) == h) {
                                    load += crewSize[i];
                                    System.out.print(
                                        i + " (" + crewSize[i] + ") "
                                    );
                                }
                            }
                            System.out.println(
                                " --- " + load + " / " + boatSize[h]
                            );
                        }
                    }
                }
                System.out.println();
            }
            else
                System.out.println("No solution");
        }
        catch (IloException e) {
            System.err.println("Error " + e);
        }
        finally {
            if (cp != null)
                cp.end();
        }
    }
}
