/* --------------------------------------------------------------------------
 * File: libopltabMySQL.c
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
 *    MySQLConnection conn(...,...);  // connect to database
 *    data from MySQLRead(conn, "SELECT * FROM data");
 *    result to MySQLPublish(conn, "INSERT INTO results VALUES(?)");
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mysql.h>
#include <ilopl/data/iloopltabledatasource.h>

/* MySQL 8.0 and later no longer define the my_bool type.
 * This also means that the ABI changed (passing pointer to int rather
 * pointer to char now).
 */
typedef void BOOL_TYPE;

#define TUPLE_SEPARATOR '.' /** Separator in fully qualified names for
                             * fields in sub-tuples. */

#define MAX_STRING_LENGTH 4096 /** Maximum length of string values we can
                                * exchange with the database. Strings longer
                                * than this will result in errors.
                                */

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

/** Convenience method to check for errors and raise appropriate exceptions.
 * If <code>status</code> is non-zero then the function will throw an
 * error that is as verbose as possible.
 */
static int checkConn(MYSQL *con, int status,
                     char const *query, IloOplTableError error)
{
  if ( status ) {
    if ( query )
      error->setBoth(error, status, "%s: error %d (%d, %s)",
                     query, status, mysql_errno(con), mysql_error(con));
    else
      error->setBoth(error, status, "error %d (%d, %s)",
                     status, mysql_errno(con), mysql_error(con));
    return 0;
  }
  return 1;
}

/** Convenience method to check for errors and raise appropriate exceptions.
 * If <code>status</code> is non-zero then the function will throw an
 * error that is as verbose as possible.
 */
static int checkStmt(MYSQL_STMT *stmt, int status,
                     char const *query, IloOplTableError error)
{
  if ( status ) {
    if ( query )
      error->setBoth(error, status, "%s: error %d (%d, %s)",
                     query, status, mysql_stmt_errno(stmt),
                     mysql_stmt_error(stmt));
    else
      error->setBoth(error, status, "error %d (%d, %s)",
                     status, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
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

/* ********************************************************************** *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * ********************************************************************** */

typedef struct {
  MYSQL *con;      /**< Connection for the transaction. */
  int    running;  /**< Is a transaction running? */
} Transaction;

/** Complete a transaction.
 * If the transaction is still running then it will be rolled back.
 * In any case, the "auto commit" property of the respective connection
 * will be reset.
 * Any error in this method is ignored.
 */
static void transactionComplete(Transaction *trans)
{
  if ( trans->running ) {
    /* Not explicitly committed, so we must roll back the transaction. */
    mysql_rollback(trans->con);
    mysql_autocommit(trans->con, 1);
  }
}

/** Start transaction.
 * The function starts a transaction which is finished either explicitly
 * by calling transactionCommit() (which commits the data to the database)
 * or by implicitly by the destructor (which rolls back the transaction).
 */
static IloOplTableError transactionStart(Transaction *trans,
                                         IloOplTableError error)
{
  assert(!trans->running);
  if ( !checkConn(trans->con, mysql_autocommit(trans->con, 0), NULL, error) )
    return error;
  trans->running = 1;
  return NULL;
}

/** Commit a transaction that was previously started by start(). */
static IloOplTableError transactionCommit(Transaction *trans,
                                          IloOplTableError error)
{
  int ok;

  assert(trans->running);
  trans->running = 0;
  ok = checkConn(trans->con, mysql_commit(trans->con), NULL, error);
  (void)mysql_autocommit(trans->con, 1); /* re-enable auto-commit */
  return ok ? NULL : error;
}

/* ********************************************************************** *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * ********************************************************************** */

/** Column data.
 * Instances of this class provide the buffers that are used when binding
 * results or parameters to SQL statements.
 */
typedef struct {
  enum { T_NONE,    /**< Used to indicate that type was not yet set. */
         T_INT8,    /**< 8bit signed integer (use data.b as buffer). */
         T_INT16,   /**< 16bit signed integer (use data.s as buffer). */
         T_INT32,   /**< 32bit signed integer (use data.i as buffer). */
         T_INT64,   /**< 64bit signed integer (use data.l as buffer). */
         T_FLOAT,   /**< single precision float (use data.f as buffer). */
         T_DOUBLE,  /**< double precision float (use data.d as buffer). */
         STR      /**< NUL-terminated string (use data.t as buffer). */
  } type; /**< Type of the column. */
  union {
    int8_t  b;
    int16_t s;
    int32_t i;
    int64_t l;
    float   f;
    double  d;
    char    *t;
  } data; /**< The actual data buffer. */
  unsigned long len; /**< Number of bytes available in the buffer. */
  int       is_null; /**< Flag to indicate NULL values. */
} Data;

static IloOplTableError data2int(Data const *data, IloOplTableError error,
                                 IloOplTableIntType *value_p)
{
  switch (data->type) {
  case T_INT8: *value_p = data->is_null ? 0 : data->data.b; break;
  case T_INT16: *value_p = data->is_null ? 0 : data->data.s; break;
  case T_INT32: *value_p = data->is_null ? 0 : data->data.i; break;
  case T_INT64: *value_p = data->is_null ? 0 : data->data.l; break;
  default:
    error->setBoth(error, -1, "column is not integer");
    return error;
  }
  return NULL;
}

static IloOplTableError data2num(Data const *data, IloOplTableError error,
                                 double *value_p)
{
  switch (data->type) {
  case T_FLOAT: *value_p = data->is_null ? 0 : data->data.f; break;
  case T_DOUBLE: *value_p = data->is_null ? 0 : data->data.d; break;
  case T_INT8: *value_p = data->is_null ? 0 : data->data.b; break;
  case T_INT16: *value_p = data->is_null ? 0 : data->data.s; break;
  case T_INT32: *value_p = data->is_null ? 0 : data->data.i; break;
  case T_INT64: *value_p = data->is_null ? 0 : data->data.l; break;
  default:
    error->setBoth(error, -1, "column is not floating point");
    return error;
  }
  return NULL;
}

static IloOplTableError data2str(Data const *data, IloOplTableError error,
                                 char const **value_p)
{
  switch (data->type) {
  case STR:
    if ( data->is_null )
      *value_p = "";
    else if ( data->len >= MAX_STRING_LENGTH ) {
      error->setBoth(error, -1, "string too long");
      return error;
    }
    else {
      data->data.t[data->len] = 0;
      *value_p = data->data.t;
    }
    break;
  default:
    error->setBoth(error, -1, "column is not string");
    return error;
  }
  return NULL;
}


/* ********************************************************************** *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * ********************************************************************** */

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

typedef struct {
  IloOplTableInputRowsC base;
  MYSQL               *con;         /**< Connection. */
  MYSQL_STMT          *stmt;        /**< Statement handle for query. */
  MYSQL_RES           *res;         /**< Result handle. */
  IloOplTableColIndex cols;         /**< Number of columns in query. */
  char const          **fieldNames; /**< Names of fields. */
  Data                *data;        /**< Data buffer to exchange data with server. */
  MYSQL_BIND          *bind;        /**< Data binding information. */
} MySQLInputRows;

static IloOplTableError inGetColumnCount(IloOplTableInputRows rows,
                                         IloOplTableError error,
                                         IloOplTableColIndex *cols_p)
{
  MySQLInputRows *r = (MySQLInputRows *)rows;

  (void)error;
  *cols_p = r->cols;

  return NULL;
}

static IloOplTableError inGetSelectedTupleFields(IloOplTableInputRows rows,
                                                 IloOplTableError error,
                                                 char *sep,
                                                 char const *const **fields_p)
{
  MySQLInputRows *r = (MySQLInputRows *)rows;

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
  MySQLInputRows *r = (MySQLInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  return data2int(&r->data[column], error, value_p);
}

static IloOplTableError inReadString(IloOplTableInputRows rows,
                                     IloOplTableError error,
                                     IloOplTableColIndex column,
                                     char const **value_p)
{
  MySQLInputRows *r = (MySQLInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  return data2str(&r->data[column], error, value_p);
}

static IloOplTableError inReadNum(IloOplTableInputRows rows,
                                  IloOplTableError error,
                                  IloOplTableColIndex column,
                                  double *value_p)
{
  MySQLInputRows *r = (MySQLInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  return data2num(&r->data[column], error, value_p);
}

static IloOplTableError inNext(IloOplTableInputRows rows,
                               IloOplTableError error, int *next_p)
{
  MySQLInputRows *r = (MySQLInputRows *)rows;

  switch (mysql_stmt_fetch(r->stmt)) {
  case 0:
    *next_p = 1;
    return NULL;
  case 1:
    error->setBoth(error, mysql_stmt_errno(r->stmt),
                   "failed to get next row: %s (%u)",
                   mysql_stmt_error(r->stmt),
                   mysql_stmt_errno(r->stmt));
    break;
  case MYSQL_NO_DATA:
    *next_p = 0;
    return NULL;
  case MYSQL_DATA_TRUNCATED: /* truncation is error */
    error->setBoth(error, MYSQL_DATA_TRUNCATED, "data truncated");
    break;
  default:
    error->setBoth(error, mysql_stmt_errno(r->stmt),
                   "error while fetching next row: %s (%u)",
                   mysql_stmt_error(r->stmt), mysql_stmt_errno(r->stmt));
  }
  return error;
}

static void inDestroy(MySQLInputRows *rows)
{
  if ( rows ) {
    IloOplTableColIndex col;

    free(rows->bind);
    if ( rows->res )
      mysql_free_result(rows->res);
    if ( rows->stmt )
      mysql_stmt_close(rows->stmt);
    if ( rows->fieldNames ) {
      for (col = 0; col < rows->cols; ++col)
        free((char *)rows->fieldNames[col]);
      free((void *)rows->fieldNames);
    }
    if ( rows->data ) {
      for (col = 0; col < rows->cols; ++col)
        if ( rows->data[col].type == STR )
          free(rows->data[col].data.t);
      free(rows->data);
    }
    free(rows);
  }
}

/* ********************************************************************** *
 *                                                                        *
 *    Data output                                                         *
 *                                                                        *
 * ********************************************************************** */

typedef struct {
  IloOplTableOutputRowsC base;
  MYSQL                  *con;   /**< Database connection. */
  MYSQL_STMT             *stmt;  /**< Statement handle. */
  Data                   *data;  /**< Data that is bound to the statement. */
  MYSQL_BIND             *bind;  /**< Data mapping/binding. */
  IloOplTableColIndex    cols;   /**< Number of parameters in statement. */
  Transaction            trans;  /**< Transaction wrapping this update. */
} MySQLOutputRows;


static IloOplTableError outGetSelectedTupleFields(IloOplTableOutputRows rows,
                                                  IloOplTableError error,
                                                  char *sep,
                                                  IloOplTableColIndex *cols,
                                                  char const *const **fields_p)
{
  /* MySQL does not support named parameters. */
  (void)rows;
  (void)error;
  if ( sep )
    *sep = TUPLE_SEPARATOR;
  if ( cols )
    *cols = 0;
  *fields_p = NULL;
  return NULL;
}

static IloOplTableError outWriteInt(IloOplTableOutputRows rows,
                                    IloOplTableError error,
                                    IloOplTableColIndex column,
                                    IloOplTableIntType value)
{
  MySQLOutputRows *r = (MySQLOutputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;
  if ( r->data[column].type == T_NONE ) {
    r->data[column].type = T_INT64;
    r->bind[column].buffer_type = MYSQL_TYPE_LONGLONG;
    r->bind[column].buffer = &r->data[column].data.l;
    r->bind[column].is_null = 0;
    r->bind[column].length = 0;
  }
  r->data[column].data.l = value;
  return NULL;
}

static IloOplTableError outWriteString(IloOplTableOutputRows rows,
                                       IloOplTableError error,
                                       IloOplTableColIndex column,
                                       char const *value)
{
  MySQLOutputRows *r = (MySQLOutputRows *)rows;
  size_t len;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  if ( r->data[column].type == T_NONE ) {
    if ( (r->data[column].data.t = malloc(MAX_STRING_LENGTH)) == NULL )
      return setOOM(error);

    r->data[column].type = STR;
    r->bind[column].buffer_type = MYSQL_TYPE_STRING;
    r->bind[column].buffer = r->data[column].data.t;
    r->bind[column].is_null = 0;
    r->bind[column].length = &r->data[column].len;
  }
  /* NULL string is same as empty string */
  if ( !value )
    value = "";
  len = strlen(value);
  if ( len >= MAX_STRING_LENGTH ) {
    error->setBoth(error, -1, "string argument too long (max %llu)",
                   (unsigned long long)MAX_STRING_LENGTH);
    return error;
  }
  memcpy(r->data[column].data.t, value, len + 1);
  r->bind[column].buffer_length = (unsigned long)len;
  r->data[column].len = (unsigned long)len;
  return NULL;
}

static IloOplTableError outWriteNum(IloOplTableOutputRows rows,
                                    IloOplTableError error,
                                    IloOplTableColIndex column,
                                    double value)
{
  MySQLOutputRows *r = (MySQLOutputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;

  if ( r->data[column].type == T_NONE ) {
    r->data[column].type = T_DOUBLE;
    r->bind[column].buffer_type = MYSQL_TYPE_DOUBLE;
    r->bind[column].buffer = &r->data[column].data.d;
    r->bind[column].is_null = 0;
    r->bind[column].length = 0;
  }
  r->data[column].data.d = value;

  return NULL;
}

static IloOplTableError outEndRow(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  MySQLOutputRows *r = (MySQLOutputRows *)rows;

  if ( !checkStmt(r->stmt, mysql_stmt_bind_param(r->stmt, r->bind), NULL,
                  error) )
    return error;
  if ( !checkStmt(r->stmt, mysql_stmt_execute(r->stmt), NULL, error) )
    return error;
  if ( mysql_stmt_errno(r->stmt) ) {
    error->setBoth(error, mysql_stmt_errno(r->stmt),
                   "update failed: %s (%u)",
                   mysql_stmt_error(r->stmt), mysql_stmt_errno(r->stmt));
    return error;
  }
  return NULL;
}

static IloOplTableError outCommit(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  MySQLOutputRows *r = (MySQLOutputRows *)rows;

  return transactionCommit(&r->trans, error);
}

static void outDestroy(MySQLOutputRows *rows)
{
  if ( rows ) {
    if ( rows->stmt )
      mysql_stmt_close(rows->stmt);

    if ( rows->bind )
      free(rows->bind);
    if ( rows->data ) {
      IloOplTableColIndex col;
      for (col = 0; col < rows->cols; ++col)
        if ( rows->data[col].type == STR )
          free(rows->data[col].data.t);
      free(rows->data);
    }
    transactionComplete(&rows->trans);
    free(rows);
  }
}


/* ********************************************************************** *
 *                                                                        *
 *    Connection handling                                                 *
 *                                                                        *
 * ********************************************************************** */

typedef struct {
  IloOplTableConnectionC base;
  MYSQL                  *db;   /**< Database connection. */
  int                    named; /**< Are tuple fields addressed explicitly by name? */
} MySQLConnection;

static void connDestroy(IloOplTableConnection conn)
{
  MySQLConnection *c = (MySQLConnection *)conn;

  if ( c ) {
    /* Note that we have to run on old machines that don't have
     * sqlite3_close_v2() yet. Moreover, we don't want to defer destruction,
     * so it should be fine to just use sqlite3_close().
     */
    mysql_close(c->db);
    free(c);
  }
}

static IloOplTableError connOpenInputRows(IloOplTableConnection conn,
                                          IloOplTableError error,
                                          IloOplTableContext context,
                                          char const *query,
                                          IloOplTableInputRows *rows_p)
{
  MySQLConnection *c = (MySQLConnection *)conn;
  MYSQL *db = c->db;
  IloOplTableError err = error;
  MySQLInputRows *rows = NULL;
  IloOplTableColIndex col;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->con = db;
  rows->base.getColumnCount = inGetColumnCount;
  rows->base.getSelectedTupleFields = inGetSelectedTupleFields;
  rows->base.readInt = inReadInt;
  rows->base.readString = inReadString;
  rows->base.readNum = inReadNum;
  rows->base.next = inNext;

  /* Prepare a statement with the query and extract all required
   * meta information from it.
   */
  rows->stmt = mysql_stmt_init(db);
  if ( !rows->stmt ) {
    error->setBoth(error, mysql_errno(db),
                   "failed to init statement: %s (%u)",
                   mysql_error(db), mysql_errno(db));
    goto TERMINATE;
  }
  if ( !checkStmt(rows->stmt, mysql_stmt_prepare(rows->stmt, query,
                                                 (unsigned long)strlen(query)),
                  query, error) )
    goto TERMINATE;

  rows->res = mysql_stmt_result_metadata(rows->stmt);
  if ( !rows->res ) {
    error->setBoth(error, mysql_errno(db),
                   "%s: failed prepare result: %s (%u)",
                   query, mysql_error(db), mysql_errno(db));
    goto TERMINATE;
  }

  rows->cols = mysql_num_fields(rows->res);
  if ( (rows->data = calloc(rows->cols, sizeof(*rows->data))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }
  if ( (rows->bind = calloc(rows->cols, sizeof(*rows->bind))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  if ( c->named ) {
    if ( (rows->fieldNames = calloc(rows->cols, sizeof(*rows->fieldNames))) == NULL ) {
      setOOM(error);
      goto TERMINATE;
    }
  }

  /* Get the names and types of fields.
   * We need the types to decide how to decode the string values
   * that are read from the database.
   */
  for (col = 0; col < rows->cols; ++col) {
    MYSQL_FIELD const *field = mysql_fetch_field(rows->res);

    if ( c->named ) {
      if ( (rows->fieldNames[col] = STRDUP(field->name)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
    }

    rows->bind[col].buffer_type = field->type;
    /* NOTE: The cast to BOOL_TYPE is a mild hack here.
     *       Before MySQL version 8 the is_null field had type 'my_bool'
     *       which was essentially 'char'. Since 8 the field has type 'bool'
     *       and users were requested to use 'int' or 'bool'. We always define
     *       the field in data to be of type 'int' and cast the pointer
     *       to the required type. Since an 'int' is wider than a 'char' this
     *       should work for both types. Moreover, we only test whether that
     *       int is non-zero or not, so the actual value does not matter, we
     *       only need the library to modify a single bit in the value pointed
     *       to in order to indicate that a field is NULL.
     */
    rows->bind[col].is_null = (BOOL_TYPE *)&rows->data[col].is_null;
    rows->bind[col].length = &rows->data[col].len;
    switch (field->type) {
    case MYSQL_TYPE_TINY:
      rows->bind[col].buffer = &rows->data[col].data.b;
      rows->data[col].type = T_INT8;
      break;
    case MYSQL_TYPE_SHORT:
      rows->bind[col].buffer = &rows->data[col].data.s;
      rows->data[col].type = T_INT16;
      break;
    case MYSQL_TYPE_INT24: /* fallthrough */
    case MYSQL_TYPE_LONG:
      rows->bind[col].buffer = &rows->data[col].data.i;
      rows->data[col].type = T_INT32;
      break;
    case MYSQL_TYPE_LONGLONG:
      rows->bind[col].buffer = &rows->data[col].data.l;
      rows->data[col].type = T_INT64;
      break;
    case MYSQL_TYPE_FLOAT:
      rows->bind[col].buffer = &rows->data[col].data.f;
      rows->data[col].type = T_FLOAT;
      break;
    case MYSQL_TYPE_DOUBLE:
      rows->bind[col].buffer = &rows->data[col].data.d;
      rows->data[col].type = T_DOUBLE;
      break;
    case MYSQL_TYPE_VARCHAR:    /* fallthrough */
    case MYSQL_TYPE_VAR_STRING: /* fallthrough */
    case MYSQL_TYPE_STRING:
      if ( (rows->data[col].data.t = malloc(MAX_STRING_LENGTH)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
      rows->data[col].data.t[MAX_STRING_LENGTH - 1] = '\0';
      rows->data[col].type = STR;
      rows->bind[col].buffer = rows->data[col].data.t;
      rows->bind[col].buffer_length = MAX_STRING_LENGTH - 1;
      break;
    default:
      error->setBoth(error, (int)field->type,
                     "%s: cannot handle type %d of field %d",
                     query, (int)field->type, (int)col);
      goto TERMINATE;
    }
  }

  /* Finally execute the statement and bind results. */
  if ( !checkStmt(rows->stmt, mysql_stmt_execute(rows->stmt), query, error) )
    goto TERMINATE;
  if ( !checkStmt(rows->stmt, mysql_stmt_bind_result(rows->stmt, rows->bind),
                  query, error) )
    goto TERMINATE;

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
  inDestroy((MySQLInputRows *)rows);
}

static IloOplTableError connOpenOutputRows(IloOplTableConnection conn,
                                           IloOplTableError error,
                                           IloOplTableContext context,
                                           char const *query,
                                           IloOplTableOutputRows *rows_p)
{
  MySQLConnection *c = (MySQLConnection *)conn;
  MYSQL *db = c->db;
  IloOplTableError err = error;
  MySQLOutputRows *rows = NULL;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->con = db;
  rows->base.getSelectedTupleFields = outGetSelectedTupleFields;
  rows->base.writeInt = outWriteInt;
  rows->base.writeString = outWriteString;
  rows->base.writeNum = outWriteNum;
  rows->base.endRow = outEndRow;
  rows->base.commit = outCommit;
  rows->trans.con = db;

  /* Wrap the execution of this statement into a transaction */
  if ( transactionStart(&rows->trans, error) != NULL )
    goto TERMINATE;

  /* Prepare a statement and data binding.
   * Note that the actual binding and execution happens in function
   * next(). Also, the binding information is initialized lazily in
   * functions writeInt(), writeNum(), and writeString().
   */
   rows->stmt = mysql_stmt_init(db);
   if ( !rows->stmt ) {
     error->setBoth(error, mysql_errno(db),
                    "failed to init statement: %s (%u)",
                    mysql_error(db), mysql_errno(db));
     goto TERMINATE;
   }

   if ( !checkStmt(rows->stmt, mysql_stmt_prepare(rows->stmt, query,
                                                  (unsigned long)strlen(query)),
                   query, error) )
     goto TERMINATE;

   rows->cols = mysql_stmt_param_count(rows->stmt);
   if ( (rows->bind = calloc(rows->cols, sizeof(*rows->bind))) == NULL ) {
     setOOM(error);
     goto TERMINATE;
   }
   if ( (rows->data = calloc(rows->cols, sizeof(*rows->data))) == NULL ) {
     setOOM(error);
     goto TERMINATE;
   }

#if 0
   /* mysql_stmt_param_metadata() is currently documented as
    * <quote>This function currently does nothing.</quote>
    * So we have no way to obtain names of named parameters or anything.
    * Hence we cannot support named parameters (at least I don't see
    * how without parsing the SQL ourselves).
         MYSQL_RES *res = mysql_stmt_param_metadata(stmt);
         mysql_free_result(res);
    */
#endif

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
  outDestroy((MySQLOutputRows *)rows);
}

static char *getString(IloOplTableArgs args, char const *key,
                       IloOplTableError error)
{
  char const *value;
  char *ret;

  if ( !args->contains(args, key) ) {
    error->setBoth(error, -1, "no %s for mysql connection", key);
    return NULL;
  }
  args->getString(args, key, NULL, &value);
  if ( (ret = STRDUP(value)) == NULL ) {
    setOOM(error);
    return NULL;
  }
  return ret;
}

static IloOplTableError connCreate(char const *connstr,
                                   char const *sql,
                                   int load,
                                   IloOplTableContext context,
                                   IloOplTableError error,
                                   IloOplTableConnection *conn_p)
{
  IloOplTableError err = error;
  MySQLConnection *conn = NULL;
  IloOplTableArgs args = NULL;
  char *hostname = NULL, *database = NULL, *username = NULL, *password = NULL;
  long port = 0; /* default port is 0 */

  if ( (conn = calloc(1, sizeof(*conn))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  conn->base.destroy = connDestroy;
  conn->base.openInputRows = connOpenInputRows;
  conn->base.closeInputRows = connCloseInputRows;
  conn->base.openOutputRows = connOpenOutputRows;
  conn->base.closeOutputRows = connCloseOutputRows;

  /* Extract arguments from the connection string.
   * The connection string is assumed to be a semi-colon separated
   * list of name/value pairs. Characters can be escaped using the %-sign.
   */
  args = context->parseArgs(context, connstr, ';', '%');
  if ( args == NULL ) {
    error->setBoth(error, -1, "failed to parse connection string '%s'",
                   connstr);
    goto TERMINATE;
  }
# define K_NAMED     "named"
# define K_HOSTNAME  "hostname"
# define K_DATABASE  "database"
# define K_USERNAME  "username"
# define K_PASSWORD  "password"
# define K_PORT      "port"
  /* This must be set to true if SELECT statements contain AS clauses
   * to explicitly set fields in tuples.
   */
  if ( args->getBool(args, K_NAMED, &conn->named, &conn->named) != 0 ) {
    error->setBoth(error, -1, "failed to get 'named' from connection string");
    goto TERMINATE;
  }
  if ( (hostname = getString(args, K_HOSTNAME, error)) == NULL )
    goto TERMINATE;
  if ( (database = getString(args, K_DATABASE, error)) == NULL )
    goto TERMINATE;
  if ( (username = getString(args, K_USERNAME, error)) == NULL )
    goto TERMINATE;
  if ( (password = getString(args, K_PASSWORD, error)) == NULL )
    goto TERMINATE;

  if ( args->getInt(args, K_PORT, &port, &port) != 0 ) {
    error->setBoth(error, -1, "failed to get port from connection string");
    goto TERMINATE;
  }


  /* Initialize connection and connect to database.
   * We connect with CLIENT_MULTI_STATEMENTS so that the command in
   * sql can contain multiple semi-colon separated statements.
   */
  conn->db = mysql_init(0);
  if ( !conn->db ) {
    error->setBoth(error, -1, "failed to init mysql: %s",
                   mysql_error(conn->db));
    goto TERMINATE;
  }

  if ( !mysql_real_connect(conn->db, hostname, username,
                           password, database,
                           port, NULL /* unix_socket */,
                           CLIENT_MULTI_STATEMENTS) )
  {
    error->setBoth(error, mysql_errno(conn->db),
                   "failed to connect mysql: %s", mysql_error(conn->db));
    goto TERMINATE;
  }

  /* If the connection is opened for publishing and there is an extra
   * SQL command to execute then run this now.
   */
  if ( !load && sql && *sql ) {
    int loop = 1;

    if ( !checkConn(conn->db, mysql_query(conn->db, sql), sql, error) )
      goto TERMINATE;
    while (loop) {
      MYSQL_RES *res = mysql_store_result(conn->db);
      if ( res )
        mysql_free_result(res);
      else if ( mysql_field_count(conn->db) == 0 )
        (void)mysql_affected_rows(conn->db);
      else {
        error->setBoth(error, mysql_errno(conn->db),
                       "%s: failed: %s (%u)",
                       sql, mysql_error(conn->db), mysql_errno(conn->db));
        goto TERMINATE;
      }

      switch (mysql_next_result(conn->db)) {
      case 0: /* successful, more results. */ continue;
      case -1: /* successful, no more results */ loop = 0; break;
      default: /* error */
        error->setBoth(error, mysql_errno(conn->db),
                       "%s: failed to iterate results: %s (%u)",
                       sql, mysql_error(conn->db), mysql_errno(conn->db));
        goto TERMINATE;
      }
    }
  }

  *conn_p = &conn->base;
  conn = NULL;
  err = NULL;

 TERMINATE:
  free(password);
  free(username);
  free(database);
  free(hostname);

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
 * shared library when it encounters a "MySQLConnection" statement
 * in a .dat file.
 */
ILO_TABLE_EXPORT ILOOPLTABLEDATAHANDLER(MySQL, error, factory_p)
{
  (void)error;
  *factory_p = (IloOplTableFactory)&factory; /* cast away 'const' */
  factoryIncRef(*factory_p);
  return NULL;
}
