ARCH_NAME = $(shell uname -m)
ifeq (${ARCH_NAME}, x86_64)
ARCH = 64
else
ARCH = 32
endif

############# CPLEX #########
#By default CPLEXDIR is set to "", which automatically disables CPLEX.
#If you want to use CPLEX, modify SYSTEM, CPLEXDIR and CONCERTDIR to values for your local system.

CPLEXDIR      = /mnt/aperto/artem/ModuleDiscovery/external/ibm-cplex/cplex
CONCERTDIR    = /mnt/aperto/artem/ModuleDiscovery/external/ibm-cplex/concert

ifneq (${CPLEXDIR},)
ifeq (${ARCH},32)
SYSTEM	= x86_linux
else
SYSTEM     = x86-64_linux
endif
# CPLEX Make stuff based on /com/ilog/cplex112/examples/x86-64_debian4.0_4.1/static_pic/Makefile
LIBFORMAT  = static_pic
CPLEXLIBDIR	= $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR	= $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)


ILOGINCLUDE = -I $(CPLEXDIR)/include  -I $(CONCERTDIR)/include 
ILOGLIBS =  -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -lpthread
#Note: the "-DCPLEX" part tells chainedSolvers.cpp that CPLEX is available. 
ILOGFLAGS = -fPIC -fexceptions -DIL_STD  -DCPLEX 
#Some CPLEX makefiles have -DLONG_MAX=2147483647 since the CPLEX headers seem to refer to this but it isn't defined by default on our system somehow. For 64-bit compatibility I instead #include <limits.h> before including the CPLEX include files.
endif

############# DSDP ############
#By default DSDPDIR is set to "", which automatically disables DSDP.
#If you want to use DSDP, set DSDPDIR appropriately for your system.

ifeq (${ARCH}, 32)
DSDPDIR = /mnt/aperto/artem/ModuleDiscovery/external/DSDP5.8
else
DSDPDIR =
endif

ifneq (${DSDPDIR},)
DSDPINCLUDE = -I $(DSDPDIR)/include -DDSDP_SDP
DSDPLIBDIR = $(DSDPDIR)/lib
DSDPLIB = -L$(DSDPLIBDIR) -ldsdp
LAPACKBLAS = -llapack -lblas -lm
DSDPLIBS = $(DSDPLIB) $(LAPACKBLAS)
endif

########## Conic Bundle ###############
#By default CBUNDLEDIR is set to "", which automatically disables Conic Bundle.
#If you want to use Conic Bundle, set CBUNDLEDIR appropriately for your system.

ifeq (${ARCH}, 32)
CBUNDLEDIR =
else
CBUNDLEDIR =
endif

ifneq (${CBUNDLEDIR},)
CBUNDLEINCLUDE = -I $(CBUNDLEDIR)/include  -I $(CBUNDLEDIR)/CBsources  -I $(CBUNDLEDIR)/Matrix  -I $(CBUNDLEDIR)/Tools -DCBUNDLE_SDP
CBUNDLELIBDIR = $(CBUNDLEDIR)/lib
CBUNDLELIB = -L$(CBUNDLELIBDIR) -lcb -lm
endif

########## End Conic Bundle ###########

ifeq (${ARCH}, 64)
LIB = lib64
BIN = bin64
else
LIB = lib32
BIN = bin32
endif

WARNINGS = -Woverloaded-virtual -Wall -Wno-sign-compare

CFLAGS = $(WARNINGS) -Iinclude $(ILOGINCLUDE) $(ILOGFLAGS) $(DSDPINCLUDE) $(CBUNDLEINCLUDE)
LIBFLAGS = $(ILOGLIBS) $(DSDPLIBS) $(CBUNDLELIB)
#OPTFLAGS = -pg 
OPTFLAGS = -O3

vpath %.h include
vpath %.cc src
vpath %.rst doc

LIBS = \
	$(LIB)/common.o \
	$(LIB)/WeightMatrix.o \
	$(LIB)/Greedy.o \
	$(LIB)/BestOneEltMove.o

ifneq (${CBUNDLEDIR},)
LIBS := $(LIBS) $(LIB)/SDPSolverTwo.o
endif

ifneq (${CPLEXDIR},)
LIBS := $(LIBS) $(LIB)/LPSolver.o
endif

ifneq (${DSDPDIR},)
LIBS := $(LIBS) $(LIB)/SDPSolver.o
endif



.SUFFIXES : 
.PRECIOUS : $(LIB)/%.o

something :
	@echo "So, what should I make now?"

$(LIB)/%.o : %.cc %.h
	g++ $(CFLAGS) $(OPTFLAGS) -c -o $@ $<

$(LIB)/%.o : %.cc
	g++ $(CFLAGS) $(OPTFLAGS) -c -o $@ $<

%.html : %.rst
	rst2html $< > doc/$@


% : $(LIB)/%.o $(LIBS)
	g++ $(OPTFLAGS) -o $(BIN)/$@ $^ $(CFLAGS) $(LIBFLAGS)

.PHONY : clean

clean:
	-rm $(LIB)/*
	-rm $(BIN)/*
