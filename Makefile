###############################################
#
# Make file for Distributed file server
# Author: Sunil bn <sunhick@gmail.com>
#
# Set the $LD_LIBRARY_PATH to the current
# directory to load shared objects (*.so)
#
# ############################################

CC = g++
DBUG = -g -O0
LDFLAGS = -L./ -pthread -lssl -lcrypto
CCFLAGS = -Wall -std=c++11 $(DBUG) -I $(IDIR)

IDIR = ./include
DFS = dfs
DFC = dfc
TEST = test

# communication protocol files
COMM_SRC = dfcomm.cc dfproto.cc
COMM_OBJS = $(COMM_SRC:.cc=.o)
LIB_COMM = libdfcomm.so

# configuration and utils files
CFG_SRC = dfutils.cc dfconfig.cc
CFG_OBJS = $(CFG_SRC:.cc=.o)
LIB_CFG = libdfconf.so

LDFLAGS += -ldfconf -ldfcomm

# Source files for dfs
DFS_SRC = dfchunksrv.cc dfmaster.cc testchunk.cc
DFS_OBJS = $(DFS_SRC:.cc=.o)

# Source files for dfc
DFC_SRC = dfclient.cc
DFC_OBJS = $(DFC_SRC:.cc=.o)

# Source files for Test
TEST_SRC = dfchunksrv.cc testchunk.cc 
TEST_OBJS = $(TEST_SRC:.cc=.o)

OBJS = $(DFC_OBJS) $(DFS_OBJS) $(TEST_OBJS) $(COMM_OBJS) $(CFG_OBJS)

# SRC = $(\wildcard df*.cc)
# OBJS = $(\SRC:.cc=.o)

TARBALL = dfs.tar.gz

ifeq ($(DEBUG), 1)
	DBUG += -DDEBUG
endif

all:  $(DFS) $(DFC) $(TEST)

$(LIB_COMM): $(COMM_SRC)
	$(CC) -Wall -fPIC $(CCFLAGS) -c $(COMM_SRC)
	$(CC) -shared -Wl,-soname,$@.1 -o $@.1.0  $(COMM_OBJS)
	ln -sf $@.1.0 $@.1
	ln -sf $@.1.0 $@

$(LIB_CFG): $(CFG_SRC)
	$(CC) -Wall -fPIC $(CCFLAGS) -c $(CFG_SRC)
	$(CC) -shared -Wl,-soname,$@.1 -o $@.1.0  $(CFG_OBJS)
	ln -sf $@.1.0 $@.1
	ln -sf $@.1.0 $@

$(COMM_OBJS): $(COMM_SRC)
	$(CC) -Wall -fPIC -c $<

$(CFG_OBJS): $(CFG_SRC)
	$(CC) -Wall -fPIC -c $<

$(TEST): $(TEST_OBJS) $(LIB_COMM) $(LIB_CFG)
	$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS)

$(DFC): $(DFC_OBJS) $(LIB_COMM) $(LIB_CFG)
	$(CC) $(DFC_OBJS) -o $@ $(LDFLAGS)

$(DFS): $(DFS_OBJS) $(LIB_COMM) $(LIB_CFG)
	$(CC) $(DFS_OBJS) -o $@ $(LDFLAGS)

%.o: %.cc
	$(CC) -c -o $@ $(CCFLAGS) $<

.PHONY: clean
clean:
	@rm $(OBJS) $(DFS) $(DFC) $(TEST) *~ include/*~ $(TARBALL) $(LIB_COMM) $(LIB_COMM).1 $(LIB_COMM).1.0 $(LIB_CFG) $(LIB_CFG).1 $(LIB_CFG).1.0

# create a source tar ball 
tar:
	tar -cvzf $(TARBALL) *.* include/*.* README Makefile
