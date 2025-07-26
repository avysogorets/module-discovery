// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/SchedTruckDrone.java
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

The TSP with Drone problem involves the collaboration between a truck and
a drone to deliver customers. The drone can pick up packages from the truck
and deliver them to the customer (but it cannot serve more than one location
at a time) while the truck is serving other customers.
The drone takes off or returns to the truck only at a customer location.
The problem consists in allocating delivery locations to either the truck
or the drone, while minimizing the total time to serve all customers, and
return to the depot.

The following model is an implementation of the paper "A Study on the Traveling
Salesman Problem with a Drone" (authors: Ziye Tang, Willem-Jan van Hoeve and Paul Shaw).
The sample data file "uniform-1-n11.txt" is also available at:
https://github.com/pcbouman-eur/TSP-D-Instances

------------------------------------------------------------ */

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StreamTokenizer;
import java.util.ArrayList;
import java.util.List;

import ilog.concert.IloException;
import ilog.concert.IloIntervalSequenceVar;
import ilog.concert.IloIntervalVar;
import ilog.concert.IloNumExpr;
import ilog.concert.IloTransitionDistance;
import ilog.cp.IloCP;

public class SchedTruckDrone {
    // Scaling factor for converting travel times to integers
    private static final int TIME_FACTOR = 1000000;

    private static class DataReader {

        private StreamTokenizer st;

        public DataReader(String filename) throws IOException {
            FileInputStream fstream = new FileInputStream(filename);
            Reader r = new BufferedReader(new InputStreamReader(fstream));
            st = new StreamTokenizer(r);
            st.wordChars('(', '_');  // Data file contains special characters that should be handled as regular "word" characters
        }

        public String next() throws IOException {
            st.nextToken();
            if (st.ttype == StreamTokenizer.TT_EOF)
                throw new IOException();
            return st.sval;
        }

        public int next_as_int() throws IOException {
            next();
            return (int) st.nval;
        }

        public double next_as_double() throws IOException {
            next();
            return st.nval;
        }

        public void skip_elems(int n) throws IOException {
            for (int i = 0; i < n; i++)
                next();
        }
    }

    /**
     * Holds a representation of the raw data of the problem.
     */
    private static class TSPDProblem {
        private double truckSpeed = -1;
        private double droneSpeed = -1;
        private int nbCustomers = -1;
        private List<double[]> xyList = new ArrayList<double[]>();
        private List<String> idsList = new ArrayList<String>();

        public TSPDProblem(String filename) throws IOException {
            read(filename);
        }

        public void read(String filename) throws IOException {
            DataReader data = new DataReader(filename);

            
            data.skip_elems(5); // /*The speed of the Truck*/
            this.truckSpeed = data.next_as_double();

            data.skip_elems(5); // /*The speed of the Drone*/
            this.droneSpeed = data.next_as_double();

            data.skip_elems(3); // /*Number of Nodes*/
            this.nbCustomers = data.next_as_int() - 1;

            data.skip_elems(2); // /*The Depot*/
            double[] locXy = new double[] {data.next_as_double(), data.next_as_double()};
            String locId = data.next();
            xyList.add( locXy );
            idsList.add( locId );

            data.skip_elems(5); /*The Locations (x_coor y_coor name)*/

            for (int cust_idx = 0; cust_idx < this.nbCustomers; cust_idx++) {
                locXy = new double[] {data.next_as_double(), data.next_as_double()};
                locId = data.next();
                xyList.add( locXy );
                idsList.add( locId );
            }
        }

        public int getNumNodes() { return nbCustomers + 1; }

        public double getTruckSpeed() { return truckSpeed; }

        public double getDroneSpeed() { return droneSpeed; }

        public double getDistance(int from_, int to_) {
            assert(from_ >= 0);
            assert(from_ <= getNumNodes());
            assert(to_ >= 0);
            assert(to_ <= getNumNodes());

            double[] c1 = xyList.get(from_);
            double[] c2 = xyList.get(to_);
            double dx = c2[0] - c1[0];
            double dy = c2[1] - c1[1];
            double d = Math.sqrt(dx * dx + dy * dy);
            return d;
        }

        public String getNodeId(int nodeIdx) {
            assert(nodeIdx >= 0);
            assert(nodeIdx <= getNumNodes());
            return idsList.get(nodeIdx);
        }
    }

    /**
     *  Represents the data structures of the model.
     */
    private static class TSPD extends IloCP {

        private TSPDProblem problem;
        
        private IloIntervalVar[] visit;
        private IloIntervalVar[] tVisit;
        private IloIntervalVar[] dVisit;
        private IloIntervalVar[] dVisitBefore;
        private IloIntervalVar[] dVisitAfter;
        private IloIntervalVar[][] tdVisit;
        private IloIntervalVar[][] dtVisit;
        
        private IloIntervalSequenceVar tVisitSeq;
        private IloIntervalSequenceVar dVisitSeq;

        public TSPD(TSPDProblem pb) throws IloException {
            initDataModel(pb);

            buildVariables();
            buildStructure();

            // Objective
            IloNumExpr obj = endOf(tVisit[problem.getNumNodes()]);
            add(minimize(obj));
        }

        private void buildVariables() throws IloException {
            int numCust = problem.getNumNodes() - 1;

            visit = new IloIntervalVar[numCust + 2];
            tVisit = new IloIntervalVar[numCust + 2];
            dVisit = new IloIntervalVar[numCust + 2];
            dVisitBefore = new IloIntervalVar[numCust + 2];
            dVisitAfter = new IloIntervalVar[numCust + 2];
            for (int i = 0; i < numCust + 2; i++) {
                visit[i] = intervalVar("visit_" + i);

                tVisit[i] = intervalVar("tVisit_" + i);
                tVisit[i].setOptional();

                dVisit[i] = intervalVar("dVisit_" + i);
                dVisit[i].setOptional();

                dVisitBefore[i] = intervalVar("dVisit_before_" + i);
                dVisitBefore[i].setOptional();

                dVisitAfter[i] = intervalVar("dVisit_after_" + i);
                dVisitAfter[i].setOptional();
            }

            int[] tVisitTypes = new int[numCust + 2];
            int[] dVisitTypes = new int[numCust + 2];
            for (int i = 0; i < numCust + 2; i++) {
                tVisitTypes[i] = i;
                dVisitTypes[i] = i;
            }
            tVisitTypes[numCust + 1] = 0;  // Last type corresponds to depot (index = 0)
            tVisitSeq = intervalSequenceVar(tVisit, tVisitTypes);
            dVisitSeq = intervalSequenceVar(dVisit, dVisitTypes);
    
            tdVisit = new IloIntervalVar[numCust + 2][numCust + 2];
            dtVisit = new IloIntervalVar[numCust + 2][numCust + 2];
            for (int i = 0; i < numCust + 2; i++) {
                for (int j = 0; j < numCust + 2; j++) {
                    tdVisit[i][j] = intervalVar("tdVisit_" + i + "_" + j);
                    tdVisit[i][j].setOptional();
    
                    dtVisit[i][j] = intervalVar("dtVisit_" + i + "_" + j);
                    dtVisit[i][j].setOptional();
                }
            }
        }

        private void buildStructure() throws IloException {
            int numCust = problem.getNumNodes() - 1;
        
            // Force presence of truck visits from and to depot
            tVisit[0].setPresent();
            tVisit[numCust + 1].setPresent();

            // Truck returning at depot is the last event
            for (int i = 0; i < numCust + 1; i++) {
                add(endBeforeStart(tVisit[i], tVisit[numCust + 1]));
            }
            add(first(tVisitSeq, tVisit[0]));
            add(last(tVisitSeq, tVisit[numCust + 1]));

            //
            int[][] distanceTable = new int[numCust + 1][numCust + 1];
            for (int i = 0; i < numCust + 1; i++) {
                for (int j = 0; j < numCust + 1; j++) {
                    distanceTable[i][j] = (int) (
                        problem.getDistance(i, j) * TIME_FACTOR * problem.getTruckSpeed()
                    );
                }
            }
            IloTransitionDistance distanceMatrix = transitionDistance(distanceTable);
            add(noOverlap(tVisitSeq, distanceMatrix));
            add(noOverlap(dVisitSeq));

            //
            for (int i = 0; i < numCust + 2; i++) {
                add(alternative(visit[i], new IloIntervalVar[] {tVisit[i], dVisit[i]}));
                add(alternative(dVisitBefore[i], tdVisit[i]));
                add(alternative(dVisitAfter[i], dtVisit[i]));

                add(span(dVisit[i], new IloIntervalVar[] {dVisitBefore[i], dVisitAfter[i]}));
                add(endAtStart(dVisitBefore[i], dVisitAfter[i]));
                add(ifThen(presenceOf(dVisit[i]), and(presenceOf(dVisitBefore[i]), presenceOf(dVisitAfter[i]))));
            }

            //
            for (int i = 0; i < numCust + 1; i++) {
                for (int j = 0; j < numCust + 2; j++) {
                    // Lower bounds on tdVisit and dtVisit
                    add(ge(lengthOf(tdVisit[i][j], IntervalMax), (int) (problem.getDistance((j < numCust + 1 ? j : 0), i) * TIME_FACTOR * problem.getDroneSpeed())));
                    add(ge(lengthOf(dtVisit[i][j], IntervalMax), (int) (problem.getDistance(i, (j < numCust + 1 ? j : 0)) * TIME_FACTOR * problem.getDroneSpeed())));

                    // Prevents warning about constraints that are always True
                    if ((j != 0) && (j != numCust + 1)) {
                        add(ifThen(presenceOf(tdVisit[i][j]), presenceOf(tVisit[j])));
                        add(ifThen(presenceOf(dtVisit[i][j]), presenceOf(tVisit[j])));
                    }
                    // Precedence constraints
                    add(startBeforeStart(tVisit[j], tdVisit[i][j]));
                    add(startBeforeEnd(tdVisit[i][j], tVisit[j]));
                    add(endBeforeEnd(dtVisit[i][j], tVisit[j]));
                    add(startBeforeEnd(tVisit[j], dtVisit[i][j]));   // ADDED to original model from Tang, van Hoeve & Shaw paper (otherwise may generate invalid solutions)
                }
            }
        }

        private void initDataModel(TSPDProblem pb) {
            this.problem = pb;
        }

        public double solve(double tlim) throws IloException {
            setParameter(IloCP.IntParam.SearchType, IloCP.ParameterValues.Auto);
            setParameter(IloCP.IntParam.LogPeriod, 1000000);
            setParameter(IloCP.DoubleParam.TimeLimit, tlim);

            exportModel("tspd_java.cpo");

            double objValue = Double.POSITIVE_INFINITY;
            if (solve()) {
                objValue = this.getObjValue();
            }
            return objValue;
        }
    }

    public static void main(String[] args) throws IOException {
        String filename = "../../../examples/data/uniform-1-n11.txt";
        double timeLimit = 5;
        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            timeLimit = Double.valueOf(args[1]);

        TSPDProblem tspDProb = new TSPDProblem(filename);

        TSPD tspd = null;
        try {
            tspd = new TSPD(tspDProb);
            double obj = tspd.solve(timeLimit);
            if (obj != Double.POSITIVE_INFINITY)
                System.out.println("Found a solution with total time = " + (obj / TIME_FACTOR));
            else
                System.out.println("No solution found");
        }
        catch (IloException e) {
            System.err.println("Error " + e);
        }
        finally {
            if (tspd != null)
                tspd.end();
        }
    }
}
