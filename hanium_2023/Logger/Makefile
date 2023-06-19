#--compiler----
CXX = g++

#--Flages----

SIGN_VER_FLAGS = -lssl -lcrypto -L /usr/local/opt/openssl/lib

OPENCV = `pkg-config opencv4 --cflags --libs`
LIBS = $(OPENCV) -lpthread -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
#--sources----
SOURCES :=   command_parser.cpp Logger.cpp

#--Object files----
OBJS = $(SOURCES:.cpp=.o)

all: Logger

Logger: $(OBJS)
	$(CXX)  $(OBJS) -o $@ $(LIBS) $(SIGN_VER_FLAGS)

command_parser.o: ./../L_Client/command_parser.cpp
	$(CXX) $(LIBS) -c -o $@ $<

Logger.o: ./Logger.cpp
	${CXX} -c -o $@  $< $(LIBS) 

clean:
	$(RM) *.o  Logger
