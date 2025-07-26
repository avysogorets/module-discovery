// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corporation 1998, 2024. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------
K-means is a way of clustering points in a multi-dimensional space
where the set of points to be clustered are partitioned into k subsets.
The idea is to minimize the inter-point distances inside a cluster in
order to produce clusters which group together close points.

See https://en.wikipedia.org/wiki/K-means_clustering

------------------------------------------------------------ */


using CP;

int N = 500;       // number of points
int D = 2;         // number of dimensions
int K = 5;         // number of clusters

range RN = 0..N-1;
range RD = 0..D-1;
range RK = 0..K-1;

float coords[RD][RN];
execute {
  for (var i in RD)
    for (var j in RN)
      coords[i][j]= Math.random();
}

execute {
  cp.param.SearchType = "Neighborhood";
  cp.param.TimeLimit = 50;
  cp.param.LogPeriod = 100000;
}

// x[c] = cluster to which node c belongs
dvar int x[RN] in 0..K-1;

// Size (number of nodes) in each cluster.  If this is zero, we make
// it 1 to avoid division by zero later (if a particular cluster is
// not used).
dexpr int csize[c in RK] = maxl(1, count(all(i in RN) x[i], c));

// Boolean vector saying which points are in this cluster
dvar int included[i in RN][c in RK] in 0..1;

// Calculate the cluster centre for this dimension
dexpr float centre[c in RK][d in RD] = sum(i in RN) included[i][c] * coords[d][i] / csize[c];

float squared[d in RD][n in RN] = coords[d][n] * coords[d][n];

dexpr float sum_of_x2[c in RK][d in RD] = sum(i in RN) included[i][c] * squared[d][i];

dexpr float squared_centre[c in RK][d in RD] = centre[c][d] * centre[c][d];

// Calculate the total distance^2 for this cluster & dimension
dexpr float dist2[c in RK][d in RD] = sum_of_x2[c][d] - (squared_centre[c][d] * csize[c]);

// the total distance squared
dexpr float total_dist2 = sum(c in RK, d in RD) dist2[c][d];

// Minimize the total distance squared
minimize(total_dist2);

subject to {
  forall(c in RK)
    forall(i in RN)
      included[i][c] == (x[i]==c);
}

