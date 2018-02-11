ifeq ($(SCIDB),) 
  X := $(shell which scidb 2>/dev/null)
  ifneq ($(X),)
    X := $(shell dirname ${X})
    SCIDB := $(shell dirname ${X})
  endif
endif

# A way to set the 3rdparty prefix path that is convenient
# for SciDB developers.
ifeq ($(SCIDB_VER),)
  SCIDB_3RDPARTY = $(SCIDB)
else
  SCIDB_3RDPARTY = /opt/scidb/$(SCIDB_VER)
endif

# A better way to set the 3rdparty prefix path that does
# not assume an absolute path. You can still use the above
# method if you prefer.
ifeq ($(SCIDB_THIRDPARTY_PREFIX),) 
  SCIDB_THIRDPARTY_PREFIX := $(SCIDB_3RDPARTY)
endif

$(info Using SciDB path $(SCIDB))
SCIDB_VARIANT=$(shell $(SCIDB)/bin/scidb --ver | head -n 1 | cut -d : -f 2 | cut -d '.' -f 1,2 | sed -e "s/\./ *100 + /" | bc)

INSTALL_DIR = $(SCIDB)/lib/scidb/plugins

# Include the OPTIMIZED flags for non-debug use
OPTIMIZED=-O2 -DNDEBUG
DEBUG=-g -ggdb3
CFLAGS = -pedantic -W -Wextra -Wall -Wno-variadic-macros -Wno-strict-aliasing \
         -Wno-long-long -Wno-unused-parameter -fPIC -D_STDC_FORMAT_MACROS \
         -Wno-system-headers 
	 $(OPTIMIZED) -D_STDC_LIMIT_MACROS -std=c99
CXXFLAGS = -pedantic -W -Wextra -Wall -Wno-variadic-macros -Wno-strict-aliasing \
         -Wno-long-long -Wno-unused-parameter -fPIC -DSCIDB_VARIANT=$(SCIDB_VARIANT) $(OPTIMIZED)
INC = -I. -DPROJECT_ROOT="\"$(SCIDB)\"" -I"$(SCIDB_THIRDPARTY_PREFIX)/3rdparty/boost/include/" \
      -I"$(SCIDB)/include" -I./extern

LIBS = -shared -Wl,-soname,libdev_tools.so -ldl -L. \
       -L"$(SCIDB_THIRDPARTY_PREFIX)/3rdparty/boost/lib" -L"$(SCIDB)/lib" \
       -Wl,-rpath,$(SCIDB)/lib:$(RPATH)

SRCS = Logicalinstall_github.cpp \
       Physicalinstall_github.cpp

# Compiler settings for SciDB version >= 15.7
ifneq ("$(wildcard /usr/bin/g++-4.9)","")
  CC := "/usr/bin/gcc-4.9"
  CXX := "/usr/bin/g++-4.9"
  CXXFLAGS+=-std=c++11 -DCPP11
else
  ifneq ("$(wildcard /opt/rh/devtoolset-3/root/usr/bin/gcc)","")
    CC := "/opt/rh/devtoolset-3/root/usr/bin/gcc"
    CXX := "/opt/rh/devtoolset-3/root/usr/bin/g++"
    CXXFLAGS+=-std=c++11 -DCPP11
  endif
endif


all: libdev_tools.so

clean:
	rm -rf *.so *.o

libdev_tools.so: $(SRCS)
	@if test ! -d "$(SCIDB)"; then echo  "Error. Try:\n\nmake SCIDB=<PATH TO SCIDB INSTALL PATH>"; exit 1; fi
	$(CXX) $(CXXFLAGS) $(INC) -o Logicalinstall_github.o -c Logicalinstall_github.cpp
	$(CXX) $(CXXFLAGS) $(INC) -o Physicalinstall_github.o -c Physicalinstall_github.cpp
	$(CXX) $(CXXFLAGS) $(INC) -o libdev_tools.so plugin.cpp Logicalinstall_github.o Physicalinstall_github.o $(LIBS)
	@echo "Now copy *.so to $(INSTALL_DIR) on all your SciDB nodes, and restart SciDB."
