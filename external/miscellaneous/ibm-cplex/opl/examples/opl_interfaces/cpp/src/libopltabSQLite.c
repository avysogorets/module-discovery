/* --------------------------------------------------------------------------
 * File: libopltabSQLite.c
 * --------------------------------------------------------------------------
 * Licensed Materials - Property of IBM
 * 
 * 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 
 * Copyright IBM Corporation  2024. All Rights Reserved.
 *
 * Note to U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule
 * Contract with IBM Corp.
 * --------------------------------------------------------------------------
 */

/* Table connection implementation based on sqlite.
 * This file defines classes that can be registered with an IloOplModel
 * instance as a custom data source so that in .dat files we can use
 * statements like
 *    SQLiteConnection conn(...,...);  // connect to database
 *    data from SQLiteRead(conn, "SELECT * FROM data");
 *    result to SQLitePublish(conn, "INSERT INTO results VALUES(?)");
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sqlite3.h>
#include <ilopl/data/iloopltabledatasource.h>

#define TUPLE_SEPARATOR '.' /** Separator in fully qualified names for
                             * fields in sub-tuples. */

/* ********************************************************************** *
 *                                                                        *
 *    Utility functions                                                   *
 *                                                                        *
 * ********************************************************************** */

#ifdef _WIN32
#  define STRDUP _strdup
#else
#  define STRDUP strdup
#endif

/** Helper to finalize a statement and set it to NULL afterwards. */
static void finalize(sqlite3_stmt **stmt) {
  if ( *stmt ) {
    sqlite3_finalize(*stmt);
    *stmt = 0;
  }
}

/** Helper to execute an arbitrary SQL command.
 * The function returns true if no error and false otherwise. If any error
 * occurs then the respective error message is stored in error.
 * @param db    The database against which the statement is executed.
 * @param stmt  The statement to execute.
 * @param error String that receives the error message in case of error.
 * @return true if no error, false otherwise.
 */
static int exec(sqlite3 *db, char const *stmt, IloOplTableError error) {
  char *err = 0;
  int res = sqlite3_exec(db, stmt, 0, 0, &err);
  if ( err ) {
    if ( error )
      error->setBoth(error, res, "failed to execute '%s': %s", stmt, err);
    return 0;
  }
  return 1;
}

/** Convenience function to set an error indicator to indicate out of memory.
 * @param error The error indicator to set.
 * @return always returns error.
 */
static IloOplTableError setOOM(IloOplTableError error) {
  error->setBoth(error, -1, "out of memory");
  return error;
}

/** Check that a column is within range.
 * @param col     The column index to test.
 * @param columns The number of columns available.
 * @param error   In case the index is out of range this indicator will set
 *                to contain an appropriate code/message.
 * @return true if the index is within range, false otherwise.
 */
static int checkColumn(IloOplTableColIndex col,
                       IloOplTableColIndex columns,
                       IloOplTableError error)
{
  if ( col < 0 || col >= columns ) {
    error->setBoth(error, -1, "index %lld out of range [0,%lld]",
                   col, columns);
    return 0;
  }
  return 1;
}

/* ********************************************************************** *
 *                                                                        *
 *    Data input                                                          *
 *                                                                        *
 * ********************************************************************** */

/** Struct to represent the result of a query operation. */
typedef struct {
  IloOplTableInputRowsC base;
  sqlite3              *db;          /**< Database connection. */
  sqlite3_stmt         *stmt;        /**< Statement handle. */
  IloOplTableColIndex  cols;         /**< Number of columns in stmt. */
  char                 *buffer;      /**< Buffer for reading text fields. */
  size_t               bufcap;       /**< Capacity of buffer. */
  char const           **fieldNames; /**< Selected tuple fields. */
} SQLiteInputRows;

static IloOplTableError inGetColumnCount(IloOplTableInputRows rows,
                                         IloOplTableError error,
                                         IloOplTableColIndex *cols_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;

  (void)error;
  *cols_p = r->cols;

  return NULL;
}

static IloOplTableError inGetSelectedTupleFields(IloOplTableInputRows rows,
                                                 IloOplTableError error,
                                                 char *sep,
                                                 char const *const **fields_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;
  
  (void)error;
  if ( sep )
    *sep = TUPLE_SEPARATOR;
  *fields_p = r->fieldNames;
  return NULL;
}

static IloOplTableError inReadInt(IloOplTableInputRows rows,
                                  IloOplTableError error,
                                  IloOplTableColIndex column,
                                  IloOplTableIntType *value_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  switch (sqlite3_column_type(r->stmt, column)) {
  case SQLITE_INTEGER:
    *value_p= sqlite3_column_int64(r->stmt, column);
    return NULL;
  default:
    error->setBoth(error, -1, "column %d cannot be used as integer", column);
  }
  return error;
}

static IloOplTableError inReadString(IloOplTableInputRows rows,
                                     IloOplTableError error,
                                     IloOplTableColIndex column,
                                     char const **value_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  switch (sqlite3_column_type(r->stmt, column)) {
  case SQLITE_TEXT:
    {
      /* Get size of string and make sure input buffer is large enough. */
      int const bytes = sqlite3_column_bytes(r->stmt, column);

      if ( bytes < 0 ) {
        error->setBoth(error, bytes, "cannot get length of column %d: %d %s",
                       column, bytes, sqlite3_errmsg(r->db));
        return error;
      }
      else {
        size_t required = (size_t)bytes;

        if ( required > r->bufcap ) {
          size_t newcap = r->bufcap * 2;

          free(r->buffer);
          r->buffer = NULL;
          r->bufcap = 0;
          if ( newcap < required )
            newcap = required;
          if ( newcap < 128 )
            newcap = 128;
          if ( (r->buffer = calloc(newcap, sizeof(*r->buffer))) == NULL )
            return setOOM(error);
          r->bufcap = newcap;
        }

        /* Buffer is guaranteed to be big enough, get the string from
         * the database.
         */
        memcpy(r->buffer, sqlite3_column_text(r->stmt, column), required);
        r->buffer[required] = '\0';
        *value_p= r->buffer;
        return NULL;
      }
    }
    break;
  default:
    error->setBoth(error, -1, "column %d cannot be used as string", column);
  }
  return error;
}

static IloOplTableError inReadNum(IloOplTableInputRows rows,
                                  IloOplTableError error,
                                  IloOplTableColIndex column,
                                  double *value_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  switch (sqlite3_column_type(r->stmt, column)) {
  case SQLITE_FLOAT:
    *value_p = sqlite3_column_double(r->stmt, column);
    return NULL;
  default:
    error->setBoth(error, -1, "column %d cannot be used as double", column);
  }
  return error;
}

static IloOplTableError inNext(IloOplTableInputRows rows,
                               IloOplTableError error, int *next_p)
{
  SQLiteInputRows *r = (SQLiteInputRows *)rows;
  int status = sqlite3_step(r->stmt);

  if ( status == SQLITE_DONE ) {
    *next_p = 0;
    return NULL;
  }
  else if ( status == SQLITE_ROW ) {
    *next_p = 1;
    return NULL;
  }
  else {
    error->setBoth(error, status, "failed to step row: %d %s",
                   status, sqlite3_errmsg(r->db));
    return error;
  }
}

static void inDestroy(SQLiteInputRows *rows)
{
  if ( rows ) {
    if ( rows->stmt )
      finalize(&rows->stmt);
    free(rows->buffer);
    if ( rows->fieldNames ) {
      IloOplTableColIndex col;

      for (col = 0; col < rows->cols; ++col)
        free((char *)rows->fieldNames[col]);
      free((void *)rows->fieldNames);
    }
    free(rows);
  }
}

/* ********************************************************************** *
 *                                                                        *
 *    Data output                                                         *
 *                                                                        *
 * ********************************************************************** */

/** Different states in the lifecycle of a transaction. */
typedef enum { NOTHING,
               START,
               ROLLBACK,
               COMMIT } Transaction;

/** Representation of an insert or update operation.
 */
typedef struct {
  IloOplTableOutputRowsC base;
  sqlite3                *db;    /**< Database connection. */
  sqlite3_stmt           *stmt;  /**< Statement handle. */
  IloOplTableColIndex    params; /**< Number of parameters in statement. */
  Transaction            trans;  /**< Next operation in transaction lifecycle. */
  char const             **fieldNames; /**< Names of named parameters (if any). */

} SQLiteOutputRows;

/** Start, commit, rollback a transaction.
 * This functions performs the next operation on the transaction
 * life cycle of this statement.
 */
static IloOplTableError outTransaction(SQLiteOutputRows *rows,
                                       IloOplTableError error)
{
  switch (rows->trans) {
  case NOTHING: break;
  case START:
    if ( !exec(rows->db, "BEGIN TRANSACTION", error) )
      return error;
    rows->trans = ROLLBACK; /* next operation is COMMIT only on success */
    break;
  case ROLLBACK:
    (void)exec(rows->db, "ROLLBACK TRANSACTION", error);
    rows->trans = NOTHING;
    break;
  case COMMIT:
    if ( !exec(rows->db, "END TRANSACTION", error) ) {
      rows->trans = ROLLBACK;
      return error;
    }
    rows->trans = NOTHING;
    break;
  }
  return NULL;
}

static IloOplTableError outGetSelectedTupleFields(IloOplTableOutputRows rows,
                                                  IloOplTableError error,
                                                  char *sep,
                                                  IloOplTableColIndex *cols,
                                                  char const *const **fields_p)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;

  (void)error;
  if ( sep )
    *sep = TUPLE_SEPARATOR;
  if ( cols )
    *cols = r->params;
  *fields_p = r->fieldNames;

  return NULL;
}

static IloOplTableError outWriteInt(IloOplTableOutputRows rows,
                                    IloOplTableError error,
                                    IloOplTableColIndex column,
                                    IloOplTableIntType value)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;
  int status;

  /* NOTE: The first parameter has index 1! */
  if ( !checkColumn(column, r->params, error) )
    return error;

  status = sqlite3_bind_int64(r->stmt, column + 1, value);
  if ( status ) {
    error->setBoth(error, status, "failed to bind int (%d): %s", status,
                   sqlite3_errmsg(r->db));
    return error;
  }
  return NULL;
}

static IloOplTableError outWriteString(IloOplTableOutputRows rows,
                                       IloOplTableError error,
                                       IloOplTableColIndex column,
                                       char const *value)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;
  size_t len;
  int status;

  /* NOTE: The first parameter has index 1! */
  if ( !checkColumn(column, r->params, error) )
    return error;

  len = strlen(value);
  status = sqlite3_bind_text(r->stmt, column + 1, value, (int)len,
                             SQLITE_TRANSIENT);
  if ( status ) {
    error->setBoth(error, status, "failed to bind string (%d): %s", status,
                   sqlite3_errmsg(r->db));
    return error;
  }
  return NULL;
}

static IloOplTableError outWriteNum(IloOplTableOutputRows rows,
                                    IloOplTableError error,
                                    IloOplTableColIndex column,
                                    double value)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;
  int status;

  /* NOTE: The first parameter has index 1! */
  if ( !checkColumn(column, r->params, error) )
    return error;

  status = sqlite3_bind_double(r->stmt, column + 1, value);
  if ( status ) {
    error->setBoth(error, status, "failed to bind double (%d): %s", status,
                   sqlite3_errmsg(r->db));
    return error;
  }
  return NULL;
}

static IloOplTableError outEndRow(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;
  int status = sqlite3_step(r->stmt);

  if ( status != SQLITE_DONE ) {
    error->setBoth(error, status, "failed to end row (%d): %s", status,
                   sqlite3_errmsg(r->db));
    return error;
  }

  status = sqlite3_reset(r->stmt);
  if ( status ) {
    error->setBoth(error, status, "failed to reset row (%d): %s", status,
                   sqlite3_errmsg(r->db));
    return error;
  }
  return NULL;
}

static IloOplTableError outCommit(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  SQLiteOutputRows *r = (SQLiteOutputRows *)rows;
  /* Finalize the statement so that subsequent attempts to bind
   * data to it will produce an error. Then complete the transaction.
   */
  finalize(&r->stmt);
  r->trans = COMMIT;
  return outTransaction(r, error);
}

static void outDestroy(SQLiteOutputRows *rows)
{
  if ( rows ) {
    if ( rows->fieldNames ) {
      IloOplTableColIndex param;

      for (param = 0; param < rows->params; ++param)
        free((char *)rows->fieldNames[param]);
      free((void *)rows->fieldNames);
    }
    finalize(&rows->stmt);
    outTransaction(rows, NULL); /* errors are ignored */
    free(rows);
  }
}


/* ********************************************************************** *
 *                                                                        *
 *    Connection handling                                                 *
 *                                                                        *
 * ********************************************************************** */

/** Database connection. */
typedef struct {
  IloOplTableConnectionC base;
  sqlite3                *db;       /**< Database connection. */
  int                    writeonly; /**< Is this an output-only connection? */
  int                    named;     /**< Are tuple fields addressed explicitly by name? */
} SQLiteConnection;

static void connDestroy(IloOplTableConnection conn)
{
  SQLiteConnection *c = (SQLiteConnection *)conn;

  if ( c ) {
    /* Note that we have to run on old machines that don't have
     * sqlite3_close_v2() yet. Moreover, we don't want to defer destruction,
     * so it should be fine to just use sqlite3_close().
     */
    sqlite3_close(c->db);
    free(c);
  }
}

static IloOplTableError connOpenInputRows(IloOplTableConnection conn,
                                          IloOplTableError error,
                                          IloOplTableContext context,
                                          char const *query,
                                          IloOplTableInputRows *rows_p)
{
  SQLiteConnection *c = (SQLiteConnection *)conn;
  IloOplTableError err = error;
  SQLiteInputRows *rows = NULL;
  char const *tail = NULL;
  int status;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->db = c->db;
  rows->base.getColumnCount = inGetColumnCount;
  rows->base.getSelectedTupleFields = inGetSelectedTupleFields;
  rows->base.readInt = inReadInt;
  rows->base.readString = inReadString;
  rows->base.readNum = inReadNum;
  rows->base.next = inNext;

  status = sqlite3_prepare_v2(rows->db, query, (int)strlen(query),
                              &rows->stmt, &tail);
  if ( tail && *tail ) {
    error->setBoth(error, -1, "query '%s' has trailing garbage: %s",
                   query, tail);
    goto TERMINATE;
  }

  if ( status ) {
    error->setBoth(error, status, "query '%s' failed: %s", query,
                   sqlite3_errmsg(rows->db));
    goto TERMINATE;
  }

  rows->cols = sqlite3_column_count(rows->stmt);
  if ( rows->cols < 0 ) {
    error->setBoth(error, -1, "failed to get column count: %s",
                   sqlite3_errmsg(rows->db));
    goto TERMINATE;
  }

  /* If tuples are loaded using named fields (i.e., with an AS
   * tag in the SELECT statement) then load the names of the fields
   * now.
   */
  if ( c->named ) {
    IloOplTableColIndex col;

    if ( (rows->fieldNames = calloc(rows->cols, sizeof(*rows->fieldNames))) == NULL ) {
      setOOM(error);
      goto TERMINATE;
    }

    for (col = 0; col < rows->cols; ++col) {
      char const *name = sqlite3_column_name(rows->stmt, col);
      if ( !name ) {
        setOOM(error);
        goto TERMINATE;
      }
      if ( (rows->fieldNames[col] = STRDUP(name)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
    }
  }
  *rows_p = &rows->base;
  rows = NULL;
  err = NULL;

 TERMINATE:
  inDestroy(rows);
  return err;
}

static void connCloseInputRows(IloOplTableConnection conn,
                               IloOplTableInputRows rows)
{
  (void)conn;
  inDestroy((SQLiteInputRows *)rows);
}

static IloOplTableError connOpenOutputRows(IloOplTableConnection conn,
                                           IloOplTableError error,
                                           IloOplTableContext context,
                                           char const *query,
                                           IloOplTableOutputRows *rows_p)
{
  SQLiteConnection *c = (SQLiteConnection *)conn;
  IloOplTableError err = error;
  SQLiteOutputRows *rows = NULL;
  char const *tail = NULL;
  int status;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->db = c->db;
  rows->base.getSelectedTupleFields = outGetSelectedTupleFields;
  rows->base.writeInt = outWriteInt;
  rows->base.writeString = outWriteString;
  rows->base.writeNum = outWriteNum;
  rows->base.endRow = outEndRow;
  rows->base.commit = outCommit;
  rows->trans = START;

  if ( outTransaction(rows, error) != NULL )
    goto TERMINATE;

  status = sqlite3_prepare_v2(rows->db, query, (int)strlen(query),
                              &rows->stmt, &tail);
  if ( tail && *tail ) {
    error->setBoth(error, -1, "update '%s' has trailing garbage: %s",
                   query, tail);
    goto TERMINATE;
  }

  if ( status ) {
    error->setBoth(error, status, "update '%s' failed: %s", query,
                   sqlite3_errmsg(rows->db));
    goto TERMINATE;
  }

  /* NOTE: We assume that parameters are NOT bound using the ?NNN or
   *       :NNN syntax with gaps.
   */
  rows->params = sqlite3_bind_parameter_count(rows->stmt);
  if ( rows->params < 0 ) {
    error->setBoth(error, -1, "failed to get parameter count: %s",
                   sqlite3_errmsg(rows->db));
    goto TERMINATE;
  }

  /* If tuple fields are selected by name then extract the names
   * of fields to be written now.
   */
  if ( c->named ) {
    IloOplTableColIndex param;

    if ( (rows->fieldNames = calloc(rows->params, sizeof(*rows->fieldNames))) == NULL )
      {
        setOOM(error);
        goto TERMINATE;
      }

    for (param = 0; param < rows->params; ++param) {
      char const *name = sqlite3_bind_parameter_name(rows->stmt, param + 1);

      if ( !name ) {
        error->setBoth(error, -1, "missing parameter name");
        goto TERMINATE;
      }
      /* SQLite returns the initial character as well. */
      if ( *name == ':' || *name == '$' || *name == '@' || *name == '?' )
        ++name;
      if ( (rows->fieldNames[param] = STRDUP(name)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
    }
  }

  *rows_p = &rows->base;
  rows = NULL;
  err = NULL;

 TERMINATE:

  outDestroy(rows);
  return err;
}

static void connCloseOutputRows(IloOplTableConnection conn,
                                IloOplTableOutputRows rows)
{
  (void)conn;
  outDestroy((SQLiteOutputRows *)rows);
}

static IloOplTableError connCreate(char const *connstr,
                                   char const *sql,
                                   int load,
                                   IloOplTableContext context,
                                   IloOplTableError error,
                                   IloOplTableConnection *conn_p)
{
  IloOplTableError err = error;
  SQLiteConnection *conn = NULL;
  IloOplTableArgs args = NULL;
  char *tmp = NULL;
  char const *q = strchr(connstr, '?');
  char const *path;

  if ( (conn = calloc(1, sizeof(*conn))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  conn->base.destroy = connDestroy;
  conn->base.openInputRows = connOpenInputRows;
  conn->base.closeInputRows = connCloseInputRows;
  conn->base.openOutputRows = connOpenOutputRows;
  conn->base.closeOutputRows = connCloseOutputRows;

  /* Extract URL-encoded arguments from the connection string. */
  if ( q ) {
#   define K_WRITEONLY "writeonly"
#   define K_NAMED     "named"
    char const *orig;

    args = context->parseArgs(context, q + 1, '&', '%');
    if ( args == NULL ) {
      error->setBoth(error, -1, "failed to parse '%s'", q + 1);
      goto TERMINATE;
    }

    orig = args->original(args, K_WRITEONLY, K_NAMED, NULL);
    if ( *orig ) {
      if ( (tmp = malloc((strlen(orig) + (q - connstr) + 2))) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
      memcpy(tmp, connstr, q - connstr);
      tmp[q - connstr] = 0;
      strcat(tmp, "?");
      strcat(tmp, orig);
      connstr = tmp;
    }
    else {
      if ( (tmp = malloc(q - connstr + 1)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
      memcpy(tmp, connstr, q - connstr);
      tmp[q - connstr] = 0;
      connstr = tmp;
    }

    if ( args->getBool(args, K_WRITEONLY, &conn->writeonly, &conn->writeonly) )
      {
        error->setBoth(error, -1, "failed to read 'writeonly' from '%s'", orig);
        goto TERMINATE;
      }
    if ( args->getBool(args, K_NAMED, &conn->named, &conn->named) )
      {
        error->setBoth(error, -1, "failed to read 'named' from '%s'", orig);
        goto TERMINATE;
      }
  }

  /* If the connection was opened to load data then resolve relative
   * file names with respect to the data source name.
   */
  path = connstr;
  if ( load ) {
    path = context->resolvePath(context, connstr);
    if ( path == NULL ) {
      error->setBoth(error, -1, "failed to resolve '%s'", connstr);
      goto TERMINATE;
    }
  }

  /* If writeonly and the connection is opened only for reading then
   * don't even attempt to connect to the database.
   */
  if ( !load || !conn->writeonly ) {
    int status = sqlite3_open_v2(path, &conn->db,
                                 (load ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE) |
                                 (load ? 0 : SQLITE_OPEN_CREATE) |
                                 SQLITE_OPEN_FULLMUTEX,
                                 NULL);
    if ( status != 0 ) {
      error->setBoth(error, status, "failed to connect to %s (%s): %d",
                     connstr, path, status);
      goto TERMINATE;
    }
  }

  if ( load ) {
    /* nothing to do */
  }
  else {
    /* Connection was opened for writing. Execute the second
     * argument to the *Connection() call as an SQL command.
     */
    if ( sql && *sql ) {
      if ( !exec(conn->db, sql, error) )
        goto TERMINATE;
    }
  }

  *conn_p = &conn->base;
  conn = NULL;
  err = NULL;

 TERMINATE:

  if ( conn )
    connDestroy(&conn->base);
  return err;
}

/* ********************************************************************** *
 *                                                                        *
 *    Factory and public constructor function                             *
 *                                                                        *
 * ********************************************************************** */

static int factoryRefCount = 0;

static IloOplTableError factoryConnect(char const *subId,
                                       char const *spec,
                                       int load,
                                       IloOplTableContext context,
                                       IloOplTableError error,
                                       IloOplTableConnection *conn_p)
{
  return connCreate(subId, spec, load, context, error, conn_p);
}

static void factoryIncRef(IloOplTableFactory fac)
{
  (void)fac;
  ++factoryRefCount;
}

static void factoryDecRef(IloOplTableFactory fac)
{
  (void)fac;
  assert(factoryRefCount > 0);
  --factoryRefCount;
}

static IloOplTableFactoryC const factory = {
  factoryConnect, factoryIncRef, factoryDecRef
};

/* This exports and defines the function that OPL will lookup in a
 * shared library when it encounters a "SQLiteConnection" statement
 * in a .dat file.
 */
ILO_TABLE_EXPORT ILOOPLTABLEDATAHANDLER(SQLite, error, factory_p)
{
  (void)error;
  *factory_p = (IloOplTableFactory)&factory; /* cast away 'const' */
  factoryIncRef(*factory_p);
  return NULL;
}

