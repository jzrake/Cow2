# =====================================================================
# Cow build system
# =====================================================================
#
#
# External library dependencies: HDF5, MPI
# Embedded library dependencies: None
#
#
# Notes
# -----
#
# - A useful resource for techniques to process Makefile dependencies:
# www.microhowto.info/howto/automatically_generate_makefile_dependencies.html
#
# - Using -O0 rather than -O3 during development may reduce compilation time
# significantly.


# Build configuration
# =====================================================================
#
# If a Makefile.in exists in this directory, then use it.
#
-include Makefile.in
#
# Any macros that are omitted receive these default values:
LUA_ARCH ?= generic
AR       ?= ar rcu
RANLIB   ?= ranlib
CXXFLAGS ?= -std=c++14 -Wall -O0 -g
CXX      ?= mpicxx
H5I      ?= -I/usr/include
H5L      ?= -L/usr/lib -lhdf5


# Build macros
# =====================================================================
SRC      := $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ      := $(SRC:%.cpp=%.o)
DEP      := $(SRC:%.cpp=%.d)
CXXFLAGS += -MMD -MP
CXXFLAGS += $(H5I)
LDFLAGS  += $(H5L)


# Build rules
# =====================================================================
#
default: cow src/libcow.a

cow: src/main.o $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

src/libcow.a: $(OBJ)
	$(AR) $@ $?
	$(RANLIB) $@

show:
	@echo $(SRC)
	@echo $(OBJ)

doxygen:
	doxygen Doxygen.conf

clean:
	$(RM) $(OBJ) src/main.o src/libcow.a cow

-include $(DEP)
