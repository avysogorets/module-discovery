// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Alloc.java
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

Frequency assignment problem
----------------------------

Problem Description

The problem is given here in the form of discrete data; that is,
each frequency is represented by a number that can be called its
channel number.  For practical purposes, the network is divided
into cells (this problem is an actual cellular phone problem).
In each cell, there is a transmitter which uses different
channels.  The shape of the cells have been determined, as well
as the precise location where the transmitters will be
installed.  For each of these cells, traffic requires a number
of frequencies.

Between two cells, the distance between frequencies is given in
the matrix on the next page.

The problem of frequency assignment is to avoid interference.
As a consequence, the distance between the frequencies within a
cell must be greater than 16.  To avoid inter-cell interference,
the distance must vary because of the geography.

------------------------------------------------------------ */


import ilog.cp.*;
import ilog.concert.*;

public class Alloc {
    static final int nbAvailFreq = 256;
    static final int[] nbChannel = {
        8,6,6,1,4,4,8,8,8,8,4,9,8,4,4,10,8,9,8,4,5,4,8,1,1
    };
    static final int[][] dist = {
        { 16,1,1,0,0,0,0,0,1,1,1,1,1,2,2,1,1,0,0,0,2,2,1,1,1 },
        { 1,16,2,0,0,0,0,0,2,2,1,1,1,2,2,1,1,0,0,0,0,0,0,0,0 },
        { 1,2,16,0,0,0,0,0,2,2,1,1,1,2,2,1,1,0,0,0,0,0,0,0,0 },
        { 0,0,0,16,2,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
        { 0,0,0,2,16,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
        { 0,0,0,2,2,16,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
        { 0,0,0,0,0,0,16,2,0,0,1,1,1,0,0,1,1,1,1,2,0,0,0,1,1 },
        { 0,0,0,0,0,0,2,16,0,0,1,1,1,0,0,1,1,1,1,2,0,0,0,1,1 },
        { 1,2,2,0,0,0,0,0,16,2,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1 },
        { 1,2,2,0,0,0,0,0,2,16,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1 },
        { 1,1,1,0,0,0,1,1,2,2,16,2,2,2,2,2,2,1,1,2,1,1,0,1,1 },
        { 1,1,1,0,0,0,1,1,2,2,2,16,2,2,2,2,2,1,1,2,1,1,0,1,1 },
        { 1,1,1,0,0,0,1,1,2,2,2,2,16,2,2,2,2,1,1,2,1,1,0,1,1 },
        { 2,2,2,0,0,0,0,0,2,2,2,2,2,16,2,1,1,1,1,1,1,1,1,1,1 },
        { 2,2,2,0,0,0,0,0,2,2,2,2,2,2,16,1,1,1,1,1,1,1,1,1,1 },
        { 1,1,1,0,0,0,1,1,1,1,2,2,2,1,1,16,2,2,2,1,2,2,1,2,2 },
        { 1,1,1,0,0,0,1,1,1,1,2,2,2,1,1,2,16,2,2,1,2,2,1,2,2 },
        { 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,16,2,2,1,1,0,2,2 },
        { 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,16,2,1,1,0,2,2 },
        { 0,0,0,1,1,1,2,2,1,1,2,2,2,1,1,1,1,2,2,16,1,1,0,1,1 },
        { 2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,1,1,1,16,2,1,2,2 },
        { 2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,1,1,1,2,16,1,2,2 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,16,1,1 },
        { 1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,2,2,1,16,2 },
        { 1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,2,2,1,2,16 }
    };

    public static int getNumTransmitters() {
        int nt = 0;
        for (int i = 0; i < nbChannel.length; i++)
            nt += nbChannel[i];
        return nt;
    }

    public static int getCell(int t) {
        int c = 0;
        while (t >= nbChannel[c]) {
            t -= nbChannel[c];
            c++;
        }
        return c;
    }

    public static int getMinDistance(int t1, int t2) {
        return dist[getCell(t1)][getCell(t2)];
    }

    //
    // Find a bound based on a maximum clique problem.
    // We create a graph where an edge connects two vertices
    // if the two vertices represent transmitters which must
    // have different frequencies.  A clique if size k in this
    // graph means that at least k different frequencies must
    // be used.  Note that any clique provides a valid bound
    // and hence the search can be time bounded and still
    // provide a usable bound.
    //
    public static int maxCliqueBound() {
        IloCP cp = null;
        int lb = 1;
        try {
            cp = new IloCP();
            int nbTransmitters = getNumTransmitters();
            IloIntVar[] inClique = cp.intVarArray(nbTransmitters, 0, 1);
            for (int t2 = 1; t2 < nbTransmitters; t2++)
                for (int t1 = 0; t1 < t2; t1++)
                    if (getMinDistance(t1, t2) == 0)
                        cp.add(cp.eq(cp.min(inClique[t1], inClique[t2]), 0));
            cp.add(cp.maximize(cp.sum(inClique)));
            cp.setParameter(IloCP.DoubleParam.TimeLimit, 10);
            cp.setParameter(IloCP.IntParam.LogVerbosity, IloCP.ParameterValues.Quiet);
            boolean ok = cp.solve();
            if (ok) lb = (int)cp.getObjValue();
        }
        catch (IloException e) {
            System.err.println("Error: " + e);
        }
        finally {
            if (cp != null)
                cp.end();
        }
        return lb;
    }

    public static void main(String[] args) {
        IloCP cp = null;
        try {
            cp = new IloCP();
            int nbTransmitters = getNumTransmitters();
            IloIntVar[] freq = cp.intVarArray(
                nbTransmitters, 0, nbAvailFreq-1, "freq"
            );
            for (int t2 = 1; t2 < nbTransmitters; t2++) {
                for (int t1 = 0; t1 < t2; t1++) {
                    int d = getMinDistance(t1, t2);
                    if (d > 0) 
                        cp.add(cp.ge(cp.abs(cp.diff(freq[t1], freq[t2])), d));
                }
            }

            // Minimize the total number of frequencies
            IloIntExpr nbFreq = cp.countDifferent(freq);
            cp.add(cp.minimize(nbFreq));
            int lb = maxCliqueBound();
            cp.add(cp.ge(nbFreq, lb));

            if (cp.solve()) {
                for (int t = 0; t < nbTransmitters; t++) {
                    int c = getCell(t);
                    if (c > 0 && getCell(t-1) != c)
                        System.out.println();
                    System.out.print((int)cp.getValue(freq[t]) + " ");
                }
                System.out.println();
                System.out.println("Total # of sites       " + nbTransmitters);
                System.out.println("Total # of frequencies " + (int)cp.getValue(nbFreq));
            } else
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
