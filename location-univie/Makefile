# relative paths of source dir, object dir and executable
sourcedir = src
objectdir = obj
executable = loc-uv

# compute list of all .cpp files in sourcedir
# for each of them, compute the .o file they should be compiled into
sourcefiles = $(shell find $(sourcedir) -name '*.cpp')
objectfiles = $(patsubst $(sourcedir)/%.cpp, $(objectdir)/%.o, $(sourcefiles))
dependencyfiles = $(patsubst $(objectdir)/%.o, $(objectdir)/%.d, $(objectfiles))

# paths of CPLEX dirs
cplexdir = /usr/lib/CPLEX

# path of e4share-basic
e4share-basic = ../basic

# search paths for header files
includes = -DIL_STD -isystem $(cplexdir)/concert/include -isystem $(cplexdir)/cplex/include  -I $(e4share-basic)/src -I src

# search paths for linked libraries
libdirs = -L $(cplexdir)/concert/lib/x86-64_sles10_4.1/static_pic -L $(cplexdir)/cplex/lib/x86-64_sles10_4.1/static_pic -L $(e4share-basic)

# linked libraries
libs = -lilocplex -lcplex -lconcert -lpthread -lboost_program_options -lboost_filesystem -lboost_system -le4share-basic

# c++ compiler and its flags for compiling and linking
CXX = clang++
CXXFLAGS = -Wall -Wextra -MMD -std=c++11 $(includes)
LDFLAGS = $(libdirs) $(libs) -static


all: debug

# include all dependency files, if they exist
-include $(dependencyfiles)

.PHONY: all debug release clean

debug: CXXFLAGS += -g3 -ggdb -O0
debug: $(executable)

release: CXXFLAGS += -O3
release: $(executable)

$(executable): $(objectfiles)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(objectdir)/%.o : $(sourcedir)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(objectdir)
	rm -f $(executable)

