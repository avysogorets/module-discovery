/* --------------------------------------------------------------------------
 * File: libopltabODBC.c
 * --------------------------------------------------------------------------
 * Licensed Materials - Property of IBM
 *
 * 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
 * Copyright IBM Corporation 2019, 2024. All Rights Reserved.
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
 *    ODBCConnection conn(...,...);  // connect to database
 *    data from ODBCRead(conn, "SELECT * FROM data");
 *    result to ODBCPublish(conn, "INSERT INTO results VALUES(?)");
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <ilopl/data/iloopltabledatasource.h>

#define TUPLE_SEPARATOR '.' /** Separator in fully qualified names for
                             * fields in sub-tuples.
							 */

#define MAX_STRING_LENGTH 4096 /** Maximum length of string values we can
                                * exchange with the database. Strings longer
                                * than this will result in errors.
                                */

#define MAX_NAME_LEN    1024   /** Maximum length of a column name.
                                * Columns with names longer than this
                                * will cause trouble.
                                */

static int verbose = 0; /** If 1, the connector will display verbose information */

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

/** Cast a C-Style string to type SQLCHAR.
 * This may involve casting away 'const' and changing the signedness
 * of the type.
 */
static SQLCHAR *sqlchar(char const *text) { return (SQLCHAR *)text; }


/** Check the result of an ODBC function call.
 * If status indicates failure then error is set appropriately.
 * @return true on success, false otherwise.
 */
static int check(long status, IloOplTableError error) {
  if ( status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO ) {
    error->setBoth(error, (int)status, "SQL failed with %ld", status);
    return 0;
  }
  return 1;
}

/**
 * @return true if the status is ok
 */
static int isOk(long status) {
	return ( status == SQL_SUCCESS ) || ( status == SQL_SUCCESS_WITH_INFO ) || ( status == SQL_NO_DATA);
}

/**
 * @return true if the param status is ok.
 */
static int isOkParamStatus(long status) {
    return (status == SQL_PARAM_SUCCESS) || (status == SQL_PARAM_SUCCESS_WITH_INFO) || (status == SQL_PARAM_UNUSED);
}

static char error_message_buf[2048];

/**
 * Given the handle type, handle and status, format an error message suitable for the error.
 * The error message if formated into buf using at most buf_len-1 bytes (non counting the null character)
 * @return a string allocated in an internal buffer
 */
static char* formatErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle,
							  long status)
{
	char buffer[1024];
	SQLCHAR state[6];
    SQLINTEGER nativeError = 0;
    SQLSMALLINT textlen = 0;
    if ( SQLGetDiagRec(handleType, handle, 1, state, &nativeError,
                       sqlchar(buffer), sizeof(buffer), &textlen)
        != SQL_SUCCESS )
    {
      sprintf(buffer, "unknown sql error %ld", status);
    }
    else {
      if ( textlen >= sizeof(buffer) )
        textlen = sizeof(buffer) - 1;
      buffer[textlen] = 0;
    }
	snprintf(error_message_buf, sizeof(error_message_buf), "%s (%ld, native %lld)", buffer, status, (long long)nativeError);
  error_message_buf[sizeof(error_message_buf)-1] = '\0';
  return error_message_buf;
}

/** More verbose check() function.
 * A potential exception message also contains the query that triggered
 * the error and a verbose error description obtained from the driver.
 * @return true on success, false otherwise.
 */
static int checkExt(SQLSMALLINT handleType, SQLHANDLE handle,
                    long status, char const *query, IloOplTableError error)
{
  if ( status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO ) {
    char buffer[1024];
    SQLCHAR state[6];
    SQLINTEGER nativeError = 0;
    SQLSMALLINT textlen = 0;
    if ( SQLGetDiagRec(handleType, handle, 1, state, &nativeError,
                       sqlchar(buffer), sizeof(buffer), &textlen)
        != SQL_SUCCESS )
    {
      sprintf(buffer, "unknown sql error %ld", status);
    }
    else {
      if ( textlen >= sizeof(buffer) )
        textlen = sizeof(buffer) - 1;
      buffer[textlen] = 0;
    }
    if ( query )
      error->setBoth(error, (int)status, "During the handling of %s: %s (%ld, native %lld)", query,
                     buffer, status, (long long)nativeError);
    else
      error->setBoth(error, (int)status, "%s (%ld, native %lld)",
                     buffer, status, (long long)nativeError);
    return 0;
  }
  return 1;
}

#define checkENV(h,s,q,e) checkExt(SQL_HANDLE_ENV, (h), (s), (q), (e))
#define checkDBC(h,s,q,e) checkExt(SQL_HANDLE_DBC, (h), (s), (q), (e))
#define checkSTMT(h,s,q,e) checkExt(SQL_HANDLE_STMT, (h), (s), (q), (e))

static int exec(SQLHDBC dbc, char const *sql, IloOplTableError error)
{
  while (*sql && isspace(*sql)) ++sql;
  if ( *sql ) {
    SQLHSTMT stmt;
    int ok;

    if ( !check(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), error) )
      return 0;
    ok = checkSTMT(stmt, SQLExecDirect(stmt, sqlchar(sql), SQL_NTS),
                   sql, error);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return ok;
  }
  return 0;
}

/** Convenience function to set an error indicator to indicate out of memory.
 * @param error The error indicator to set.
 * @return always returns error.
 */
static IloOplTableError setOOM(IloOplTableError error) {
  error->setBoth(error, -1, "out of memory");
  return error;
}

static int logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    return verbose ? vfprintf(stderr, format, args) : 0;
}

/* ********************************************************************** *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * ********************************************************************** */

typedef struct _Transaction {
  SQLHDBC dbc;    /**< Connection for the transaction (SQL_HANDLE_DBC). */
  int     running; /**< Is a transaction running? */
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
    /* Not explicitly committed -> we must rollback. */
    SQLEndTran(SQL_HANDLE_DBC, trans->dbc, SQL_ROLLBACK);
    SQLSetConnectAttr(trans->dbc, SQL_ATTR_AUTOCOMMIT,
                      (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);
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
  if ( !checkDBC(trans->dbc,
                 SQLSetConnectAttr(trans->dbc, SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                   SQL_NTS), NULL, error) )
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

  ok = checkDBC(trans->dbc, SQLEndTran(SQL_HANDLE_DBC, trans->dbc,
                                          SQL_COMMIT), NULL, error);
  (void)SQLSetConnectAttr(trans->dbc, SQL_ATTR_AUTOCOMMIT,
                          (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);
  return ok ? NULL : error;
}

/* ********************************************************************** *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * ********************************************************************** */

typedef enum _Type {
  T_NONE,   /**< Marks types that are not yet initialized. */
  T_INT8,   /**< 8bit signed integer, uses data.b as buffer. */
  T_INT16,  /**< 16bit signed integer, uses data.s as buffer. */
  T_INT32,  /**< 32bit signed integer, uses data.i as buffer. */
  T_INT64,  /**< 64bit signed integer, uses data.l as buffer. */
  T_FLOAT,  /**< Single precision float, uses data.f as buffer. */
  T_DOUBLE, /**< Double precision float, uses data.d as buffer. */
  T_TEXT    /**< NUL terminated string, uses data.s as buffer. */
} Type;     /**< Column types. */

/** Descriptor for a single column in a query or update.
 * This specifies the data type of the column as well as a buffer that
 * is used to exchange data for this column with ODBC.
 */
typedef struct _Column {
  Type type;     /**< Type of column. */
  union {
    int8_t  b;
    int16_t s;
    int32_t i;
    int64_t l;
    float   f;
    double  d;
    char    *t;
  } data;        /**< Data that is bound to columns. */
  SQLLEN l;      /**< For the last argument to SQLBindCol(). */
} Column;

typedef struct _ResultArray {
    Type type;         /* type of column */
    size_t array_size; /* size of the array (== batch size) */
    size_t data_size;  /* size of one data */
    void* data;        /* data. `data_size` * `array_size` */
    SQLLEN* l;         /* Length of each data */
} ResultArray;


static char* asTextPtr(ResultArray* results, size_t index) {
    return (char*)results->data + index * results->data_size;
}
static int8_t asInt8(ResultArray* results, size_t index) {
    return ((int8_t*)results->data)[index];
}
static int16_t asInt16(ResultArray* results, size_t index) {
    return ((int16_t*)results->data)[index];
}
static int32_t asInt32(ResultArray* results, size_t index) {
    return ((int32_t*)results->data)[index];
}
static void setInt32(ResultArray* results, size_t index, int32_t value) {
    ((int32_t*)results->data)[index] = value;
}
static int64_t asInt64(ResultArray* results, size_t index) {
    return ((int64_t*)results->data)[index];
}
static void setInt64(ResultArray* results, size_t index, int64_t value) {
    ((int64_t*)results->data)[index] = value;
}
static float asFloat(ResultArray* results, size_t index) {
    return ((float*)results->data)[index];
}
static double asDouble(ResultArray* results, size_t index) {
    return ((double*)results->data)[index];
}
static void setDouble(ResultArray* results, size_t index, double value) {
    ((double*)results->data)[index] = value;
}
static size_t getDataSize(Type type) {
    switch (type) {
    case T_INT8:
        return sizeof(int8_t);
    case T_INT16: 
        return sizeof(int16_t);
    case T_INT32:
        return sizeof(int32_t);
    case T_INT64:
        return sizeof(int64_t);
    case T_FLOAT:
        return sizeof(float);
    case T_DOUBLE: 
        return sizeof(double);
    case T_TEXT:  
        return sizeof(MAX_STRING_LENGTH);
    default:
        return 0;
    }
}
/* Returns the Type corresponding to the sqlType
 * Returns None if there is no corresponding type
 */
static Type getDataTypeFromSQLDataType(SQLSMALLINT sqlType) {
    Type datatype = T_NONE;
    switch (sqlType) {
    case SQL_TINYINT:
        datatype = T_INT8;
        break;
    case SQL_SMALLINT:
        datatype = T_INT16;
        break;
    case SQL_INTEGER:
        datatype = T_INT32;
        break;
    case SQL_BIGINT:
        datatype = T_INT64;
        break;
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
        datatype = T_DOUBLE;
        break;
    case SQL_REAL:
        datatype = T_FLOAT;
        break;
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        datatype = T_TEXT;
        break;
    default:
        goto TERMINATE;
    }
TERMINATE:
    return datatype;
}

/*
 * Free all buffers from the ResultArray, but do not free the result array itself
 */
static void deleteResultArrayData(ResultArray* results) {
    if (results != NULL) {
        if (results->data != NULL) {
            free(results->data);
            results->data = NULL;
        }
        if (results->l != NULL) {
            free(results->l);
            results->l = NULL;
        }
    }
}

/**
 * Initialize ResultArray, returning 0 if everything is ok
 */
static int initResultArray(ResultArray* results,
    Type type,
    size_t batch_size,
    IloOplTableError error)
{
    results->type = type;
    results->array_size = batch_size;
    results->data_size = getDataSize(type);
    if (results->type != T_TEXT) {
        if ((results->data = calloc(results->array_size, results->data_size)) == NULL) {
            setOOM(error);
            goto TERMINATE;
        }
    }
    else {
        /* data array for text will be allocated in prepareAndBindColsBatched */
        results->data = NULL;
    }
    if ((results->l = calloc(results->array_size, sizeof(*results->l))) == NULL) {
        setOOM(error);
        goto TERMINATE;
    } 
    return 0;
TERMINATE:
    return -1;
}


/**
 * How are OPL integers (64 bits int) converted ?
 * We use an enum here so that users can modify this sample to handle more conversion types.
 */
typedef enum _IntConversion {
  CONVERT_NONE,  /**< No conversion of output int types. */
  CONVERT_INT    /**< Converted to int32 (SQL_C_SLONG / SQL_INTEGER) */
} IntConversion;

static IloOplTableError col2int(Column const *col, IloOplTableError error,
                                IloOplTableIntType *value_p) {
  SQLLEN r = 0;

  switch (col->type) {
  case T_INT8:
    r = 1;
    *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.b;
    break;
  case T_INT16:
    r = 2;
    *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.s;
    break;
  case T_INT32:
    r = 4;
    *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.i;
    break;
  case T_INT64:
    r = 8;
    *value_p = (col->l == SQL_NULL_DATA) ? 0LL : col->data.l;
    break;
  case T_FLOAT:
    r = 4;
    *value_p = (col->l == SQL_NULL_DATA) ? 0: (int32_t)col->data.f;
    break;
  case T_DOUBLE:
    r = 8;
    *value_p = (col->l == SQL_NULL_DATA) ? 0: (int64_t)col->data.d;
    break;
  default:
    error->setBoth(error, -1, "column is not integer");
    return error;
  }
  if ( col->l != SQL_NULL_DATA && col->l < r ) {
    error->setBoth(error, -1, "not enough data for int column %lld vs %lld",
                   (long long)col->l, (long long)r);
    return error;
  }
  return NULL;
}

static IloOplTableError col2num(Column const *col, IloOplTableError error,
                           double *value_p)
{
  SQLLEN r;

  switch (col->type) {
    case T_INT8:
      r = 1;
      *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.b;
      break;
    case T_INT16:
      r = 2;
      *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.s;
      break;
    case T_INT32:
      r = 4;
      *value_p = (col->l == SQL_NULL_DATA) ? 0 : col->data.i;
      break;
    case T_INT64:
      r = 8;
      *value_p = (col->l == SQL_NULL_DATA) ? 0LL : col->data.l;
      break;    
  case T_DOUBLE:
    r = 8;
    *value_p = (col->l == SQL_NULL_DATA) ? 0.0 : col->data.d;
    break;
  case T_FLOAT:
    r = 4;
    *value_p = (col->l == SQL_NULL_DATA) ? 0.0 : col->data.f;
    break;
  default:
    error->setBoth(error, -1, "column is not floating point");
    return error;
  }
  if ( col->l != SQL_NULL_DATA && col->l < r ) {
    error->setBoth(error, -1, "not enough data for double column %lld vs %lld",
                   (long long)col->l, (long long)r);
    return error;
  }
  return NULL;
}

static IloOplTableError col2str(Column const *col, IloOplTableError error,
                           char const **value_p)
{
  switch (col->type) {
  case T_TEXT:
    col->data.t[col->l] = '\0';
    *value_p = (col->l == SQL_NULL_DATA) ? "" : col->data.t;
    break;
  default:
    error->setBoth(error, -1, "column is not text");
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
 * @param query   The query for this check - helps having nice error messages
 * @return true if the index is within range, false otherwise.
 */
static int checkColumnWithQuery(IloOplTableColIndex col,
                                IloOplTableColIndex columns,
                                IloOplTableError error,
                                const char* query)
{
  if ( col < 0 || col >= columns ) {
    if (query)
      error->setBoth(error, -1, "When processing query '%s', index %lld out of range [0,%lld]",
                     query, col, columns);
    else
      error->setBoth(error, -1, "index %lld out of range [0,%lld]",
                   col, columns);
    return 0;
  }
  return 1;
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
  return checkColumnWithQuery(col, columns, error, NULL);
}



/* ********************************************************************** *
 *                                                                        *
 *    Data input                                                          *
 *                                                                        *
 * ********************************************************************** */

typedef struct _ODBCInputRows {
  IloOplTableInputRowsC base;
  SQLHSTMT              stmt;         /* ODBC handle (SQL_HANDLE_STMT). */
  Column                *result;      /* Buffer to store query results (no batch) */
  SQLSMALLINT           cols;         /* Number of columns in statement. */
  char const            **fieldNames; /* Names for named fields. */
  const char* query; /* The query for this row (error handling purposes). */
  /* Fields used for batch reading*/
  ResultArray*          resultArray;      /* Array of result arrays */
  int32_t               currentRowIndex;  /* Index of the row being processed */
  SQLULEN               fetchedRowCount;  /* Number of rows that have been fetched */
  SQLUSMALLINT*         status;           /* row status after Execute - allocation size is conn.dbReadBatchSize */
  size_t                dbReadBatchSize;  /* if != 0, the batch size */
} ODBCInputRows;

static IloOplTableError inGetColumnCount(IloOplTableInputRows rows,
                                         IloOplTableError error,
                                         IloOplTableColIndex *cols_p)
{
  ODBCInputRows *r = (ODBCInputRows *)rows;

  (void)error;
  *cols_p = r->cols;

  return NULL;
}

static IloOplTableError inGetSelectedTupleFields(IloOplTableInputRows rows,
                                                 IloOplTableError error,
                                                 char *sep,
                                                 char const *const **fields_p)
{
  ODBCInputRows *r = (ODBCInputRows *)rows;

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
  ODBCInputRows *r = (ODBCInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;
  if (r->dbReadBatchSize == 0) {
      return col2int(&r->result[column], error, value_p);
  }
  else {
      /* batch reading */
      int32_t rowIndex = r->currentRowIndex;
      if (r->resultArray[column].l[rowIndex] == SQL_NULL_DATA) {
          *value_p = 0;
      }
      else {
          SQLLEN actual_len;
          switch (r->resultArray[column].type) {
          case T_INT8:
              actual_len = 1;
              *value_p = asInt8(r->resultArray + column, rowIndex);
              break;
          case T_INT16:
              actual_len = 2;
              *value_p = asInt16(r->resultArray + column, rowIndex);
              break;
          case T_INT32:
              actual_len = 4;
              *value_p = asInt32(r->resultArray + column, rowIndex);
              break;
          case T_INT64:
              actual_len = 8;
              *value_p = asInt64(r->resultArray + column, rowIndex);
              break;
          case T_FLOAT:
              actual_len = 4;
              *value_p = asFloat(r->resultArray + column, rowIndex);
              break;
          case T_DOUBLE:
              actual_len = 8;
              *value_p = asDouble(r->resultArray + column, rowIndex);
              break;
          default:
              fprintf(stderr, "type = %d\n", r->resultArray[column].type);
              error->setBoth(error, -1, "column is not integer");
              return error;
          }
          if (r->resultArray[column].l[rowIndex] < actual_len) {
              error->setBoth(error, -1, "not enough data for int column %lld vs %lld",
                  (long long)r->resultArray[column].l[rowIndex], (long long)actual_len);
              return error;
          }
      }
  }
  return NULL;
}

static IloOplTableError inReadString(IloOplTableInputRows rows,
                                     IloOplTableError error,
                                     IloOplTableColIndex column,
                                     char const **value_p)
{
  ODBCInputRows *r = (ODBCInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;
  if (r->dbReadBatchSize == 0) {
      return col2str(&r->result[column], error, value_p);
  }
  else {
      int32_t rowIndex = r->currentRowIndex;

      if (r->resultArray[column].l[rowIndex] == SQL_NULL_DATA) {
          *value_p = 0;
      }
      else {
          char* textPtr = NULL;
          switch (r->resultArray[column].type) {
          case T_TEXT:

              textPtr = asTextPtr(r->resultArray + column, rowIndex);
              /* Make sure the buffer is NUL terminated */
              textPtr[r->resultArray[column].l[rowIndex]] = '\0';
              *value_p = textPtr;
              break;
          default:
              fprintf(stderr, "Unknown type\n");
              error->setBoth(error, -1, "column is not text");
              return error;
          }
      }
  }
  return NULL;
}

static IloOplTableError inReadNum(IloOplTableInputRows rows,
                                  IloOplTableError error,
                                  IloOplTableColIndex column,
                                  double *value_p)
{
  ODBCInputRows *r = (ODBCInputRows *)rows;

  if ( !checkColumn(column, r->cols, error) )
    return error;
  if (r->dbReadBatchSize == 0) {
      return col2num(&r->result[column], error, value_p);
  }
  else {
      int32_t rowIndex = r->currentRowIndex;
      if (r->resultArray[column].l[rowIndex] == SQL_NULL_DATA) {
          *value_p = 0;
      }
      else {
          SQLLEN actual_len;
          switch (r->resultArray[column].type) {
          case T_INT8:
              actual_len = 1;
              *value_p = asInt8(r->resultArray + column, rowIndex);
              break;
          case T_INT16:
              actual_len = 2;
              *value_p = asInt16(r->resultArray + column, rowIndex);
              break;
          case T_INT32:
              actual_len = 4;
              *value_p = asInt32(r->resultArray + column, rowIndex);
              break;
          case T_INT64:
              actual_len = 8;
              *value_p = asInt64(r->resultArray + column, rowIndex);
              break;
          case T_FLOAT:
              actual_len = 4;
              *value_p = asFloat(r->resultArray + column, rowIndex);
              break;
          case T_DOUBLE:
              actual_len = 8;
              *value_p = asDouble(r->resultArray + column, rowIndex);
              break;
          default:
              fprintf(stderr, "type = %d\n", r->resultArray[column].type);
              error->setBoth(error, -1, "column is not floating point");
              return error;
          }
          if (r->resultArray[column].l[rowIndex] < actual_len) {
              error->setBoth(error, -1, "not enough data for int column %lld vs %lld",
                  (long long)r->resultArray[column].l[rowIndex], (long long)actual_len);
              return error;
          }
      }
  }

  return NULL;
}

static IloOplTableError inNext(IloOplTableInputRows rows,
                               IloOplTableError error, int *next_p)
{
  ODBCInputRows *r = (ODBCInputRows *)rows;
  if (r->dbReadBatchSize == 0) {
      long status = SQLFetch(r->stmt);

      if (status == SQL_NO_DATA)
          *next_p = 0;
      else {
          if (!checkSTMT(r->stmt, status, NULL, error))
              return error;
          *next_p = 1;
      }
  } /* non batch reading */
  else {
      r->currentRowIndex++;
      if (r->currentRowIndex >= r->fetchedRowCount) {
          /* We need to fetch more data */
          SQLRETURN ret;
          ret = SQLFetchScroll(r->stmt, SQL_FETCH_NEXT, 0);  // Returns SQL_NO_DATA if no data
          if (SQL_SUCCEEDED(ret)) {
              r->currentRowIndex = 0;
              *next_p = 1;
          }
          else if (ret == SQL_NO_DATA) {
              *next_p = 0;
          }
          else {
              if (!checkSTMT(r->stmt, ret, r->query, error))
                  return error;
              return error;
          }
      }
      else {
          /* raise error if the status for the row is not SUCCESS */
          if (!checkSTMT(r->stmt, r->status[r->currentRowIndex], r->query, error))
              return error;
          *next_p = 1;
      }
  } /* Batch reading */
  return NULL;
}

static void inDestroy(ODBCInputRows *rows)
{
  if ( rows ) {
    SQLSMALLINT col;

    if ( rows->stmt )
      SQLFreeHandle(SQL_HANDLE_STMT, rows->stmt);


    if ( rows->result ) {
      for (col = 0; col < rows->cols; ++col)
        if ( rows->result[col].type == T_TEXT )
          free(rows->result[col].data.t);
      free(rows->result);
    }

    if (rows->resultArray) {
        for (col = 0; col < rows->cols; col++) {
            deleteResultArrayData(rows->resultArray + col);
        }
        free(rows->resultArray);
    }

    if (rows->status) {
        free(rows->status);
    }

    if ( rows->fieldNames ) {
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

typedef struct _ODBCOutputRows {
  IloOplTableOutputRowsC base;
  SQLHSTMT               stmt;    /**< ODBC handle (SQL_HANDLE_STMT). */
  Column                 *params; /**< Data buffers for parameters. */
  SQLSMALLINT            cols;    /**< Number of parameters in statement. */
  Transaction            trans;   /**< Transaction for the update. */
  const char             *query;  /**< The query for this row (error handling purposes). */
  IntConversion          outputIntAs; /**< How to convert types */
  /* Fields used for batch updating */
  ResultArray*          paramArray;         /* Array of paramsets */
  int32_t               currentOutRowIndex;    /* Index of the row being processed */
  SQLULEN               processedRowCount;    /* Number of rows that have been processed */
  SQLUSMALLINT*         status;             /* row status after Execute - allocation size is conn.dbReadBatchSize */
  size_t                dbUpdateBatchSize;  /* if != 0, the batch size */
} ODBCOutputRows;


static IloOplTableError outGetSelectedTupleFields(IloOplTableOutputRows rows,
                                                  IloOplTableError error,
                                                  char *sep,
                                                  IloOplTableColIndex *cols,
                                                  char const *const **fields_p)
{
  /* ODBC does not support named parameters. */
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
  ODBCOutputRows *r = (ODBCOutputRows *)rows;

  if (!checkColumnWithQuery(column, r->cols, error, r->query))
      return error;

  /* NOTE: column is 0 indexed, but param index in SQL is 1 indexed */
  if (r->dbUpdateBatchSize > 0) {
      ResultArray* ra = r->paramArray + column;
      if (ra->type == T_NONE) {
          Type allocType = T_INT64;
          if (r->outputIntAs == CONVERT_INT) {
              allocType = T_INT32;
          }
          initResultArray(ra, allocType, r->dbUpdateBatchSize, error);

          /* Default is to use 64 bits int*/
          SQLSMALLINT cType = SQL_C_SBIGINT;
          SQLSMALLINT sqlType = SQL_BIGINT;
          if (r->outputIntAs == CONVERT_INT) { /* in case we convert ints to 32 bits*/
              cType = SQL_C_SLONG;
              sqlType = SQL_INTEGER;
          }
          long status = SQLBindParameter(r->stmt, column + 1,
              SQL_PARAM_INPUT, /* InputOutput type */
              cType,   /* Value type */
              sqlType,      /* Parameter type */
              0 /* ignored */, 0 /* ignored */,
              ra->data,
              ra->data_size,
              ra->l);

          if (!isOk(status)) {
              char* message = formatErrorMessage(SQL_HANDLE_STMT, r->stmt, status);
              error->setBoth(error, (int)status, "During the handling of %s, %s at column %d", r->query, message, column + 1);
              return error;
          }
      } /* Init SQL Parameter and buffer */
      /* set the value */
      if (r->outputIntAs == CONVERT_INT) {
          setInt32(ra, r->currentOutRowIndex, value);
      }
      else { 
          setInt64(ra, r->currentOutRowIndex, value);
      }
  }
  else {
      if (r->params[column].type == T_NONE) {
          long status;
          switch (r->outputIntAs) {
          case CONVERT_INT:
              status = SQLBindParameter(r->stmt, column + 1,
                  SQL_PARAM_INPUT, /* InputOutput type */
                  SQL_C_SLONG,   /* Value type */
                  SQL_INTEGER,      /* Parameter type */
                  0 /* ignored */, 0 /* ignored */,
                  &r->params[column].data.i,
                  sizeof(r->params[column].data.i),
                  &r->params[column].l);
              r->params[column].type = T_INT32;
              break;
          default:
              status = SQLBindParameter(r->stmt, column + 1,
                  SQL_PARAM_INPUT, /* InputOutput type */
                  SQL_C_SBIGINT,   /* Value type */
                  SQL_BIGINT,      /* Parameter type */
                  0 /* ignored */, 0 /* ignored */,
                  &r->params[column].data.l,
                  sizeof(r->params[column].data.l),
                  &r->params[column].l);
              r->params[column].type = T_INT64;
              break;
          }
          if (!isOk(status)) {
              char* message = formatErrorMessage(SQL_HANDLE_STMT, r->stmt, status);
              error->setBoth(error, (int)status, "During the handling of %s, %s at column %d", r->query, message, column + 1);
              return error;
          }
      } /* Init parameter type if first time */

      /* set the value */
      if (r->outputIntAs == CONVERT_INT) {
          r->params[column].data.i = value;
      } else {
          r->params[column].data.l = value;
      }
  }
  return NULL;
}

static IloOplTableError outWriteString(IloOplTableOutputRows rows,
                                       IloOplTableError error,
                                       IloOplTableColIndex column,
                                       char const *value)
{
  ODBCOutputRows *r = (ODBCOutputRows *)rows;
  size_t len;

  if (!checkColumnWithQuery(column, r->cols, error, r->query))
      return error;

  /* NOTE: column is 0 indexed, but param index in SQL is 1 indexed */
  if (r->dbUpdateBatchSize > 0) {
      ResultArray* ra = r->paramArray + column;
      if (ra->type == T_NONE) {
          Type allocType = T_TEXT;
          initResultArray(ra, allocType, r->dbUpdateBatchSize, error);
          size_t alloc_size = MAX_STRING_LENGTH;
          if ((ra->data = calloc(ra->array_size * alloc_size, sizeof(char))) == NULL) {
              setOOM(error);
              return error;
          }
          ra->data_size = alloc_size;
          if (!checkSTMT(r->stmt, SQLBindParameter(r->stmt, column + 1,
                  SQL_PARAM_INPUT,            /* InputOutputType */
                  SQL_C_CHAR,                 /* ValueType */
                  SQL_VARCHAR,                /* ParameterType */
                  ra->data_size,      /* ColumnSize */
                  0,                          /* DecimalDigits */
                  ra->data,   /* ParameterValuePtr */
                  ra->data_size,          /* BufferLength */
                  ra->l),
              NULL,
              error))
          {
              return error;
          }
      } /* Init and bind parameter */

      if (!value) /* NULL value is empty string */
          value = "";

      len = strlen(value);
      if (len >= ra->data_size) {
          error->setBoth(error, -1, "output string too long");
          return error;
      }
      char* txt = asTextPtr(ra, r->currentOutRowIndex);
      memcpy(txt, value, len);
      txt[len] = 0;
      ra->l[r->currentOutRowIndex] = SQL_NTS;
  }
  else {
      if (r->params[column].type == T_NONE) {
          r->params[column].type = T_TEXT;
          if ((r->params[column].data.t = malloc(MAX_STRING_LENGTH)) == NULL)
              return setOOM(error);
          if (!checkSTMT(r->stmt, SQLBindParameter(r->stmt, column + 1,
              SQL_PARAM_INPUT,            /* InputOutputType */
              SQL_C_CHAR,                 /* ValueType */
              SQL_VARCHAR,                /* ParameterType */
              MAX_STRING_LENGTH - 1,      /* ColumnSize */
              0,                          /* DecimalDigits */
              r->params[column].data.t,   /* ParameterValuePtr */
              MAX_STRING_LENGTH - 1,      /* BufferLength */
              &(r->params[column].l)),
              NULL, error))
              return error;
          r->params[column].data.t[MAX_STRING_LENGTH - 1] = '\0';
          r->params[column].type = T_TEXT;
      }

      if (!value) /* NULL value is empty string */
          value = "";

      len = strlen(value);
      if (len >= MAX_STRING_LENGTH) {
          error->setBoth(error, -1, "output string too long");
          return error;
      }
      memcpy(r->params[column].data.t, value, len);
      r->params[column].data.t[len] = 0;
      r->params[column].l = len;
  }
  return NULL;
}

static IloOplTableError outWriteNum(IloOplTableOutputRows rows,
                                    IloOplTableError error,
                                    IloOplTableColIndex column,
                                    double value)
{
  ODBCOutputRows *r = (ODBCOutputRows *)rows;

  if (!checkColumnWithQuery(column, r->cols, error, r->query))
      return error;

  /* NOTE: column is 0 indexed, but param index in SQL is 1 indexed */
  if (r->dbUpdateBatchSize > 0) {
      ResultArray* ra = r->paramArray + column;
      if (ra->type == T_NONE) {
          Type allocType = T_DOUBLE;
          initResultArray(ra, allocType, r->dbUpdateBatchSize, error);
          if (!checkSTMT(r->stmt, SQLBindParameter(r->stmt, column + 1,
              SQL_PARAM_INPUT,
              SQL_C_DOUBLE,
              SQL_REAL,
              0 /* ignored */, 0 /* ignored */,
              ra->data,
              ra->data_size,
              ra->l),
              r->query, error))
              return error;
      }
      setDouble(ra, r->currentOutRowIndex, value);
  }
  else {
      if (r->params[column].type == T_NONE) {
          if (!checkSTMT(r->stmt, SQLBindParameter(r->stmt, column + 1,
              SQL_PARAM_INPUT,
              SQL_C_DOUBLE,
              SQL_REAL,
              0 /* ignored */, 0 /* ignored */,
              &r->params[column].data.d,
              sizeof(r->params[column].data.d),
              &r->params[column].l),
              r->query, error))
              return error;
          r->params[column].type = T_DOUBLE;
      }
      r->params[column].data.d = value;
  }
  return NULL;
}

static IloOplTableError executeBatch(ODBCOutputRows* r,
    IloOplTableError error) {
    logInfo("Executing batch of %lld\n", (long long)r->currentOutRowIndex);
    /* If there are no rows in the buffer, just return */
    if (r->currentOutRowIndex <= 0)
        return NULL;
    SQLSetStmtAttr(r->stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)((SQLULEN)r->currentOutRowIndex), 0);

    long status = SQLExecute(r->stmt);
    if (!isOk(status)) {
        char* message = formatErrorMessage(SQL_HANDLE_STMT, r->stmt, status);
        error->setBoth(error, (int)status, "During the handling of %s, %s when ending row", r->query, message);
        return error;
    }

    size_t i;
    logInfo("   processed %lld rows\n", (long long)r->processedRowCount);
    for (i = 0; i < r->processedRowCount; i++) {
        if (!isOkParamStatus(r->status[i])) {
            char* message = formatErrorMessage(SQL_HANDLE_STMT, r->stmt, status);
            error->setBoth(error, (int)status, "During the handling of %s, %s when ending row for row %ld",
                r->query, message, i);
            return error;
        }
    }

    r->currentOutRowIndex = 0;

    return NULL;
}

static IloOplTableError outEndRow(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  ODBCOutputRows *r = (ODBCOutputRows *)rows;

  if (r->dbUpdateBatchSize > 0) {
      r->currentOutRowIndex++;
      if (r->currentOutRowIndex >= r->dbUpdateBatchSize) {
          return executeBatch(r, error);
      }
  }
  else {
      long status = SQLExecute(r->stmt);

      if (!isOk(status)) {
          char* message = formatErrorMessage(SQL_HANDLE_STMT, r->stmt, status);
          error->setBoth(error, (int)status, "During the handling of %s, %s when ending row", r->query, message);
          return error;
      }
  }
  return NULL;
}

static IloOplTableError outCommit(IloOplTableOutputRows rows,
                                  IloOplTableError error)
{
  ODBCOutputRows *r = (ODBCOutputRows *)rows;
 
  if (r->dbUpdateBatchSize > 0 && r->currentOutRowIndex > 0) {
      IloOplTableError status = executeBatch(r, error);
      if (status != NULL) {
          return status;
      }
  }
  return transactionCommit(&r->trans, error);
}

static void outDestroy(ODBCOutputRows *rows)
{
  if ( rows ) {
    SQLSMALLINT col;

    if ( rows->stmt )
      SQLFreeHandle(SQL_HANDLE_STMT, rows->stmt);

    if ( rows->params ) {
      for (col = 0; col < rows->cols; ++col)
        if ( rows->params[col].type == T_TEXT )
          free(rows->params[col].data.t);
      free(rows->params);
    }
    transactionComplete(&rows->trans);

    /* free resources for batch updating */
    if (rows->paramArray) {
        for (col = 0; col < rows->cols; col++) {
            deleteResultArrayData(rows->paramArray + col);
        }
        free(rows->paramArray);
        rows->paramArray = NULL;
    }
    if (rows->status) {
        free(rows->status);
        rows->status = NULL;
    }

    free(rows);
  }
}


/* ********************************************************************** *
 *                                                                        *
 *    Connection handling                                                 *
 *                                                                        *
 * ********************************************************************** */

typedef struct _ODBCConnection {
  IloOplTableConnectionC base;
  SQLHENV  env;   /**< ODBC environment handle (SQL_HANDLE_ENV). */
  SQLHDBC  dbc;   /**< Database connection (SQL_HANDLE_DBC). */
  int      named; /**< Are tuples assumed to be queried by fields? */
  IntConversion outputIntAs; /**< How to convert int types */

  size_t   dbReadBatchSize;   /**< Batch size for read operations */
  size_t   dbUpdateBatchSize; /**< Batch size for update oprations */
} ODBCConnection;



static void connDestroy(IloOplTableConnection conn)
{
  ODBCConnection *c = (ODBCConnection *)conn;
  if ( c ) {
    if ( c->dbc ) {
      SQLDisconnect(c->dbc);
      SQLFreeHandle(SQL_HANDLE_DBC, c->dbc);
    }
    if ( c->env )
      SQLFreeHandle(SQL_HANDLE_ENV, c->env);
    free(c);
  }
}

/* Returns NULL if no error happened */
static IloOplTableError prepareAndBindColsUnbatched(IloOplTableConnection conn,
                                          IloOplTableError error,
                                          ODBCInputRows* rows,
                                          char const* query,
                                          SQLUSMALLINT col,
                                          SQLSMALLINT datatype)
{
    size_t col_index = col - 1;
    Column* r = &rows->result[col_index];
    switch (datatype) {
    case SQL_TINYINT:
        r->type = T_INT8;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_TINYINT,
            &r->data.b, sizeof(r->data.b),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_SMALLINT:
        r->type = T_INT16;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SHORT,
            &r->data.s, sizeof(r->data.s),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_INTEGER:
        r->type = T_INT32;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SLONG,
            &r->data.i, sizeof(r->data.i),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_BIGINT:
        r->type = T_INT64;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SBIGINT,
            &r->data.l, sizeof(r->data.l),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
        r->type = T_DOUBLE;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_DOUBLE,
            &r->data.d, sizeof(r->data.d),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_REAL:
        r->type = T_FLOAT;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_FLOAT,
            &r->data.f, sizeof(r->data.f),
            &r->l), query, error))
            goto TERMINATE;
        break;
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        r->type = T_TEXT;
        if ((r->data.t = malloc(MAX_STRING_LENGTH)) == NULL) {
            setOOM(error);
            goto TERMINATE;
        }
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_CHAR,
            r->data.t, MAX_STRING_LENGTH - 1,
            &r->l), query, error))
            goto TERMINATE;
        r->data.t[MAX_STRING_LENGTH - 1] = '\0';
        break;
    case SQL_INVALID_HANDLE: // -2
        error->setBoth(error, -1, "Column %d is invalid or does not exist",
            (int)col);
        goto TERMINATE;
    default:
        error->setBoth(error, -1, "Cannot handle column type %d on column %d",
            (int)datatype, (int)col);
        goto TERMINATE;
    }
    return NULL;
TERMINATE:
    return error;
}


/* Returns NULL if no error happened */
static IloOplTableError prepareAndBindColsBatched(IloOplTableConnection conn,
    IloOplTableError error,
    ODBCInputRows* rows,
    char const* query,
    SQLUSMALLINT col,
    SQLSMALLINT datatype,
    size_t stringLen)
{
    size_t col_index = col - 1;
    ResultArray* ra = rows->resultArray + col_index;
    initResultArray(ra, getDataTypeFromSQLDataType(datatype), rows->dbReadBatchSize, error);
    switch (datatype) {
    case SQL_TINYINT:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_TINYINT,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_SMALLINT:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SHORT,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_INTEGER:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SLONG,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_BIGINT:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_SBIGINT,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_DOUBLE,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_REAL:
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_FLOAT,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
        ; /* allocate a buffer of stringLen+1 for '\0' */
        size_t alloc_size = stringLen + 1;
        if ((ra->data = calloc(ra->array_size * alloc_size, sizeof(char))) == NULL) {
            setOOM(error);
            goto TERMINATE;
        }
        ra->data_size = alloc_size;
        if (!checkSTMT(rows->stmt, SQLBindCol(rows->stmt, col, SQL_C_CHAR,
            ra->data, ra->data_size,
            ra->l), query, error))
            goto TERMINATE;
        break;
    case SQL_INVALID_HANDLE: // -2
        error->setBoth(error, -1, "Column %d is invalid or does not exist",
            (int)col);
        goto TERMINATE;
    default:
        error->setBoth(error, -1, "Cannot handle column type %d on column %d",
            (int)datatype, (int)col);
        goto TERMINATE;
    }
    return NULL;
TERMINATE:
    return error;
}


static IloOplTableError connOpenInputRows(IloOplTableConnection conn,
                                          IloOplTableError error,
                                          IloOplTableContext context,
                                          char const *query,
                                          IloOplTableInputRows *rows_p)
{
  ODBCConnection *c = (ODBCConnection *)conn;
  SQLHDBC dbc = c->dbc;
  IloOplTableError err = error;
  ODBCInputRows *rows = NULL;
  SQLSMALLINT col, cols;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->base.getColumnCount = inGetColumnCount;
  rows->base.getSelectedTupleFields = inGetSelectedTupleFields;
  rows->base.readInt = inReadInt;
  rows->base.readString = inReadString;
  rows->base.readNum = inReadNum;
  rows->base.next = inNext;
  rows->query = query;

  /* Prepare a statement with the SQL to be executed. Then query
   * the statements meta data to prepare fetching results.
   */
  if ( !checkDBC(dbc, SQLAllocHandle(SQL_HANDLE_STMT, dbc, &rows->stmt),
                 query, error) )
    goto TERMINATE;

  if (c->dbReadBatchSize != 0) {
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_ROWS_FETCHED_PTR, &(rows->fetchedRowCount), 0);
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_ROW_STATUS_PTR, rows->status, 0);
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)c->dbReadBatchSize, 0);
  }
  rows->dbReadBatchSize = c->dbReadBatchSize;

  if ( !checkSTMT(rows->stmt, SQLPrepare(rows->stmt, sqlchar(query), SQL_NTS),
                  query, error) )
    goto TERMINATE;

  if ( !checkSTMT(rows->stmt, SQLNumResultCols(rows->stmt, &cols),
                  query, error) )
    goto TERMINATE;
  rows->cols = cols;
  
  if ( (rows->result = calloc(cols, sizeof(*rows->result))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  /* Initialize result buffer */
  rows->currentRowIndex = -1;    /* Needs to start at -1 because it is incremented in inNext()*/
  rows->fetchedRowCount = 0;
  if (c->dbReadBatchSize != 0) {
      if ((rows->resultArray = calloc(cols, sizeof(*rows->resultArray))) == NULL) {
          setOOM(error);
          goto TERMINATE;
      }
      if ((rows->status = calloc(c->dbReadBatchSize, sizeof(*rows->status))) == NULL) {
          setOOM(error);
          goto TERMINATE;
      }
  }
  else {
      rows->resultArray = NULL;
      rows->status = NULL;
  }

  if ( c->named ) {
    if ( (rows->fieldNames = calloc(cols, sizeof(*rows->fieldNames))) == NULL ) {
      setOOM(error);
      goto TERMINATE;
    }
  }

  /* Find the types of columns and bind fields to them. */
  for (col = 1; col <= cols; ++col) {
    size_t col_index = ((size_t)col) - 1;
    SQLSMALLINT datatype;
    char name[MAX_NAME_LEN];
    SQLSMALLINT namelen;
    SQLULEN columnSize;

    if ( !checkSTMT(rows->stmt, SQLDescribeCol(rows->stmt, col, sqlchar(name),
                                               sizeof(name), &namelen,
                                               &datatype, &columnSize, 0, 0), query,
                    error) )
      goto TERMINATE;
    if ( (size_t)namelen >= sizeof(name) ) {
      error->setBoth(error, -1, "name too long (max name is %lu)",
                       sizeof(name));
      goto TERMINATE;
    }

  
    if ( c->named ) {
      if ( (rows->fieldNames[col_index] = STRDUP(name)) == NULL ) {
        setOOM(error);
        goto TERMINATE;
      }
    }

    IloOplTableError err = NULL;
    if (c->dbReadBatchSize == 0) {
        err = prepareAndBindColsUnbatched(conn,
            error,
            rows,
            query,
            col,
            datatype);
    }
    else {
        /* We want to limit the column size to the max string length */
        if (columnSize >= MAX_STRING_LENGTH) {
            columnSize = MAX_STRING_LENGTH - 1;
        }
        err = prepareAndBindColsBatched(conn,
            error,
            rows,
            query,
            col,
            datatype,
            columnSize);
    }
    if (err != NULL)
        goto TERMINATE;
  } /* Loop on cols */

  /* Finally execute the statement. */
  if ( !checkSTMT(rows->stmt, SQLExecute(rows->stmt), query, error) )
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
  inDestroy((ODBCInputRows *)rows);
}

static IloOplTableError connOpenOutputRows(IloOplTableConnection conn,
                                           IloOplTableError error,
                                           IloOplTableContext context,
                                           char const *query,
                                           IloOplTableOutputRows *rows_p)
{
  ODBCConnection *c = (ODBCConnection *)conn;
  SQLHDBC dbc = c->dbc;
  IloOplTableError err = error;
  ODBCOutputRows *rows = NULL;
  SQLSMALLINT nParams;

  (void)context; /* unused in this example */

  if ( (rows = calloc(1, sizeof(*rows))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }

  rows->base.getSelectedTupleFields = outGetSelectedTupleFields;
  rows->base.writeInt = outWriteInt;
  rows->base.writeString = outWriteString;
  rows->base.writeNum = outWriteNum;
  rows->base.endRow = outEndRow;
  rows->base.commit = outCommit;
  rows->trans.dbc = dbc;
  rows->query = query;
  /**
   * Some drivers like Oracle ODBC < 21 do not know how to handle SQL_C_SBIGINT.
   * When this is set to 1, *all* opl integer values (which are normally 64 bits)
   * will be cast to 32 bits.
   */
  rows->outputIntAs = c->outputIntAs;

  /* Wrap the execution of this statement into a transaction */
  if ( transactionStart(&rows->trans, error) )
    goto TERMINATE;

  if ( !checkDBC(dbc, SQLAllocHandle(SQL_HANDLE_STMT, dbc, &rows->stmt),
                 query, error) )
    goto TERMINATE;

  if ( !checkSTMT(rows->stmt, SQLPrepare(rows->stmt, sqlchar(query), SQL_NTS),
                  query, error) )
    goto TERMINATE;

  /* Query the number of parameters and size the parameter buffers
   * accordingly. Note that the individual elements in the array are
   * lazily initialized in the various writeXXX() functions. Binding
   * of the buffers to the statement happens in endRow().
   */
  if ( !checkSTMT(rows->stmt, SQLNumParams(rows->stmt, &nParams), query,
                  error) )
    goto TERMINATE;
  assert(nParams > 0);
  rows->cols = nParams;

  if ( (rows->params = calloc(rows->cols, sizeof(*rows->params))) == NULL ) {
    setOOM(error);
    goto TERMINATE;
  }


  if (c->dbUpdateBatchSize != 0) {
      rows->dbUpdateBatchSize = c->dbUpdateBatchSize;

      /* Initialize result buffer */
      if ((rows->paramArray = calloc(rows->cols, sizeof(*rows->paramArray))) == NULL) {
          setOOM(error);
          goto TERMINATE;
      }
      if ((rows->status = calloc(c->dbUpdateBatchSize, sizeof(*rows->status))) == NULL) {
          setOOM(error);
          goto TERMINATE;
      }
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_PARAM_STATUS_PTR, rows->status, 0);
      SQLSetStmtAttr(rows->stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &rows->processedRowCount, 0); 
  } /* if (c->dbUpdateBatchSize != 0) */
  else {
      rows->paramArray = NULL;
      rows->status = NULL;
  }
  rows->currentOutRowIndex = 0;

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
  outDestroy((ODBCOutputRows *)rows);
}

static IloOplTableError connCreate(char const *connstr,
                                   char const *sql,
                                   int load,
                                   IloOplTableContext context,
                                   IloOplTableError error,
                                   IloOplTableConnection *conn_p)
{
  IloOplTableError err = error;
  ODBCConnection *conn = NULL;
  IloOplTableArgs args = NULL;
  char *copy = NULL;

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
  if ( args->getBool(args, K_NAMED, &conn->named, &conn->named) != 0 ) {
    error->setBoth(error, -1, "failed to get 'named' from '%s'", connstr);
    goto TERMINATE;
  }
# define K_OPL_INT_WIDTH "OPL_INT_WIDTH"
  long default_int_width=0;
  if ( args->getInt(args, K_OPL_INT_WIDTH, &default_int_width, &default_int_width) != 0 ) {
    error->setBoth(error, -1, "failed to get 'OPL_INT_WIDTH' from '%s'", connstr);
    goto TERMINATE;
  }
  if (default_int_width == 0) {
    conn->outputIntAs = CONVERT_NONE;  // Default do not convert
  } else if (default_int_width == 32) {
    conn->outputIntAs = CONVERT_INT;
  } else {
    error->setBoth(error, -1, "Illegal value for OPL_INT_WIDTH: '%ld'", default_int_width);
    goto TERMINATE;
  }
#define DEFAULT_DB_READ_BATCH_SIZE 0
#define K_DB_READ_BATCH_SIZE       "dbReadBatchSize"
  long default_db_read_batch_size = DEFAULT_DB_READ_BATCH_SIZE;
  if (args->getInt(args, K_DB_READ_BATCH_SIZE, &default_db_read_batch_size, &default_db_read_batch_size) != 0) {
      error->setBoth(error, -1, "failed to get 'dbReadBatchSize' from '%s'", connstr);
      goto TERMINATE;
  }
  conn->dbReadBatchSize = default_db_read_batch_size;
#define DEFAULT_DB_UPDATE_BATCH_SIZE 5000
#define K_DB_UPDATE_BATCH_SIZE       "dbUpdateBatchSize"
  long default_db_update_batch_size = DEFAULT_DB_UPDATE_BATCH_SIZE;
  if (args->getInt(args, K_DB_UPDATE_BATCH_SIZE, &default_db_update_batch_size, &default_db_update_batch_size) != 0) {
      error->setBoth(error, -1, "failed to get 'dbUpdateBatchSize' from '%s'", connstr);
      goto TERMINATE;
  }
  conn->dbUpdateBatchSize = default_db_update_batch_size;

#define K_VERBOSE                     "dbVerbose"
  if (args->getBool(args, K_VERBOSE, &verbose, &verbose) != 0) {
      error->setBoth(error, -1, "failed to get 'dbVerbose' from '%s'", connstr);
      goto TERMINATE;
  }

  connstr = args->original(args, K_NAMED, K_OPL_INT_WIDTH, NULL);


  /* Allocate ODBC handle for the right version. */
  if ( !check(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &conn->env),
              error) )
    goto TERMINATE;
  if ( !checkENV(conn->env, SQLSetEnvAttr(conn->env, SQL_ATTR_ODBC_VERSION,
                                          (SQLPOINTER)SQL_OV_ODBC3, 0),
                 NULL, error) )
    goto TERMINATE;

  /* Connect to dabase using connection string. */
  if ( !check(SQLAllocHandle(SQL_HANDLE_DBC, conn->env, &conn->dbc), error) )
    goto TERMINATE;
  if ( !checkDBC(conn->dbc, SQLDriverConnect(conn->dbc, 0,
                                             sqlchar(connstr), SQL_NTS,
                                             0, 0, 0,
                                             SQL_DRIVER_NOPROMPT),
                 connstr, error) )
    goto TERMINATE;

  /* If we open the connection for publishing and there is an initial
   * SQL statement to be executed then execute that now.
   */
  if ( !load && sql && *sql ) {
    /* Split sql command at semi-colons.
     * Note that we strip initial blanks since those may confuse some
     * drivers.
     */
    char *start, *semi;

    if ( (copy = STRDUP(sql)) == NULL ) {
      setOOM(error);
      goto TERMINATE;
    }

    start = copy;
    while ((semi = strchr(start, ';')) != NULL) {
      *semi = '\0';
      if ( !exec(conn->dbc, start, error) )
        goto TERMINATE;
      start = semi + 1;
    }

    if ( *start ) {
      if ( !exec(conn->dbc, start, error) )
        goto TERMINATE;
    }
  }

  *conn_p = &conn->base;
  conn = NULL;
  err = NULL;

 TERMINATE:
  free(copy);

  if (conn) {
      connDestroy(&conn->base);
  }
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
 * shared library when it encounters a "ODBCConnection" statement
 * in a .dat file.
 */
ILO_TABLE_EXPORT ILOOPLTABLEDATAHANDLER(ODBC, error, factory_p)
{
  (void)error;
  *factory_p = (IloOplTableFactory)&factory; /* cast away 'const' */
  factoryIncRef(*factory_p);
  return NULL;
}
