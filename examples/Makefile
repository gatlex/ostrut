## To build the project with this makefile make sure that the library
## and include paths are correctly set, e.g. with the current setup:
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/Development/local-libraries/boost/stage/lib
# export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:${HOME}/Development/local-libraries/boost/
#
## To execute the binaries also update the dynamic library path, e.g.:
# export LIBRARY_PATH=$LIBRARY_PATH:${HOME}/Development/local-libraries/boost/stage/lib

SUBDIRS = 
DOCDIR = doc
INCLUDEDIR=../include/

BIN = basics hello-container-world

# Debug:
CXXFLAGS = -g -std=c++11 -pedantic -Wall -I$(INCLUDEDIR) -O0 -DDEBUG
LDFLAGS = -g

# # Release:
# CXXFLAGS = -g -std=c++11 -pedantic -Wall -I$(INCLUDEDIR) -O2 -DNDEBUG
# LDFLAGS = -g

DOXYGEN = doxygen

.PHONY: all flatclean clean doc $(SUBDIRS)

all:	$(BIN) $(SUBDIRS)

flatclean:
	rm -rf $(BIN) $(DOCDIR)/*

clean:	flatclean
	for d in $(SUBDIRS) ; do $(MAKE) -C $$d clean ; done

# Implicite rule to make binaries from cc-files.
%: %.cc $(INCLUDEDIR)/ostrut/*.hh Makefile
	$(CXX) -o $@ $<  ${CXXFLAGS} ${LDFLAGS}

doc:
	$(DOXYGEN) doxygen.conf

$(SUBDIRS):
	$(MAKE) -C $@ all

