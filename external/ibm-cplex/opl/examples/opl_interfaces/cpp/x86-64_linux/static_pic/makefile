SYSTEM = x86-64_linux
LIBFORMAT = static_pic

CCC = /usr/bin/g++
CC = /usr/bin/gcc

# System/lib-format specific compile/link flags
CSYSFLAGS  = -fPIC
LDSYSFLAGS = 

# Compilation and linking flags enabling Multi-threading
CMTFLAGS =
LDMTFLAGS = -lpthread

# Code optimization/debugging options
DEBUG = -O1 -DNDEBUG

# JDK environment
JDK = $(JAVA_HOME)
JDKOPTIONS = 
JDKINCLUDE = linux


OPLDIR=../../../../..
CPLEXDIR=$(OPLDIR)
CPDIR=$(OPLDIR)

CPLEXVERSION=2212
CPLEXNAME=cplex$(CPLEXVERSION)
SO_EXT=.so

DEMODIR=$(OPLDIR)/examples/opl_interfaces/cpp
DEMOSRCDIR=$(DEMODIR)/src
SUFFIX=cpp

#---------------------------------------------------------
# Database plugins
#---------------------------------------------------------

ifeq ($(SQLite_HEADER),)
SQLite_HEADER := $(firstword \
	$(wildcard /usr/local/include/sqlite3.h) \
	$(wildcard /usr/include/sqlite3.h) \
	)
endif
ifneq ($(SQLite_HEADER),)
SQLite_BASE = $(patsubst %/include/sqlite3.h,%,$(SQLite_HEADER))
SQLite_INCLUDE = -I$(dir $(SQLite_HEADER))
SQLite_LIB = -L$(firstword \
	 $(wildcard $(subst /include/,/lib64/,$(dir $(SQLite_HEADER)))) \
	 $(wildcard $(subst /include/,/lib/,$(dir $(SQLite_HEADER)))) \
	) -lsqlite3
OPLSQLite_DLL := libopltabSQLite$(CPLEXVERSION)$(SO_EXT)
endif

ifeq ($(ODBC_HEADER),)
ODBC_HEADER := $(firstword \
	$(wildcard /usr/local/include/sql.h) \
	$(wildcard /usr/include/sql.h) \
	)
endif
ifneq ($(ODBC_HEADER),)
ODBC_INCLUDE = -I$(dir $(ODBC_HEADER))
ODBC_LIB = -L$(firstword \
	 $(wildcard $(subst /include/,/lib64/,$(dir $(ODBC_HEADER)))) \
	 $(wildcard $(subst /include/,/lib/,$(dir $(ODBC_HEADER)))) \
	) -lodbc
OPLODBC_DLL := libopltabODBC$(CPLEXVERSION)$(SO_EXT)
endif

ifeq ($(MySQL_HEADER),)
MySQL_HEADER := $(firstword \
	$(wildcard /usr/local/include/mysql/mysql.h) \
	$(wildcard /usr/include/mysql/mysql.h) \
	$(wildcard /usr/local/include/mysql.h) \
	$(wildcard /usr/include/mysql.h) \
	)
endif
ifneq ($(MySQL_HEADER),)
MySQL_INCLUDE = -I$(dir $(MySQL_HEADER))
MySQL_LIB = -L$(firstword \
	 $(wildcard $(subst /include/,/lib64/,$(dir $(MySQL_HEADER)))) \
	 $(wildcard $(subst /include/,/lib/,$(dir $(MySQL_HEADER)))) \
	) -lmysqlclient
OPLMySQL_DLL := libopltabMySQL$(CPLEXVERSION)$(SO_EXT)
endif

#---------------------------------------------------------
# Compiler and linker flags
#---------------------------------------------------------

# OPTIONS and LDOPTIONS provide a way for the user to add his own
# specific compilation and linking options.

CFLAGS = $(CSYSFLAGS) $(DEBUG) \
	-I$(OPLDIR)/include \
	-I$(CPLEXDIR)/include \
	-I$(CPDIR)/include \
	$(CMTFLAGS) \
	$(OPTIONS)  

LDFLAGS = \
	-L$(OPLDIR)/lib/$(SYSTEM)/$(LIBFORMAT) \
		-lopl -liljs \
	-L$(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT) \
		-lilocplex -lcp -lconcert \
	-L$(CPLEXDIR)/bin/$(SYSTEM) \
		-l$(CPLEXNAME) \
		-ldl \
	$(LDSYSFLAGS) \
	$(LDMTFLAGS) \
	$(LDOPTIONS)
#---------------------------------------------------------
# FILES
#---------------------------------------------------------
TEST = carseq customdatasource cutstock cutstock_change \
          iterators mulprod mulprod_main oplrunsample steelmill \
          warehouse $(OPLSQLite_DLL) $(OPLODBC_DLL) $(OPLMySQL_DLL)

execute: $(TEST)
	./carseq
	./customdatasource
	./cutstock
	./cutstock_change
	./iterators
	./mulprod
	./mulprod_main
	./oplrunsample
	./steelmill
	./warehouse

build: $(TEST)

clean:
	/bin/rm -rf *.o
	/bin/rm -rf $(TEST)
	touch out.out
	/bin/rm -rf *.out

carseq.o: $(DEMOSRCDIR)/carseq.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/carseq.$(SUFFIX) -c
carseq: carseq.o
	$(CCC) $(CFLAGS) carseq.o -o carseq $(LDFLAGS)

customdatasource.o: $(DEMOSRCDIR)/customdatasource.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/customdatasource.$(SUFFIX) -c
customdatasource: customdatasource.o
	$(CCC) $(CFLAGS) customdatasource.o -o customdatasource $(LDFLAGS)

cutstock.o: $(DEMOSRCDIR)/cutstock.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/cutstock.$(SUFFIX) -c
cutstock: cutstock.o
	$(CCC) $(CFLAGS) cutstock.o -o cutstock $(LDFLAGS)

cutstock_change.o: $(DEMOSRCDIR)/cutstock_change.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/cutstock_change.$(SUFFIX) -c
cutstock_change: cutstock_change.o
	$(CCC) $(CFLAGS) cutstock_change.o -o cutstock_change $(LDFLAGS)

iterators.o: $(DEMOSRCDIR)/iterators.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/iterators.$(SUFFIX) -c
iterators: iterators.o
	$(CCC) $(CFLAGS) iterators.o -o iterators $(LDFLAGS)

mulprod.o: $(DEMOSRCDIR)/mulprod.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/mulprod.$(SUFFIX) -c
mulprod: mulprod.o
	$(CCC) $(CFLAGS) mulprod.o -o mulprod $(LDFLAGS)

mulprod_main.o: $(DEMOSRCDIR)/mulprod_main.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/mulprod_main.$(SUFFIX) -c
mulprod_main: mulprod_main.o
	$(CCC) $(CFLAGS) mulprod_main.o -o mulprod_main $(LDFLAGS)

oplrunsample.o: $(DEMOSRCDIR)/oplrunsample.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/oplrunsample.$(SUFFIX) -c
oplrunsample: oplrunsample.o
	$(CCC) $(CFLAGS) oplrunsample.o -o oplrunsample $(LDFLAGS)

steelmill.o: $(DEMOSRCDIR)/steelmill.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/steelmill.$(SUFFIX) -c
steelmill: steelmill.o
	$(CCC) $(CFLAGS) steelmill.o -o steelmill $(LDFLAGS)

warehouse.o: $(DEMOSRCDIR)/warehouse.$(SUFFIX)
	$(CCC) $(CFLAGS) $(DEMOSRCDIR)/warehouse.$(SUFFIX) -c
warehouse: warehouse.o
	$(CCC) $(CFLAGS) warehouse.o -o warehouse $(LDFLAGS)


ifneq ($(OPLSQLite_DLL),)
libopltabSQLite.o: $(DEMOSRCDIR)/libopltabSQLite.c
	$(CC) $(CFLAGS) $(SQLite_INCLUDE) $(DEMOSRCDIR)/libopltabSQLite.c -c
$(OPLSQLite_DLL): libopltabSQLite.o
	$(CC) $(CFLAGS) libopltabSQLite.o -shared -o $(OPLSQLite_DLL) $(LDFLAGS) $(SQLite_LIB)
endif

ifneq ($(OPLODBC_DLL),)
libopltabODBC.o: $(DEMOSRCDIR)/libopltabODBC.c
	$(CC) $(CFLAGS) $(ODBC_INCLUDE) $(DEMOSRCDIR)/libopltabODBC.c -c
$(OPLODBC_DLL): libopltabODBC.o
	$(CC) $(CFLAGS) libopltabODBC.o -shared -o $(OPLODBC_DLL) $(LDFLAGS) $(ODBC_LIB)
endif

ifneq ($(OPLMySQL_DLL),)
libopltabMySQL.o: $(DEMOSRCDIR)/libopltabMySQL.c
	$(CC) $(CFLAGS) $(MySQL_INCLUDE) $(DEMOSRCDIR)/libopltabMySQL.c -c
$(OPLMySQL_DLL): libopltabMySQL.o
	$(CC) $(CFLAGS) libopltabMySQL.o -shared -o $(OPLMySQL_DLL) $(LDFLAGS) $(MySQL_LIB)
endif
