// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Callbacks.java
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

This example demonstrates the use of a callback to deliver real-time
information on the lower and upper bounds of the objective function,
and the gap.

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Callbacks {
    static public class BoundsCallback implements IloCP.Callback {
        private double  _lb;
        private double  _ub;
        private double  _gap;
        public BoundsCallback() { init(); }
        public void init() {
            _lb  = Double.NEGATIVE_INFINITY;
            _ub  = Double.POSITIVE_INFINITY;
            _gap = Double.POSITIVE_INFINITY;
        }
        public void invoke(IloCP cp, IloCP.Callback.Reason reason) {
            try {
                // Initialize the local fields at the beginning of search,
                // and print the banner
                if (reason == IloCP.Callback.Reason.StartSolve) {
                    init();
                    System.out.println("Time\tLB\tUB\tGap");
                    System.out.println("=============================");
                }
                else if (reason == IloCP.Callback.Reason.EndSolve) {
                    // Finish the line at the end of search
                    System.out.println();
                }
                else {
                    boolean soln = (reason == IloCP.Callback.Reason.Solution);
                    boolean bnd = (reason == IloCP.Callback.Reason.ObjBound);
                    if (soln || bnd) {
                        // Write a newline if a line has already been written
                        if (_gap != Double.POSITIVE_INFINITY)
                            System.out.println();
                        if (soln) {
                            // Update upper bound and gap (solution found)
                            _ub = cp.getObjValue();
                            if (_lb > Double.NEGATIVE_INFINITY)
                                _gap = cp.getObjGap();
                        }
                        else if (bnd) {
                            // Update lower bound and gap (new bound)
                            _lb = cp.getObjBound();
                            if (_ub < Double.POSITIVE_INFINITY)
                                _gap = cp.getObjGap();
                        }
                    }
                    if (_gap != Double.POSITIVE_INFINITY) {
                        // Both a bound and a solution.
                        System.out.format(
                            "\r                                  " +
                            "                \r%.1f\t%.0f\t%.0f\t%.1f%%",
                            cp.getInfo(IloCP.DoubleInfo.SolveTime),
                            _lb, _ub, 100 * _gap
                        );
                        System.out.flush();
                    }
                }
            }
            catch (IloException ex) {
                System.err.println(
                    "FATAL: Exception encountered during callback " + ex
                );
                System.exit(-1);
            }
        }
    };

    static void SolveWithCallback(IloCP cp) throws IloException {
        // Add callback, solve, then remove it.
        BoundsCallback cb = new Callbacks.BoundsCallback();
        cp.addCallback(cb);
        cp.solve();
        cp.removeCallback(cb);
    }

    public static void main(String[] args) {
        IloCP cp = null;
        try {
            // Load a model, and solve using a callback.
            cp = new IloCP();
            cp.importModel("../../../examples/data/sched_jobshop.cpo");
            cp.setParameter(
                IloCP.IntParam.LogVerbosity,
                IloCP.ParameterValues.Quiet
            );
            SolveWithCallback(cp);
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
};
