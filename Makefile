AR := ar rcu
RANLIB := ranlib
SRC := $(filter-out src/main.cpp, $(wildcard src/*.cpp))
HDR := $(wildcard src/*.hpp)
OBJ := $(SRC:.cpp=.o)
H5I := -I$(HOME)/Software/hdf5-1.8.17/include
H5L := -L$(HOME)/Software/hdf5-1.8.17/lib -lhdf5
CXX := mpicxx
CFLAGS := -std=c++11 -Wall -O0 -g

default : libcow.a

src/HDF5.o : src/HDF5.cpp $(HDR)
	$(CXX) $(CFLAGS) -o $@ -c $< $(H5I)

%.o : %.cpp $(HDR)
	$(CXX) $(CFLAGS) -o $@ -c $<

cow : src/main.o $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ $(H5L)

libcow.a : $(OBJ)
	$(AR) $@ $?
	$(RANLIB) $@

show :
	@echo $(SRC)
	@echo $(OBJ)

doxygen :
	doxygen Doxygen.conf

clean :
	$(RM) $(OBJ) src/main.o cow
