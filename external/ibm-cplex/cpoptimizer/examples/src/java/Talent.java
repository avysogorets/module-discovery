// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Talent.java
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

This example is inspired from the talent hold cost scheduling problem
described in:

T.C.E Cheng, J. Diamond, B.M.T. Lin.  Optimal scheduling in film
production to minimize talent holding cost.  Journal of Optimization
Theory and Applications, 79:197-206, 1993.

of which the 'Rehearsal problem' is a specific case:

Barbara M. Smith.  Constraint Programming In Practice: Scheduling
a Rehearsal.  Report APES-67-2003, September 2003.

See: http://www.csplib.org/Problems/prob039/

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.ArrayList;

public class Talent {
    private static class DataReader {
        private StreamTokenizer st;
        public DataReader(String filename) throws IOException {
             FileInputStream fstream = new FileInputStream(filename);
             Reader r = new BufferedReader(new InputStreamReader(fstream));
             st = new StreamTokenizer(r);
         }
         public int next() throws IOException {
             st.nextToken();
             return (int)st.nval;
         }
    }

    public static void main(String[] args) throws IOException {
        IloCP cp = null;
        try {
            String fileName = "../../../examples/data/rehearsal.data";
            if (args.length > 1)
                fileName = args[1];

            DataReader data = new DataReader(fileName);
            cp = new IloCP();

            int numActors = data.next();
            int[] actorPay = new int[numActors];
            for (int a = 0; a < numActors; a++)
                actorPay[a] = data.next();

            int numScenes = data.next();
            int[] sceneDuration = new int[numScenes];
            for (int s = 0; s < numScenes; s++)
                sceneDuration[s] = data.next();

            boolean inScene[][] = new boolean[numActors][numScenes];
            for (int a = 0; a < numActors; a++) {
                for (int s = 0; s < numScenes; s++)
                    inScene[a][s] = data.next() == 0 ? false : true;
            }

            // Create the decision variables, cost, and the model
            IloIntVar[] scene = new IloIntVar[numScenes];
            for (int s = 0; s < numScenes; s++)
                scene[s] = cp.intVar(0, numScenes - 1, "SC[" + s + "]");

            // Expression representing the global cost
            IloIntExpr idleCost = cp.intExpr();

            // Make the slot-based secondary model
            IloIntVar[] slot = new IloIntVar[numScenes];
            for (int s = 0; s < numScenes; s++)
                slot[s] = cp.intVar(0, numScenes - 1, "SL[" + s + "]");
            cp.add(cp.inverse(scene, slot));

            // Loop over all actors, building cost
            for (int a = 0; a < numActors; a++) {
                // Expression for the waiting time for this actor
                IloIntExpr actorWait = cp.intExpr();

                // Calculate the first and last slots where this actor plays
                ArrayList<IloIntVar> position = new ArrayList<IloIntVar>();

                for (int s = 0; s < numScenes; s++)
                    if (inScene[a][s])
                        position.add(slot[s]);
                IloIntVar[] pos = position.toArray(new IloIntVar[position.size()]);

                IloIntExpr firstSlot = cp.min(pos);
                IloIntExpr lastSlot = cp.max(pos);

                // If an actor is not in a scene, s/he waits
                // if s/he is on set when the scene is filmed
                for (int s = 0; s < numScenes; s++) {
                    if (!inScene[a][s]) { // not in scene
                        IloIntExpr wait = cp.and(
                            cp.le(firstSlot, slot[s]),
                            cp.le(slot[s], lastSlot)
                        );
                        actorWait = cp.sum(
                            actorWait,
                            cp.prod(sceneDuration[s], wait)
                        );
                    }
                }

                // Accumulate the cost of waiting time for this actor
                idleCost = cp.sum(idleCost, cp.prod(actorPay[a], actorWait));
            }
            cp.add(cp.minimize(idleCost));

            cp.solve();
            System.out.println(
                "Solution of idle cost " +
                (int)cp.getValue(idleCost)
            );
            System.out.print("Order:");
            for (int s = 0; s < numScenes; s++)
                System.out.print(" " + ((int)cp.getValue(scene[s]) + 1));
            System.out.println();

            // Give more detailed information on the schedule
            for (int a = 0; a < numActors; a++) {
                System.out.print("|");
                for (int s = 0; s < numScenes; s++) {
                    int sc = (int)cp.getValue(scene[s]);
                    for (int d = 0; d < sceneDuration[sc]; d++) {
                        if (inScene[a][sc])
                            System.out.print("X");
                        else
                            System.out.print(".");
                    }
                    System.out.print("|");
                }
                System.out.println("  (Rate = " + actorPay[a] + ")");
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
