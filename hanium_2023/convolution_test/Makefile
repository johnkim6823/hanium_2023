#--compiler----
CXX = g++

#--Flages----
OPENCV = `pkg-config opencv4 --cflags --libs`
LIBS = $(OPENCV)
#--sources----
SOURCES :=  convolution.cpp

#--Object files----
OBJS = $(SOURCES:.cpp=.o)

all: convolution

convolution: $(OBJS)
	$(CXX)  $(OBJS) -o $@ $(LIBS) 

convolution.o: ./convolution.cpp
	${CXX} -c -o $@  $< $(LIBS) 

clean:
	$(RM) *.o convolution
