# relative paths of source dir, object dir and executable
sourcedir = src
objectdir = obj
libname = e4share-basic
staticlib = lib$(libname).a
sharedlib = lib$(libname).so

# compute list of all .cpp files in sourcedir
# for each of them, compute the .o file they should be compiled into
sourcefiles = $(shell find $(sourcedir) -name '*.cpp')
objectfiles = $(patsubst $(sourcedir)/%.cpp, $(objectdir)/%.o, $(sourcefiles))
dependencyfiles = $(patsubst $(objectdir)/%.o, $(objectdir)/%.d, $(objectfiles))


# c++ compiler and its flags for compiling and linking
CXX = clang++
CXXFLAGS = -Weverything -MMD -std=c++11 -fPIC


all: debug

# include all dependency files, if they exist
-include $(dependencyfiles)

.PHONY: all debug release lib clean

debug: CXXFLAGS += -g3 -ggdb -O0
debug: lib

release: CXXFLAGS += -O3
release: lib

lib: $(staticlib) $(sharedlib)

$(staticlib) : $(objectfiles)
	ar rcs $(staticlib) $(objectfiles)
	
$(sharedlib) : $(objectfiles)
	$(CXX) -o $(sharedlib) -shared $(objectfiles)

$(objectdir)/%.o : $(sourcedir)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(objectdir)
	rm -f $(staticlib) $(sharedlib)

