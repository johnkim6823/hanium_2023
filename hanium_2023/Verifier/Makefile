#--compiler----
CXX = g++

#--Flages----
SIGN_VER_FLAGS = -lssl -lcrypto -L/usr/local/opt/openssl/lib
MYSQL_FLAGS =  -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient 
OPENCV = `pkg-config opencv4 --cflags --libs`
LIBS = $(OPENCV) -lpthread

#--sources----
SOURCES :=  Verifier.cpp 

#--Object files----
OBJS = $(SOURCES:.cpp=.o)

Verifier: $(OBJS)
	$(CXX) $(OBJS) -g -o $@  $(LIBS) $(MYSQL_FLAGS) $(SIGN_VER_FLAGS)


%.o: ./%.cpp
	$(CXX) -c $< -g -o $@  $(LIBS) $(MYSQL_FLAGS) $(SIGN_VER_FLAGS)

clean:
	$(RM) *.o Verifier
