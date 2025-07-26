// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Kmeans.java
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

K-means
----------------------------

Problem Description

K-means is a way of clustering points in a multi-dimensional space
where the set of points to be clustered are partitioned into k subsets.
The idea is to minimize the inter-point distances inside a cluster in
order to produce clusters which group together close points.

See https://en.wikipedia.org/wiki/K-means_clustering

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.cp.IloCP.ParameterValues;

import java.util.ArrayList;
import java.util.List;

import ilog.concert.*;

public class Kmeans {
    static final int n = 500;
    static final int d = 2;
    static final int k = 5;
    static final long randomSeed = 0;

    private static List<List<Double>> createPointsCoords(int n, int d, long randomSeed) {
        java.util.Random rnd = new java.util.Random(randomSeed);
        List<List<Double>> coords = new ArrayList<List<Double>>();
        for (int i = 0; i < d; i++) {
            List<Double> points = new ArrayList<Double>();
            coords.add(points);
            for (int j = 0; j < n; j++)
                points.add(rnd.nextDouble());
        }
        return coords;
    }

    private static IloCP createModel(List<List<Double>> coords, int k, boolean trustNumerics) {
        IloCP cp = null;
        int d = coords.size();
        int n = coords.get(0).size();

        try {
            cp = new IloCP();

            IloIntVar[] x = cp.intVarArray(n, 0, k-1);
            for (int i = 0; i < n; i++)
                x[i].setName("C_" + i);

                IloIntExpr[] csize = new IloIntExpr[k];
                for (int c = 0; c < k; c++)
                    csize[c] = cp.max(1, cp.count(x, c));

            IloNumExpr totalDist2 = cp.constant(0);
            for (int c = 0; c < k; c++) {
                IloIntExpr[] included = new IloIntExpr[n];
                for (int i = 0; i < n; i++)
                    included[i] = cp.eq(x[i], c);
                for (int dim = 0; dim < d; dim++) {
                    List<Double> points = coords.get(dim);
                    IloNumExpr center = cp.constant(0);
                    for (int i = 0; i < n; i++)
                        center = cp.sum(center, cp.prod(included[i], points.get(i)));
                    center = cp.quot(center, csize[c]);
                    IloNumExpr dist2 = cp.constant(0);
                    if (trustNumerics) {
                        for (int i = 0; i < n; i++)
                            dist2 = cp.sum(dist2, cp.prod(included[i], Math.pow(points.get(i), 2)));
                        dist2 = cp.diff(dist2, cp.prod(cp.square(center), csize[c]));
                    } else {
                        for (int i = 0; i < n; i++)
                            dist2 = cp.sum(dist2, cp.prod(included[i], cp.square(cp.diff(center, points.get(i)))));
                    }
                    totalDist2 = cp.sum(totalDist2, dist2);
                }
            }
            // Minimizing the sum of the square of distances
            cp.add(cp.minimize(totalDist2));
        }
        catch (IloException e) {
            System.err.println("Error " + e);
            if (cp != null)
                cp.end();
        }
        return cp;
    }

    public static void main(String[] args) {
        IloCP cp = null;
        try {
            // Create data points
            List<List<Double>> coords = createPointsCoords(n, d, randomSeed);

            // Create model with trustNumerics = TRUE
            cp = createModel(coords, k, false);

            cp.setParameter(IloCP.DoubleParam.TimeLimit, 20);
            cp.setParameter(IloCP.IntParam.LogPeriod, 50000);

            // Search using "Restart" search type
            cp.setParameter(IloCP.IntParam.SearchType, ParameterValues.Restart);
            if (!cp.solve())
                System.out.println("No solution");

            // Search using "Neighborhood" search type
            cp.setParameter(IloCP.IntParam.SearchType, ParameterValues.Neighborhood);
            if (!cp.solve())
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
