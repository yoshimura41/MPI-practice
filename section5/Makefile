MPICC=time mpic++
CPPFLAGS=-std=c++1z -O2 -Wall

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=)

all: $(OBJS)

%: %.cpp
	$(MPICC) $(CPPFLAGS) $< -o $@.out

clean:
	rm -f *.o *~ *.out

.PHONY: clean