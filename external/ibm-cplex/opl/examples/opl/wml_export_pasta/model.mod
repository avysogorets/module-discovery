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

tuple TProduct {
  key string name;
  float demand;
  float insideCost;
  float outsideCost;
};

tuple TResource {
  key string name;
  float capacity;
};

tuple TConsumption {
  key string productId;
  key string resourceId;
  float consumption; 
}

{TProduct}     Products = ...;
{TResource}    Resources = ...;
{TConsumption} Consumptions = ...;

/// solution
tuple TPlannedProduction {
  key string productId;
  float insideProduction;
  float outsideProduction; 
}

/// variables.
dvar float+ Inside [Products];
dvar float+ Outside[Products];

dexpr float totalInsideCost  = sum(p in Products)  p.insideCost * Inside[p];
dexpr float totalOutsideCost = sum(p in Products)  p.outsideCost * Outside[p];

minimize
  totalInsideCost + totalOutsideCost;
   
subject to {
  forall( r in Resources )
    ctCapacity: 
      sum( k in Consumptions, p in Products 
           : k.resourceId == r.name && p.name == k.productId ) 
        k.consumption* Inside[p] <= r.capacity;

  forall(p in Products)
    ctDemand:
      Inside[p] + Outside[p] >= p.demand;
}

{TPlannedProduction} plan = {<p.name, Inside[p], Outside[p]> | p in Products};

// Display the production plann
execute DISPLAY_PLAN {
  for( var p in plan ) {
    writeln("p[",p.productId,"] = ",p.insideProduction," inside, ", p.outsideProduction, " outside.");
  }
}