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

The TSP with Drone problem involves the collaboration between a truck and
a drone to deliver customers. The drone can pick up packages from the truck
and deliver them to the customer (but it cannot serve more than one location
at a time) while the truck is serving other customers.
The drone takes off or returns to the truck only at a customer location.
The problem consists in allocating delivery locations to either the truck
or the drone, while minimizing the total time to serve all customers, and
return to the depot.

The following model is an implementation of the paper "A Study on the Traveling
Salesman Problem with a Drone" (authors: )Ziye Tang, Willem-Jan van Hoeve and Paul Shaw).
The sample data file is an adaptation of the dataset "uniform-1-n11.txt" available
at: https://github.com/pcbouman-eur/TSP-D-Instances

------------------------------------------------------------ */

using CP;

execute CP_PARAM {
  cp.param.logPeriod = 1000000;
  cp.param.searchType = "Auto";
  cp.param.timeLimit = 5;
}


float truckSpeed = ...;
float droneSpeed = ...;
int numberOfNodes = ...;

tuple xy_id {
  float		x;
  float		y;
  string	id;
}
xy_id locations[0..numberOfNodes - 1] = ...;

range all_start_to_end_nodes = 0..numberOfNodes;
int intervalMax = maxint div 2 - 1;
int TIME_SCALE = 1000000;

tuple triplet {int id1; int id2; int value;};
{triplet} truckTravelTimesMatrix = {<i, j, ftoi(floor(sqrt(pow(locations[i].x - locations[j].x, 2.0) + pow(locations[i].y - locations[j].y, 2.0)) * TIME_SCALE * truckSpeed))>
									| i in 0..numberOfNodes - 1, j in 0..numberOfNodes - 1};

dvar interval visit[all_start_to_end_nodes] in 0..intervalMax;
dvar interval tVisit[all_start_to_end_nodes] optional in 0..intervalMax;
dvar interval dVisit[all_start_to_end_nodes] optional in 0..intervalMax;
dvar interval dVisitBefore[all_start_to_end_nodes] optional in 0..intervalMax;
dvar interval dVisitAfter[all_start_to_end_nodes] optional in 0..intervalMax;

int tTypes[i in all_start_to_end_nodes] = i mod numberOfNodes;
int dTypes[i in all_start_to_end_nodes] = i;
dvar sequence tVisitSeq in tVisit types tTypes;
dvar sequence dVisitSeq in dVisit types dTypes;

dvar interval tdVisit[all_start_to_end_nodes][all_start_to_end_nodes] optional in 0..intervalMax;
dvar interval dtVisit[all_start_to_end_nodes][all_start_to_end_nodes] optional in 0..intervalMax;

minimize endOf(tVisit[numberOfNodes]);

subject to {
  startOf(tVisit[0]) == 0;  // Truck departs from depot at time = 0
  presenceOf(tVisit[0]);
  presenceOf(tVisit[numberOfNodes]);
  forall (i in 0..(numberOfNodes-1))
    endBeforeStart(tVisit[i], tVisit[numberOfNodes]);

  first(tVisitSeq, tVisit[0]);
  last(tVisitSeq, tVisit[numberOfNodes]);
  noOverlap(tVisitSeq, truckTravelTimesMatrix);
  noOverlap(dVisitSeq);

  forall (i in all_start_to_end_nodes) {
    alternative(visit[i], append(tVisit[i], dVisit[i]));
    alternative(dVisitBefore[i], all(j in all_start_to_end_nodes) tdVisit[i][j]);
    alternative(dVisitAfter[i], all(j in all_start_to_end_nodes) dtVisit[i][j]);

    span(dVisit[i], append(dVisitBefore[i], dVisitAfter[i]));
    endAtStart(dVisitBefore[i], dVisitAfter[i]);
    presenceOf(dVisit[i]) == presenceOf(dVisitBefore[i]);
    presenceOf(dVisit[i]) == presenceOf(dVisitAfter[i]);
  }

  forall (i in 0..(numberOfNodes-1))
    forall (j in all_start_to_end_nodes) {
      // Lower bounds on tdVisit and dtVisit
      lengthOf(tdVisit[i][j], intervalMax) >= ftoi(floor(sqrt(pow(locations[i].x - locations[j mod numberOfNodes].x, 2) + pow(locations[i].y - locations[j mod numberOfNodes].y, 2)) * TIME_SCALE * droneSpeed));
      lengthOf(dtVisit[i][j], intervalMax) >= ftoi(floor(sqrt(pow(locations[i].x - locations[j mod numberOfNodes].x, 2) + pow(locations[i].y - locations[j mod numberOfNodes].y, 2)) * TIME_SCALE * droneSpeed));

      presenceOf(tdVisit[i][j]) => presenceOf(tVisit[j]);
      presenceOf(dtVisit[i][j]) => presenceOf(tVisit[j]);

      startBeforeStart(tVisit[j], tdVisit[i][j]);
	  startBeforeEnd(tdVisit[i][j], tVisit[j]);
      endBeforeEnd(dtVisit[i][j], tVisit[j]);
      startBeforeEnd(tVisit[j], dtVisit[i][j]);
    }
}


execute {
  function getIntervalIndex(itv) {
	var name = itv.name;
	return parseInt(name.substring(name.indexOf("[") + 1, name.indexOf("]")));
  }

  function getDroneOriginIndex(itv) {
    var index = getIntervalIndex(itv);
    for (var j in all_start_to_end_nodes)
      if (tdVisit[index][j].present > 0) return j;
  }

  function getDroneReturnIndex(itv) {
    var index = getIntervalIndex(itv);
    for (var j in all_start_to_end_nodes)
      if (dtVisit[index][j].present > 0) return j;
  }

  function getTruckVisitLabel(truckVisitIndex) {
    if ((truckVisitIndex == 0) | (truckVisitIndex == numberOfNodes))
      return "DEPOT";
    return truckVisitIndex;
  }  

  var tvs = tVisitSeq.first();
  var dvs = dVisitSeq.first();
  var truckVisitIndex = getIntervalIndex(tvs);
  var droneVisitIndex = getIntervalIndex(dvs);
  var droneStartIdx = getDroneOriginIndex(dvs);
  var droneEndIdx = getDroneReturnIndex(dvs);
  var droneVisitInProgress = 0;
  while (1) {
    write(getTruckVisitLabel(truckVisitIndex));
    if (droneVisitInProgress > 0) write("  |");
    while ((droneStartIdx == truckVisitIndex) & (droneStartIdx == droneEndIdx)) {
      write(" = ", droneVisitIndex);
	  if (dvs != dVisitSeq.last()) {
	      dvs = dVisitSeq.next(dvs);
	      droneVisitIndex = getIntervalIndex(dvs);
	      droneStartIdx = getDroneOriginIndex(dvs);
	      droneEndIdx = getDroneReturnIndex(dvs);
	  }
    }
    writeln();
    if (droneStartIdx == truckVisitIndex) {
      droneVisitInProgress = 1;
	  writeln("| \\");
	  writeln("|  ", droneVisitIndex);
        
    } else {
      if (droneVisitInProgress > 0)
      	writeln("|  |");
      else
        writeln("|");
    }
    tvs = tVisitSeq.next(tvs);
    truckVisitIndex = getIntervalIndex(tvs);
    if (droneEndIdx == truckVisitIndex) {
      droneVisitInProgress = 0;
      writeln("| /");

	  if (dvs != dVisitSeq.last()) {
	      dvs = dVisitSeq.next(dvs);
	      droneVisitIndex = getIntervalIndex(dvs);
	      droneStartIdx = getDroneOriginIndex(dvs);
	      droneEndIdx = getDroneReturnIndex(dvs);
	  }
    }
    if (tvs == tVisitSeq.last()) {
      writeln(getTruckVisitLabel(truckVisitIndex));
      break;
    }
  }
  
  writeln('\n----------------------------------------');
  writeln("Truck visits:");
  var tvs = tVisitSeq.first();
  writeln(tvs.name, " --> ", tvs);
  while (1) {
    tvs = tVisitSeq.next(tvs);
    writeln(tvs.name, " --> ", tvs);
	if (tvs == tVisitSeq.last()) break;
  }

  writeln('----------------------------------------');
  writeln("Drone visits:");
  var dvs = dVisitSeq.first();
  writeln(dvs.name, " -> ", dvs, "\t === Start: ", getTruckVisitLabel(getDroneOriginIndex(dvs)), " - End: ", getTruckVisitLabel(getDroneReturnIndex(dvs)));
  while (1) {
    dvs = dVisitSeq.next(dvs);
    writeln(dvs.name, " -> ", dvs, "\t === Start: ", getTruckVisitLabel(getDroneOriginIndex(dvs)), " - End: ", getTruckVisitLabel(getDroneReturnIndex(dvs)));
	if (dvs == dVisitSeq.last()) break;
  }

  writeln('----------------------------------------');

}
