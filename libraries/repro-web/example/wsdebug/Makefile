CXX = g++
BACKEND=libevent
BUILDCHAIN = make

CONTAINER = $(shell echo "reproweb_example_$(CXX)_$(BACKEND)_$(BUILDCHAIN)" | sed 's/++/pp/')
BASE_CONTAINER = $(shell echo "reproweb_$(CXX)_$(BACKEND)_$(BUILDCHAIN)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)

DEBUG = -g -DMOL_PROMISE_DEBUG
release : override DEBUG = -O3

LIBEVENT=$(shell pkg-config libevent_pthreads --libs)
PKG_INC=$(shell pkg-config cryptoneat reprocpp --cflags)
PKG_LIB=$(shell pkg-config jsoncpp cryptoneat libnghttp2 sqlite3 zlib libcurl hiredis --libs)

# if not g++ we assume clang++
DEFAULT_LIBS = -stdlib=libc++ -fcoroutines-ts  -lc++abi -std=c++17
DEFAULT_OPTS = -std=c++17 -stdlib=libc++ -fcoroutines-ts -D_RESUMABLE_FUNCTIONS_SUPPORTED 

ifeq ($(CXX),g++)
DEFAULT_OPTS = -std=c++17 
DEFAULT_LIBS = 
endif

#backend switch libevent/boost_asio
PROMISE_IMPL = -DPROMISE_USE_LIBEVENT
PROMISE_3DPARTY = $(LIBEVENT) $(DEFAULT_LIBS)

ifeq ($(BACKEND),boost_asio)
        PROMISE_IMPL = -DPROMISE_USE_BOOST_ASIO
        PROMISE_3DPARTY = -lboost_system $(DEFAULT_LIBS)
endif

#final cpp options
OPTIONS = -g -fpic -Wno-write-strings -pthread -D_REENTRANT $(DEFAULT_OPTS) $(PKG_INC)
CPPFLAGS = -Wall -I../include $(DEBUG) $(OPTIONS) $(PROMISE_IMPL)

# target
LIB = -lreprowebd -lpriohttpd  -lreprocurld -lreproredisd -lreprosqlited -lpriocppd
DEBUG_LIB = -lreproweb -lpriohttp  -lreprocurl -lreproredis -lreprosqlite -lpriocpp

BIN_SRC   = .
BIN_BUILD = ./build
BIN_DEBUG_LIBS  = $(DEBUG_LIB)  /usr/lib/libgtest.a -lpthread  $(PKG_LIB) $(PROMISE_3DPARTY) 
BIN_RELEASE_LIBS  = $(LIB)  /usr/lib/libgtest.a -lpthread  $(PKG_LIB) $(PROMISE_3DPARTY) 


# Objects for the test executable
BIN_SRCFILESABS = $(shell ls $(BIN_SRC)/*.cpp)
BIN_SRCFILES =  $(notdir $(BIN_SRCFILESABS))
BIN_OBJFILES = $(BIN_SRCFILES:%.cpp=$(BIN_BUILD)/%.o)
BIN_BINS = $(BIN_BUILD)/hello_world_debug.bin $(BIN_BUILD)/hello_world.bin


#################################################
# rule to compile all (default rule)
#################################################

all:
	make clean_obj
	make $(BIN_BUILD)/hello_world_debug.bin
	make clean_obj
	make $(BIN_BUILD)/hello_world.bin

#################################################
# rules to compile .o files from .cpp source
#################################################

$(BIN_BUILD)/%.o: $(BIN_SRC)/%.cpp 
	-mkdir -p $(BIN_BUILD)
	$(CXX) -c $^ -o $@ $(CPPFLAGS) 


#################################################
# rules to compile the artifacts from .o files
#################################################

$(BIN_BUILD)/hello_world_debug.bin: $(BIN_OBJFILES)
	$(CXX) $^ $(BIN_DEBUG_LIBS) -o $@ 

$(BIN_BUILD)/hello_world.bin: $(BIN_OBJFILES)
	$(CXX) $^ $(BIN_RELEASE_LIBS) -o $@ 

			
#################################################
# make clean
#################################################

clean_obj:
	-find -name "*.o" -exec rm {} \;

clean: clean_obj
	-find -name "*.bin" -exec rm {} \;
	-find -name "*~" -exec rm {} \;
	-rm -rf build

release : all

update-dockerfile:
	/bin/sed -i "s/FROM .*/FROM ${BASE_IMAGE}/" Dockerfile

build: update-dockerfile ## run the docker image and open a shell
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) docker-compose build

up: update-dockerfile ## run the docker image and open a shell
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) docker-compose up -d

down: ## stop running docker image, if any
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) docker-compose stop
	
.PHONY: build up down update-dockerfile