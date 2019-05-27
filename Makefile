#####################################################
# You may need to change the parameters under here. #
#####################################################

# Step 1: Choose a compiler. By default, use clang++

# If you use clang++, make sure the following line does not start with a comment (#)
# CXX=clang++
# If you use g++, uncomment the following line
CXX=g++

# Set default compiler parameters
# -Wall 	shows all warnings when compiling, always use this!
# -std=c++11 	enables the C++11 standard mode
CXXFLAGS = -Wall -std=c++11

#######################
# RakNet 4.x location #
#######################

RAKNETDIR = ../RakNet-4.069

# On MinGW, remove comment from the following line:
#EXTRALIBS = -lws2_32 -lwinmm
# On Linux, remove comment from the following line:
EXTRALIBS = -lpthread
# On Mac, remove comment from the following line:
#EXTRALIBS = -framework CoreFoundation
# Compiler flags for the pp
APP_CXXFLAGS = $(CXXFLAGS) -Iinclude -I$(RAKNETDIR)/Source
# Linker flags (order the compiler to link with our library)
LFLAGS += -L$(RAKNETDIR)/Lib/LibStatic -lRakNetLibStatic $(EXTRALIBS)

# The object for the apps
CLIENTOBJS = obj/client.o 
SERVEROBJS = obj/server.o 
# The names of the apps
CLIENTAPP = bin/StockfishClient
SERVERAPP = bin/StockfishServer

# Step 2: If you use clang++ under Mac OS X, remove these comments
#CXXFLAGS += -stdlib=libc++
#LFLAGS += -stdlib=libc++

# Step 3: Run 'make' in the same directory as this file

################################
# Settings for the  app #
################################

# The object for the testing app
TESTOBJS = obj/uci_test.o

# The name of the testing app
PROGRAM = bin/uci_test

# This is the first target. It will be built when you run 'make' or 'make build'
build: $(PROGRAM)

# Rule for linking the test app with our library
$(PROGRAM): $(TESTOBJS) $(CLIENTOBJS) $(SERVEROBJS)
	$(CXX) $(TESTOBJS) -o $(PROGRAM) $(LFLAGS) 

# Compile each source file of the library
obj/uci_test.o: src/uci_test.cpp
	$(CXX) $(APP_CXXFLAGS) -c $? -o $@
obj/client.o: src/client.cpp
	$(CXX) $(APP_CXXFLAGS) -c $? -o $@
obj/server.o: src/server.cpp
	$(CXX) $(APP_CXXFLAGS) -c $? -o $@

clean:
	rm -f $(TESTOBJS)
	rm -f $(PROGRAM)
