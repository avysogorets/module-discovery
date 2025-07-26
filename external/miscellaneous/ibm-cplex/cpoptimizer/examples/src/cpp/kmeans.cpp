// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/kmeans.cpp
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

#include <ilcp/cp.h>

/* ------------------------------------------------------------

Problem Description
-------------------

K-means is a way of clustering points in a multi-dimensional space
where the set of points to be clustered are partitioned into k subsets.
The idea is to minimize the inter-point distances inside a cluster in
order to produce clusters which group together close points.

See https://en.wikipedia.org/wiki/K-means_clustering

------------------------------------------------------------ */

// Returns an IloModel object whose decision variables 
static IloModel MakeModel(
  IloEnv env,
  IloArray<IloNumArray> coords,
  IloInt n,
  IloInt d,
  IloInt k,
  IloBool trustNumerics = IloTrue
) {
  // model
  IloModel mdl(env);
  // decision variables.  x[c] = cluster to which node c belongs
  IloIntVarArray x(env, n, 0, k-1);
  for (IloInt i = 0; i < n; ++i) {
    char name[100];
    snprintf(name, sizeof(name), "C_%ld", (long)i);
    x[i].setName(name);
  }

  // Size (number of nodes) in each cluster.  If this is zero, we make
  // it 1 to avoid division by zero later (if a particular cluster is
  // not used).
  IloIntExprArray csize(env, k);
  for (IloInt c = 0; c < k; ++c)
    csize[c] = IloMax(1, IloCount(x, c));
  // accumulate squared distances
  IloNumExpr totalDist2(env);
  for (IloInt c = 0; c < k; ++c)  {
    // Boolean vector saying which points are in this cluster
    IloNumExprArray included(env);
    for (IloInt i = 0; i < n; ++i)
      included.add(x[i] == c);

    for (IloInt dim = 0; dim < d; ++dim) {
      // Points for each point in the given dimension (x, y, z, ...)
      IloNumArray point = coords[dim];

      // Calculate the cluster centre for this dimension
      IloNumExpr center = IloNumExpr(env);
      for (IloInt i = 0; i < n; ++i)
        center += point[i] * included[i];
      center = center / csize[c];

      // Calculate the total distance^2 for this cluster & dimension
      IloNumExpr dist2 = IloNumExpr(env);
      if (trustNumerics) {
        IloNumExprArray squared(env, point.getSize());
        for (IloInt i = 0; i < point.getSize(); ++i)
          dist2 += IloSquare(point[i]) * included[i];
        dist2 -= IloSquare(center) * csize[c];
      } else {
        for (IloInt i = 0; i < point.getSize(); ++i)
          dist2 += IloSquare(center - point[i]) * included[i];
      }
      // Update the total
      totalDist2 += dist2;
    }
  }
  // Minimize the total distance squared.
  mdl.add(IloMinimize(env, totalDist2));
  return mdl;
}

int main(int argc, const char** argv) {
  IloInt n = 500;       // number of points
  IloInt d = 2;         // number of dimensions
  IloInt k = 5;         // number of clusters
  IloInt sd = 1234;     // random seed

  if(argc > 1) n = atoi(argv[1]);
  if(argc > 2) d = atoi(argv[2]);
  if(argc > 3) k = atoi(argv[3]);
  if(argc > 4) sd = atoi(argv[4]);

  std::cout << "Generating with N = " << n
            << ", D = " << d << ", K = " << k << std::endl;

  IloEnv env;
  IloRandom random(env);
  random.reSeed(sd);

  try {
    // array of point coordinates indexed by dimensions
    IloArray<IloNumArray> coords = IloArray<IloNumArray>(env);
    for (IloInt i = 0; i < d; ++i) {
      IloNumArray points(env);
      for (IloInt j = 0; j < n; ++j)
        points.add(random.getFloat());
      coords.add(points);
    }
    // create the model and CP
    IloModel mdl = MakeModel(env, coords, n, d, k);
    IloCP cp(mdl);
    // limit solve time and adjust log period
    cp.setParameter(IloCP::TimeLimit, 20);
    cp.setParameter(IloCP::LogPeriod, 50000);
    // launch the search
    std::cout << "With Restart search" << std::endl;
    // cp.setParameter(IloCP::SearchType, IloCP::Restart);
    cp.solve();
    // launch the Neighborhood search
    std::cout << "With Neighborhood search" << std::endl;
    cp.setParameter(IloCP::SearchType, IloCP::Neighborhood);
    cp.solve();
  }
  catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}

