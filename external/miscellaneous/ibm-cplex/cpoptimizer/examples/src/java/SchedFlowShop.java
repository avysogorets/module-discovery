// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/SchedFlowShop.java
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

This problem is a special case of Job-Shop Scheduling problem (see
SchedJobShop.java) for which all jobs have the same processing order
on machines because there is a technological order on the machines for
the different jobs to follow.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;

public class SchedFlowShop {

    static class DataReader {

        private StreamTokenizer st;

        public DataReader(String filename) throws IOException {
            FileInputStream fstream = new FileInputStream(filename);
            Reader r = new BufferedReader(new InputStreamReader(fstream));
            st = new StreamTokenizer(r);
        }

        public int next() throws IOException {
            st.nextToken();
            return (int) st.nval;
        }
    }

    public static void main(String[] args) throws IOException {
        String filename = "../../../examples/data/flowshop_default.data";
        int failLimit = 10000;
        int nbJobs, nbMachines;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        DataReader data = new DataReader(filename);
        IloCP cp = null;
        try {
            cp = new IloCP();
            nbJobs = data.next();
            nbMachines = data.next();

            IloIntervalVar[][] mach = new IloIntervalVar[nbMachines][nbJobs];
            IloIntExpr[] ends = new IloIntExpr[nbJobs];
            for (int i = 0; i < nbJobs; i++) {
                IloIntervalVar prec = null;
                for (int j = 0; j < nbMachines; j++) {
                    int d = data.next();
                    String name = "Op[" + i + "][" + (j + 1) + "]";
                    IloIntervalVar ti = cp.intervalVar(d, name);
                    mach[j][i] = ti;
                    if (j > 0) {
                        cp.add(cp.endBeforeStart(prec, ti));
                    }
                    prec = ti;
                }
                ends[i] = cp.endOf(prec);
            }
            for (int j = 0; j < nbMachines; j++)
                cp.add(cp.noOverlap(mach[j]));

            IloObjective objective = cp.minimize(cp.max(ends));
            cp.add(objective);

            cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
            System.out.println("Instance \t: " + filename);
            if (cp.solve()) {
                System.out.println("Makespan \t: " + (int)cp.getObjValue());
            } else {
                System.out.println("No solution found.");
            }
        }
        catch (IloException e) {
            System.err.println("Error: " + e);
        }
        finally {
            if (cp != null)
                cp.end();
        }
    }
}
