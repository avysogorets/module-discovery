// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/data/iloopltabledatasource.h
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corp. 2019, 2024
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// ---------------------------------------------------------------------------

#ifndef ILOOPLTABLEDATASOURCE_H
#define ILOOPLTABLEDATASOURCE_H 1

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <stdint.h>

#if defined(_WIN32)
#  define ILO_TABLE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) && defined(__linux__)
#  define ILO_TABLE_EXPORT  __attribute__ ((visibility("default")))
#elif defined(__APPLE__)
#  define ILO_TABLE_EXPORT  __attribute__ ((visibility("default")))
#elif defined(__SUNPRO_C)
#  define ILO_TABLE_EXPORT  __global
#else
#  define ILO_TABLE_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
  typedef int32_t IloOplTableColIndex;

  
  typedef int64_t IloOplTableIntType;

  
  typedef struct IloOplTableContextS const IloOplTableContextC, *IloOplTableContext;
  typedef struct IloOplTableErrorS IloOplTableErrorC, *IloOplTableError;
  typedef struct IloOplTableInputRowsS IloOplTableInputRowsC, *IloOplTableInputRows;
  typedef struct IloOplTableOutputRowsS IloOplTableOutputRowsC, *IloOplTableOutputRows;
  typedef struct IloOplTableConnectionS IloOplTableConnectionC, *IloOplTableConnection;
  typedef struct IloOplTableArgsS const IloOplTableArgsC, *IloOplTableArgs;
  typedef struct IloOplTableFactoryS IloOplTableFactoryC, *IloOplTableFactory;

  
  struct IloOplTableErrorS {
    
    void (*setCode)(IloOplTableError err, int code);
    
    void (*setMessage)(IloOplTableError err, char const *format, ...);
    
    void (*setBoth)(IloOplTableError err, int code, char const *format, ...);
    
    int (*getCode)(IloOplTableError err);
    
    char const *(*getMessage)(IloOplTableError err);
  };

  
  struct IloOplTableInputRowsS {
    
    IloOplTableError
    (*getColumnCount)(IloOplTableInputRows rows,
                      IloOplTableError error,
                      IloOplTableColIndex *cols_p);

    
    IloOplTableError 
    (*getSelectedTupleFields)(IloOplTableInputRows rows,
                              IloOplTableError error, char *sep,
                              char const *const **fields_p);

    
    IloOplTableError 
    (*readInt)(IloOplTableInputRows rows,
               IloOplTableError error,
               IloOplTableColIndex column,
               IloOplTableIntType *value_p);
    
    IloOplTableError 
    (*readString)(IloOplTableInputRows rows,
                  IloOplTableError error,
                  IloOplTableColIndex column,
                  char const **value_p);
    
    IloOplTableError 
    (*readNum)(IloOplTableInputRows rows,
               IloOplTableError error,
               IloOplTableColIndex column,
               double *value_p);
    
    IloOplTableError 
    (*next)(IloOplTableInputRows rows, IloOplTableError error, int *next_p);
  };

  
  struct IloOplTableOutputRowsS {
    
    IloOplTableError 
    (*getSelectedTupleFields)(IloOplTableOutputRows rows,
                              IloOplTableError error,
                              char *sep, IloOplTableColIndex *cols,
                              char const *const **fields_p);

    
    IloOplTableError 
    (*writeInt)(IloOplTableOutputRows rows, IloOplTableError error,
                IloOplTableColIndex column, IloOplTableIntType value);
    
    IloOplTableError
    (*writeString)(IloOplTableOutputRows rows, IloOplTableError error,
                   IloOplTableColIndex column, char const *value);
    
    IloOplTableError
    (*writeNum)(IloOplTableOutputRows rows, IloOplTableError error,
                IloOplTableColIndex column, double value);
    
    IloOplTableError
    (*endRow)(IloOplTableOutputRows rows, IloOplTableError error);

    
    IloOplTableError
    (*commit)(IloOplTableOutputRows rows, IloOplTableError error);
  };

  
  struct IloOplTableConnectionS {
    
    void (*destroy)(IloOplTableConnection conn);
    
    IloOplTableError
    (*openInputRows)(IloOplTableConnection conn,
                     IloOplTableError error, IloOplTableContext context,
                     char const *query, IloOplTableInputRows *rows_p);

    
    void
    (*closeInputRows)(IloOplTableConnection conn, IloOplTableInputRows rows);

    
    IloOplTableError
    (*openOutputRows)(IloOplTableConnection conn,
                      IloOplTableError error, IloOplTableContext context,
                      char const *query, IloOplTableOutputRows *rows_p);

    
    void
    (*closeOutputRows)(IloOplTableConnection conn,
                       IloOplTableOutputRows rows);

  };

  
  struct IloOplTableArgsS {
    int (*getBool)(IloOplTableArgs, char const *key,
                   int const *defaultValue, int *value_p);
    int (*getInt)(IloOplTableArgs, char const *key,
                  long const *defaultValue, long *value_p);
    int (*getDouble)(IloOplTableArgs, char const *key,
                     double const *defaultValue, double *value_p);
    int (*getString)(IloOplTableArgs, char const *key,
                     char const *defaultValue, char const **value_p);
    
    int (*contains)(IloOplTableArgs, char const *key);
    
    char const *(*original)(IloOplTableArgs, char const *filter, ...);
  };

  
  struct IloOplTableContextS {
    
    char const *(*expandEnv)(IloOplTableContext ctx, char const *arg);

    
    char const *(*expandModel)(IloOplTableContext ctx, char const *arg);

    
    char const *(*expand)(IloOplTableContext ctx, char const *arg);

    
    char const *(*resolvePath)(IloOplTableContext ctx, char const *arg);

    
    IloOplTableArgs (*parseArgs)(IloOplTableContext ctx, char const *arg,
                                 char sep, char esc);
    
    int (*getJni)(IloOplTableContext, void **jni);
  };

  
  struct IloOplTableFactoryS {
    
    IloOplTableError (*connect)(char const *subId,
                                char const *spec,
                                int load,
                                IloOplTableContext context,
                                IloOplTableError error,
                                IloOplTableConnection *conn_p);
    
    void (*incRef)(IloOplTableFactory);
    
    void (*decRef)(IloOplTableFactory);
  };

  
  typedef IloOplTableError IloOplTableGetFactory(IloOplTableError error,
                                                 IloOplTableFactory *factory_p);

#ifdef __cplusplus
}
#endif

#define ILOOPLTABLEDATAHANDLER_PREFIX_STRING ""

#define ILOOPLTABLEDATAHANDLER_SUFFIX_STRING "construct"

#define ILOOPLTABLEDATAHANDLER_NAME(name) name ## construct

#define ILOOPLTABLEDATAHANDLER(name,e,f)                                \
  IloOplTableError ILOOPLTABLEDATAHANDLER_NAME(name)(IloOplTableError e, \
                                                     IloOplTableFactory *f)

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif 
