// ------------------------------------------------------------- -*- Java -*-
// File: ./examples/src/java/Cvrptw.java
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

In the Capacitated Vehicle Routing Problem with Time Windows (CVRPTW),
a fleet of vehicles with identical limited capacity leave a depot to serve
a set of customers with a single commodity and then come back to the depot.
All customers have known demand and must be served by a single visit.
For each delivery, the vehicle must arrive during the customer preferred
time windows. A service time for delivery is also associated with each
customer.

The problem is to find a sequence of customers to be delivered by each
truck such that the total distance traveled by vehicles is minimized.

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
import ilog.concert.IloIntExpr;
import ilog.concert.IloIntVar;
import ilog.concert.IloNumExpr;
import ilog.cp.IloCP;

public class Cvrptw {
    // Scaling factor for distances before rounding down
    private static final int TIME_FACTOR = 10;

    private static class DataReader {

        private StreamTokenizer st;

        public DataReader(String filename) throws IOException {
            FileInputStream fstream = new FileInputStream(filename);
            Reader r = new BufferedReader(new InputStreamReader(fstream));
            st = new StreamTokenizer(r);
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

        public void skip_elems(int n) throws IOException {
            for (int i = 0; i < n; i++)
                next();
        }
    }

    /**
     * Holds a representation of the raw data of the problem. Indices in
     *  the getter API are customer indices from 0 to getNbCustomers()-1
     */
    private static class CVRPTWProblem {
        private int nbTrucks = -1;
        private int truckCapacity = -1;
        private int maxHorizon = -1;
        private int nbCustomers = -1;
        private int[] depotXy = null;
        private List<int[]> xyList = new ArrayList<int[]>();
        private List<Integer> demands = new ArrayList<Integer>();
        private List<Integer> earliestStart = new ArrayList<Integer>();
        private List<Integer> latestStart = new ArrayList<Integer>();
        private List<Integer> serviceTime = new ArrayList<Integer>();

        public CVRPTWProblem(String filename) throws IOException {
            read(filename);
        }

        public void read(String filename) throws IOException {
            DataReader data = new DataReader(filename);

            data.skip_elems(4);
            nbTrucks = data.next_as_int();
            truckCapacity = data.next_as_int();
            data.skip_elems(13);
            
            depotXy = new int[] {data.next_as_int(), data.next_as_int()};
            data.skip_elems(2);
            maxHorizon = data.next_as_int();
            data.skip_elems(1);

            int idx = 0;
            xyList.add(depotXy);
            while (true) {
                try {
                    idx = data.next_as_int() - 1;
                    int[] locXy = new int[] {data.next_as_int(), data.next_as_int()}; 
                    xyList.add( locXy );
                    demands.add(data.next_as_int());

                    int ready = data.next_as_int();
                    int due = data.next_as_int();
                    int stime = data.next_as_int();
                    earliestStart.add(ready);
                    latestStart.add(due);
                    serviceTime.add(stime);
                }
                catch (IOException e) {
                    // End of file
                    break;
                }
            }
            nbCustomers = idx + 1;
        }

        public int getNbCustomers() { return nbCustomers; }

        public int getNbTrucks() { return nbTrucks; }

        public int getCapacity() { return truckCapacity; }

        public int getMaxHorizon() { return TIME_FACTOR * maxHorizon; }

        public int getDemand(int i) {
            assert(i >= 0);
            assert(i <= getNbCustomers());
            if (i == 0)  return 0;
            return demands.get(i - 1);
        }

        public int getServiceTime(int i) {
            assert(i >= 0);
            assert(i <= getNbCustomers());
            if (i == 0) return 0;
            return TIME_FACTOR * serviceTime.get(i - 1);
        }

        public int getEarliestStart(int i) {
            assert(i >= 0);
            assert(i <= getNbCustomers());
            if (i == 0) return 0;
            return TIME_FACTOR * earliestStart.get(i - 1);
        }

        public int getLatestStart(int i) {
            assert(i >= 0);
            assert(i <= getNbCustomers());
            if (i == 0) return 0;
            return TIME_FACTOR * latestStart.get(i - 1);
        }

        public int getDistance(int from_, int to_) {
            assert(from_ >= 0);
            assert(from_ <= getNbCustomers());
            assert(to_ >= 0);
            assert(to_ <= getNbCustomers());

            int[] c1 = xyList.get(from_);
            int[] c2 = xyList.get(to_);
            int dx = c2[0] - c1[0];
            int dy = c2[1] - c1[1];
            double d = Math.sqrt(dx * dx + dy * dy);
            // In order to reproduce results for best known solutions for Solomon benchmark,
            //  actual distances between points are rounded using the formulation below:
            return (int) Math.floor(d * TIME_FACTOR);
        }
    }

    /**
     *  Represents the data structures of the model. There are
     *   2*numVehicles + numCustomers nodes.  The customer nodes are
     *   guaranteed to correspond to the CVRPTWProblem class (0..nbCustomers-1).
     *   Indices of the start and end nodes are found
     *   using getFirst(vehicle), getLast(vehicle).
     */
    private static class VRP extends IloCP {

        private static class VisitData {
            int demand;
            int serviceTime;
            int earliest;
            int latest;
        
            public VisitData(int demand, int serviceTime, int earliest, int latest) {
                this.demand = demand;
                this.serviceTime = serviceTime;
                this.earliest = earliest;
                this.latest = latest;
            }
        }

        private CVRPTWProblem problem;
        private int[] firstList, lastList, pnode;
        private int maxHorizon, capacity;
        private VisitData[] visitData;

        private IloIntVar[] veh;
        private IloIntVar[] startTime;
        private IloIntVar[] load;
        private IloIntVar used;
        private IloIntVar[] prev;

        public VRP(CVRPTWProblem pb) throws IloException {
            initDataModel(pb);

            buildVariables();
            buildStructure();
            enforceLoad();
            enforceTimes();
            // Objective
            IloNumExpr obj = getTotalDistance();
            add(minimize(obj));
        }

        private static String[] createIndexedNameList(String prefix, int count) {
            String[] result = new String[count];
            for (int i = 0; i < count; i ++) result[i] = prefix + i;
            return result;
        }

        private void buildVariables() throws IloException {
            int numCust = getNumCustomers();
            int numVehicles = getNumVehicles();
            int n = getNumVisits();

            prev = intVarArray(n, 0, n - 1, createIndexedNameList("P_", n));
            veh = intVarArray(n, 0, numVehicles - 1, createIndexedNameList("V_", n));
            load = intVarArray(numVehicles, 0, getCapacity(), createIndexedNameList("L_", numVehicles));
            used = intVar(0, numVehicles, "U");
            startTime = new IloIntVar[n];
            for (int i = 0; i < n; i++)
                startTime[i] = intVar(getEarliestStart(i), getLatestStart(i), "ST_" + i);

            add(inferred(veh));
            add(inferred(startTime));
            add(inferred(load));
            add(inferred(used));
        }

        private void buildStructure() throws IloException {
            int numCust = getNumCustomers();
            int numVehicles = getNumVehicles();

            // Prev variables, circuit, first/last
            // First points to last of previous vehicle
            for (int v = 0; v < numVehicles; v++) {
                int first_v = getFirst(v);
                int last_v = getLast(v);
                add(eq(prev[first_v], getLast((v - 1 + numVehicles) % numVehicles)));

                // Last can point it its first or any customer
                int[] before_last = new int[numCust + 1];
                for (int c = 0; c < numCust; c++)
                    before_last[c] = c;
                before_last[numCust] = first_v;
                add(allowedAssignments(prev[last_v], before_last));
            }
            for (int v = 0; v < numVehicles; v++) {
                int first_v = getFirst(v);
                int last_v = getLast(v);
                // Vehicles of first, last or just before last
                add(eq(veh[first_v], v));
                add(eq(veh[last_v], v));
                add(eq(element(veh, prev[last_v]), v));
            }

            int[] before = new int[numCust + numVehicles];
            for (int c = 0; c < numCust; c++) before[c] = c;
            for (int v = 0; v < numVehicles; v++) before[numCust + v] = getFirst(v);
            for (int c = 0; c < numCust; c++) {
                add(allowedAssignments(prev[c], before));
                add(neq(c, prev[c]));  // no customers are optional
                add(eq(veh[c], element(veh, prev[c])));
            }

            add(subCircuit(prev));
        }

        private void enforceLoad() throws IloException {
            int numCust = getNumCustomers();

            IloIntVar[] custVeh = new IloIntVar[numCust];
            int[] demand = new int[numCust];
            for (int c = 0; c < numCust; c++) {
                custVeh[c] = veh[c];
                demand[c] = getDemand(c);
            }
            add(pack(load, custVeh, demand, used));
        }

        private IloIntExpr arrivalTime(int to) throws IloException {
            int n = getNumVisits();
            IloIntExpr[] arriveTimeByPrev = new IloIntExpr[n];
            for (int from = 0; from < n; from++) {
                arriveTimeByPrev[from] = sum(
                    startTime[from],
                    getServiceTime(from) + getDistance(from, to)
                );
            }
            IloIntExpr arrive = element(arriveTimeByPrev, prev[to]);
            return arrive;
        }

        private void enforceTimes() throws IloException {
            int numCust = getNumCustomers();
            int numVehicles = getNumVehicles();

            for (int c = 0; c < numCust; c++) {
                add(eq(startTime[c], max(arrivalTime(c), getEarliestStart(c))));
            }
            for (int v = 0; v < numVehicles; v++) {
                int first_v = getFirst(v);
                int last_v = getLast(v);
                add(eq(startTime[first_v], 0));
                add(eq(startTime[last_v], arrivalTime(last_v)));
            }
        }

        private IloNumExpr getTotalDistance() throws IloException {
            int numCust = getNumCustomers();
            int numVehicles = getNumVehicles();
            int n = getNumVisits();

            IloNumExpr[] allDist = new IloNumExpr[numCust + numVehicles];
            int nbDist = 0;
            for (int c = 0; c < numCust; c++) {
                int[] ldist = new int[n];
                for (int j = 0; j < n; j++) ldist[j] = getDistance(j, c);
                allDist[nbDist++] = element(ldist, prev[c]);
            }
            for (int v = 0; v < numVehicles; v++) {
                int last_v = getLast(v);
                int[] ldist = new int[n];
                for (int j = 0; j < n; j++) ldist[j] = getDistance(j, last_v);
                allDist[nbDist++] = element(ldist, prev[last_v]);
            }
            IloNumExpr totalDistance = quot(sum(allDist), TIME_FACTOR);
            return totalDistance;
        }

        private void initDataModel(CVRPTWProblem pb) {
            this.problem = pb;
            int numCust = getNumCustomers();
            int numVehicles = getNumVehicles();
            int n = getNumVisits();

            // First, last, customer groups
            firstList = new int[numVehicles];
            lastList = new int[numVehicles];
            for (int i = 0; i < numVehicles; i++) firstList[i] = numCust + i;
            for (int i = 0; i < numVehicles; i++) lastList[i] = numCust + numVehicles + i;

            // Time and load limits
            maxHorizon = pb.getMaxHorizon();
            capacity = pb.getCapacity();

            // Node mapping between node indices in optimization model and
            //  corresponding node indices in input data (CVRPTWProblem instance)
            pnode = new int[numCust + 2 * numVehicles];
            for (int i = 0; i < numCust; i++) pnode[i] = i + 1;
            for (int i = numCust; i < n; i++) pnode[i] = 0;

            // Visit data
            visitData = new VisitData[numCust + 2 * numVehicles];
            for (int c = 0; c < numCust; c++)
                visitData[c] = new VisitData(pb.getDemand(pnode[c]),
                                             pb.getServiceTime(pnode[c]),
                                             pb.getEarliestStart(pnode[c]),
                                             pb.getLatestStart(pnode[c]));
            for (int i = numCust; i < numCust + 2 * numVehicles; i++)
                visitData[i] = new VisitData(0, 0, 0, this.maxHorizon);
        }

        public double solve(double tlim) throws IloException {
            // KPI        
            addKPI(used, "Used");

            setParameter(IloCP.IntParam.SearchType, IloCP.ParameterValues.Restart);
            setParameter(IloCP.IntParam.LogPeriod, 1000000);
            setParameter(IloCP.DoubleParam.TimeLimit, tlim);

            exportModel("cvrptw_java.cpo");

            double objValue = Double.POSITIVE_INFINITY;
            if (solve()) {
                objValue = this.getObjValue();
            }
            return objValue;
        }

        public int getNumCustomers() { return problem.getNbCustomers(); }

        public int getNumVehicles() { return problem.getNbTrucks(); }

        public int getNumVisits() { return getNumCustomers() + getNumVehicles() * 2; }

        public int getFirst(int veh) { return firstList[veh]; }

        public int getLast(int veh) { return lastList[veh]; }
        
        public int getCapacity() { return capacity; }
        
        public int getMaxHorizon() { return maxHorizon; }

        public int getDemand(int i) { return visitData[i].demand; }

        public int getServiceTime(int i) { return visitData[i].serviceTime; }

        public int getEarliestStart(int i) { return visitData[i].earliest; }

        public int getLatestStart(int i) { return visitData[i].latest; }

        public int getDistance(int i, int j) { return problem.getDistance(pnode[i],  pnode[j]); }
    }

    public static void solveModel(IloCP model) throws IloException {
        model.solve();
    }

    public static void main(String[] args) throws IOException {
        String filename = "../../../examples/data/cvrptw_C101_25.data";
        double timeLimit = 5;
        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            timeLimit = Double.valueOf(args[1]);

        CVRPTWProblem cvrptwProb = new CVRPTWProblem(filename);

        VRP vrp = null;
        try {
            vrp = new VRP(cvrptwProb);
            double obj = vrp.solve(timeLimit);
            if (obj != Double.POSITIVE_INFINITY)
                System.out.println("Found a solution of distance = " + obj);
            else
                System.out.println("No solution found");
        }
        catch (IloException e) {
            System.err.println("Error " + e);
        }
        finally {
            if (vrp != null)
                vrp.end();
        }
    }
}
