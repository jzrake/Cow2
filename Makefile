-include Makefile.in

AR ?= ar rcu
RANLIB ?= ranlib
CFLAGS ?= -std=c++11 -Wall -O0 -g
CXX ?= $(HOME)/Software/mpich-3.2/bin/mpicxx
H5I ?= -I$(HOME)/Software/hdf5-1.10.1/include
H5L ?= -L$(HOME)/Software/hdf5-1.10.1/lib -lhdf5
SRC := $(filter-out src/main.cpp, $(wildcard src/*.cpp))
HDR := $(wildcard src/*.hpp)
OBJ := $(SRC:.cpp=.o)

default : src/libcow.a

src/HDF5.o : src/HDF5.cpp $(HDR)
	$(CXX) $(CFLAGS) -o $@ -c $< $(H5I)

%.o : %.cpp $(HDR)
	$(CXX) $(CFLAGS) -o $@ -c $<

cow : src/main.o $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ $(H5L)

src/libcow.a : $(OBJ)
	$(AR) $@ $?
	$(RANLIB) $@

show :
	@echo $(SRC)
	@echo $(OBJ)

doxygen :
	doxygen Doxygen.conf

clean :
	$(RM) $(OBJ) src/main.o cow
