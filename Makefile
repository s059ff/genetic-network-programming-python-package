# If optimize compiling, set to RELEASE.
# BUILD_TYPE := DEBUG
BUILD_TYPE := RELEASE

# If enable multi thread, set to TRUE.
# ENABLE_OPENMP=TRUE

# If enable multi thread, set to the number of threads.
# OMP_NUM_THREADS=16

# If use double precision floating point, set to TRUE.
# GNP_USE_DOUBLE_PRECISION=TRUE

# Write the C++ 'Eigen' library path.
EIGEN_PATH := ~/eigen/

# Write the C++ 'picojson' library path.
PICOJSON_PATH := ~/picojson/

# Write the Anaconda install path.
ANACONDA_PATH := ~/anaconda3/

CC := clang++
FLAGS := -std=c++14 -fPIC -Wall -Wextra -Wno-conversion -Wno-sign-compare -Wno-unused-parameter -Wno-missing-field-initializers
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
INCLUDE := -I $(ANACONDA_PATH)include/ -I $(ANACONDA_PATH)include/python3.6m/ -I $(EIGEN_PATH) -I $(PICOJSON_PATH)
LINK := -L $(ANACONDA_PATH)lib
LIBS := -lboost_python3 -lboost_numpy3

ifeq ($(BUILD_TYPE), RELEASE)
	FLAGS+= -O2 -DNDEBUG
endif
ifeq ($(ENABLE_OPENMP), TRUE)
	FLAGS+= -fopenmp -DOMP_NUM_THREADS=$(OMP_NUM_THREADS)
endif
ifeq ($(GNP_USE_DOUBLE_PRECISION), TRUE)
	FLAGS+= -DGNP_USE_DOUBLE_PRECISION
endif	

all: $(OBJS)
	$(CC) $(FLAGS) -shared $(LINK) $(OBJS) $(LIBS) -o gnp.so
	for d in examples/*/; do cp gnp.so $$d; done

clean:
	rm -f *.o
	rm -f *.so
	
%.o: %.cpp Makefile *.h
	$(CC) $(FLAGS) $(INCLUDE) -c $<
