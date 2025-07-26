/* --------------------------------------------------------------------------
 * File: genericbranchpi.c
 * Version 22.1.2
 * --------------------------------------------------------------------------
 * Licensed Materials - Property of IBM
 * 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
 * Copyright IBM Corporation 2019, 2024. All Rights Reserved.
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with
 * IBM Corp.
 * --------------------------------------------------------------------------
 */

/* Demonstrates how to perform customized branching using the generic
   callback.

   For any model with integer variables passed on the command line, the
   code will solve the model using a simple customized branching strategy.
   The branching strategy implemented here is most-infeasible branching,
   i.e., the code always picks the integer variable that is most fractional
   and then branches on it. If the biggest fractionality of all integer
   variables is small then the code refrains from custom branching and lets
   CPLEX decide.

   See the usage message below.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ilcplex/cplex.h>

#define EPSILON 1E-5
#define BIGREAL 1E+32

typedef struct {
   int *qrow;
   int qnzc;
   int *qcol;
   double *qval;
   int *lind;
   int lnzc;
   double *lval;
} QCONSTR;

typedef QCONSTR * QCONSTRPtr;

/* Data that is used within the callback. */
typedef struct {
   char *ctype;      /* Variable types. */
   int objsen;       /* Objective sense. */
   double *obj;      /* Objective coefficients. */
   double *rhs;      /* Objective coefficients. */
   int matnz;        /* Linear constraint matrix info. */
   int  *cmatbeg;    /* Linear constraint matrix info. */
   int  *cmatind;    /* Linear constraint matrix info. */
   double *cmatval;  /* Linear constraint matrix info. */
   int cols;         /* Number of variables. */
   int lrows;        /* Nmber of linear rows. */
   int qrows;        /* Nmber of quadratic rows. */
   int calls;        /* How often was the callback invoked? */
   int branches;     /* How many branches did we create. */
   QCONSTRPtr *qconstr; /* Quadratic contraints */
   double *grad;     /* Array to store the gradient of any quadratic constraint */
} CALLBACKDATA;


/* Print a usage message and exit. */
static void
usage (const char *progname)
{
   fprintf(stderr, "Usage: %s filename...\n", progname);
   fprintf(
      stderr,
      "  filename   Name of a file, or multiple files, with .mps, .lp,\n"
      "             or .sav extension, and a possible, additional .gz\n"
      "             extension.\n");
   exit(2);
}

static void getgrad (QCONSTRPtr qc, const double *x, 
                     int cols, double *grad)
{
   int *qrow = qc->qrow;
   int qnzc = qc->qnzc;
   int *qcol = qc->qcol;
   double *qval = qc->qval;
   int *lind = qc->lind;
   int lnzc = qc->lnzc;
   double *lval = qc->lval;

   int j,k;
   for (j = 0; j < cols; ++j)
      grad[j] = 0.0;

   for (k = 0; k < qnzc; ++k) { 
      grad[qcol[k]] += qval[k] * x[qrow[k]];
      grad[qrow[k]] += qval[k] * x[qcol[k]];
   }
   for (j = 0; j < lnzc; ++j) {
      grad[lind[j]] += lval[j];
   }
}

/* Generic callback that implements most infeasible branching. */
static int CPXPUBLIC
branchcallback (CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                void *cbhandle)
{
   CALLBACKDATA *data = cbhandle;
   QCONSTRPtr *qconstr = data->qconstr;

   int lpstat;
   double objval;

   int status = 0;
   double *relx = NULL;
   double *lrpi = NULL;
   double *qrpi = NULL;
   int *qrpidef = NULL;

   (void)contextid; /* unused */

   /* NOTE: Strictly speaking, the increment of data->calls and data->branches
    *       should be protected by a lock/mutex/semaphore. However, to keep
    *       the code simple we don't do that here.
    */
   data->calls++;

   /* Get the solution status of the current relaxation.
    */
   status = CPXcallbackgetrelaxationstatus (context, &lpstat, 0);
   if ( status ) {
      fprintf (stderr, "Failed to get relaxation status: %d\n", status);
      goto CB_TERMINATE;
   }

   /* Only branch if the current node relaxation could be solved to optimality.
    * If there was any sort of trouble then don't do anything and thus let
    * CPLEX decide how to cope with that.
    */
   if (  lpstat != CPX_STAT_OPTIMAL       &&
         lpstat == CPX_STAT_OPTIMAL_INFEAS   ) 
        goto CB_TERMINATE;
   
   relx = malloc (data->cols * sizeof(double));
   status = CPXcallbackgetrelaxationpoint (context, relx, 0, data->cols - 1, &objval);
   if ( status ) {
      fprintf (stderr, "Failed to get relx: %d\n", status);
      goto CB_TERMINATE;
   }

   if (data->lrows > 0) 
      lrpi = malloc (data->lrows * sizeof(double));
   
   if (data->qrows > 0) {
      qrpi = malloc (data->qrows * sizeof(double));
      qrpidef = malloc (data->qrows * sizeof(int));
   }

   double zerotol = -1.0;
   status = CPXcallbackgetrelaxationpi (context, lrpi, 0, data->lrows - 1, 
                                        qrpi, qrpidef, 0, data->qrows - 1, &zerotol);
   if ( status ) {
      goto CB_TERMINATE;
   }


   double upfrac, downfrac, score;
   double bestscore = data->objsen * CPX_INFBOUND;
   int bestvar = -1;
   int cstart, cend;
   int objsen = data->objsen;
   
   int j;
   for (j = 0; j < data->cols; ++j) {
      if ( data->ctype[j] == CPX_CONTINUOUS ||
            data->ctype[j] == CPX_SEMICONT )
            continue;
      
      if ( fabs (round (relx[j]) - relx[j]) < EPSILON )
         continue;

      upfrac =  ceil (relx[j]) - relx[j];
      downfrac = relx[j] - floor (relx[j]);

      score = 0.0;
      int found = 0;
      cstart = data->cmatbeg[j];
      cend = j < data->cols - 1 ? data->cmatbeg[j + 1] : data->matnz;
      int ind;
      for (ind = cstart; ind < cend; ++ind) {
         int i = data->cmatind[ind];

         if (  fabs (lrpi[i]) < EPSILON         || 
               fabs (lrpi[i]) > CPX_INFBOUND / 2  )
               continue;
         
         score += (lrpi[i] * data->cmatval[ind]);
         if ( !found )  found = 1;
      } 

      int qi;
      for (qi = 0; qi < data->qrows; ++qi) {
         if (!qrpidef[qi])
            continue;

         getgrad (qconstr[qi], relx, data->cols, data->grad);
         double qdj = qrpi[qi] * data->grad[j];
         
         if (  fabs (qdj) < EPSILON         || 
               fabs (qdj) > CPX_INFBOUND / 2  )
               continue;

         score += qdj;
         if ( !found )  found = 1;
      }

      if ( found ) {
         score =  objsen * fmin (objsen * score * (- upfrac), 
                                 objsen * score * downfrac);
      }

      if (  found                                 && 
            objsen * (bestscore - score) > 0 ) {
            bestscore = score;
            bestvar = j;
      }       
   }

   if ( bestvar > 0 ) {
      int downchild, upchild;
      double const up = ceil (relx[bestvar]);
      double const down = floor (relx[bestvar]);

      /* Create the UP branch. */
      status = CPXcallbackmakebranch (context, 1, &bestvar, "L", &up,
                                       0, 0, NULL, NULL, NULL, NULL, NULL,
                                       objval, &upchild);
      if ( status ) {
         fprintf (stderr, "Failed to create up branch: %d\n", status);
         goto CB_TERMINATE;
      }
      data->branches++;

      /* Create the DOWN branch. */
      status = CPXcallbackmakebranch (context, 1, &bestvar, "U", &down,
                                       0, 0, NULL, NULL, NULL, NULL, NULL,
                                       objval, &downchild);
      if ( status ) {
         fprintf (stderr, "Failed to create down branch: %d\n", status);
         goto CB_TERMINATE;
      }
      data->branches++;

      /* We don't use the unique ids of the down and up child. We only
         * have them so illustrate how they are returned from
         * CPXcallbackmakebranch().
         */
      (void)downchild;
      (void)upchild;
   }
   
   CB_TERMINATE:
   
   if (relx)
      free (relx);

   if (lrpi)
      free (lrpi);

   if (qrpi)
      free (qrpi);

   if (qrpidef)
      free (qrpidef);

   return status;
} /* END branchcallback */

int
main (int argc, char *argv[])
{
   int a;

   if ( argc <= 1 )
      usage (argv[0]);

   /* Loop over all command line arguments. */
   for (a = 1; a < argc; ++a) {
      int status = 0;
      CPXENVptr env;
      CPXLPptr lp;
      double objval;
      CALLBACKDATA data = { NULL, 0, NULL, NULL, 
                           0, NULL, NULL, NULL,
                           0, 0, 0, 0, 0, NULL, NULL };
      
      /* Create CPLEX environment and immediately enable screen output so that
       * we can see potential error messages etc.
       */
      env = CPXopenCPLEX (&status);
      if ( env == NULL || status ) {
         fprintf (stderr, "Failed to create environment: %d\n", status);
         goto TERMINATE;
      }
      status = CPXsetintparam (env, CPXPARAM_ScreenOutput, CPX_ON);
      if ( status ) {
         fprintf  (stderr, "Failed to turn enable screen output: %d\n", status);
         goto TERMINATE;
      }
      status = CPXsetdblparam (env, CPXPARAM_TimeLimit, 120);
      if ( status ) {
         fprintf  (stderr, "Failed to set time limit: %d\n", status);
         goto TERMINATE;
      }

      /* Read the model file into a newly created problem object. */
      lp = CPXcreateprob (env, &status, "");
      if ( lp == NULL || status ) {
         fprintf (stderr, "Failed to create problem object: %d\n", status);
         goto TERMINATE;
      }
      status = CPXreadcopyprob (env, lp, argv[a], NULL);
      if ( status ) {
         fprintf (stderr, "Failed to read %s: %d\n", argv[a], status);
         goto TERMINATE;
      }

      /* Read the column types into the data that we will pass into the
       * callback.
       */
      data.cols = CPXgetnumcols (env, lp);
      data.lrows = CPXgetnumrows (env, lp);
      data.qrows = CPXgetnumqconstrs (env, lp);
      data.objsen = CPXgetobjsen (env, lp);
      
      data.ctype = malloc (data.cols * sizeof(char));
      status = CPXgetctype (env, lp, data.ctype, 0, data.cols - 1);
      if ( status ) {
         fprintf (stderr, "Failed to query variable types: %d\n", status);
         goto TERMINATE;
      }

      data.obj = malloc (data.cols * sizeof(double));
      status = CPXgetobj (env, lp, data.obj, 0, data.cols - 1);
      if ( status ) {
         fprintf (stderr, "Failed to query objective: %d\n", status);
         goto TERMINATE;
      }

      data.rhs = malloc (data.lrows * sizeof (double));
      CPXgetrhs (env, lp, data.rhs, 0, data.lrows - 1);

      int matspace = CPXgetnumnz (env, lp);
      data.cmatbeg = malloc (matspace * sizeof (int));
      data.cmatind = malloc (matspace * sizeof (int));
      data.cmatval = malloc (matspace * sizeof (double));
      int surplus;
      status = CPXgetcols (env, lp, &data.matnz, 
                           data.cmatbeg, data.cmatind, data.cmatval, 
                           matspace, &surplus, 0, data.cols - 1);
      if ( status || surplus != 0) {
         fprintf (stderr, "Failed to query matrix (status: %d, surplus: %d)\n", 
                  status, surplus);
         goto TERMINATE;
      }

      data.qconstr = malloc (data.qrows * sizeof(QCONSTRPtr));
      double rhs;
      char sense;
      int q;
      for (q = 0; q < data.qrows; ++q) {
         QCONSTRPtr qc = NULL;
         qc = malloc (sizeof(QCONSTR));
         int lsp, qsp;
         status = CPXgetqconstr (env, lp, &qc->lnzc, &qc->qnzc, &rhs, &sense, 
                                 NULL, NULL, 0, &lsp, NULL, NULL, NULL, 
                                 0, &qsp, q);
         if ( status != CPXERR_NEGATIVE_SURPLUS ) {
            fprintf (stderr, "Failed to query quadratic constraint: %d\n", status);
            goto TERMINATE;
         }
         qc->lnzc = - lsp;
         qc->qnzc = - qsp;
         qc->lind = malloc (qc->lnzc * sizeof(int));
         qc->lval = malloc (qc->lnzc * sizeof(double));
         qc->qrow = malloc (qc->qnzc * sizeof(int));
         qc->qcol = malloc (qc->qnzc * sizeof(int));
         qc->qval = malloc (qc->qnzc * sizeof(double));
         status = CPXgetqconstr (env, lp, &qc->lnzc, &qc->qnzc, &rhs, &sense, 
                                 qc->lind, qc->lval, qc->lnzc, &lsp, qc->qrow, 
                                 qc->qcol, qc->qval, qc->qnzc, &qsp, q);
         if ( status ) {
            fprintf (stderr, "Failed to query quadratic constraint: %d\n", status);
            goto TERMINATE;
         }
         data.qconstr[q] = qc;
      }

      data.grad = malloc (data.cols * sizeof(double));
      
     /* Register the callback function. */
      status = CPXcallbacksetfunc (env, lp, CPX_CALLBACKCONTEXT_BRANCHING,
                                   branchcallback, &data);
      if ( status ) {
         fprintf (stderr, "Failed to set callback: %d\n", status);
         goto TERMINATE;
      }

      /* Solve the model. */
      status = CPXmipopt (env, lp);
      if ( status ) {
         fprintf (stderr, "Optimization failed: %d\n", status);
         goto TERMINATE;
      }

      /* Report some statistics. */
      printf ("Model %s solved, solution status = %d\n", argv[a], CPXgetstat (env, lp));
      status = CPXgetobjval (env, lp, &objval);
      if ( status )
         printf ("No objective value (error = %d\n", status);
      else
         printf ("Objective = %f\n", objval);
      printf ("Callback was invoked %d times and created %d branches\n",
              data.calls, data.branches);

   TERMINATE:
      /* Cleanup */
      free (data.ctype);
      free (data.obj);
      free (data.cmatbeg);
      free (data.cmatind);
      free (data.cmatval);
      for (q = 0; q < data.qrows; ++q) {
         free (data.qconstr[q]->lind);
         free (data.qconstr[q]->lval);
         free (data.qconstr[q]->qrow);
         free (data.qconstr[q]->qcol);
         free (data.qconstr[q]->qval);
      }
      if (data.qrows > 0) {
         free (data.qconstr);
         free (data.grad);
      }
      CPXfreeprob (env, &lp);
      CPXcloseCPLEX (&env);
      if ( status )
         return status;
   }

   return 0;
} /* END main */
