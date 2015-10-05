###############################################
#
# Make file for Distributed file server
# Author: Sunil bn <sunhick@gmail.com>
#
# ############################################

CC = g++
DBUG = -g -O0
LDFLAGS = -pthread
CCFLAGS = -Wall -std=c++11 $(DBUG) -I $(IDIR)

IDIR = ./include
DFS = dfs
DFC = dfc
TEST = test

# Source files for dfs
DFS_SRC = dfcomm.cc dfconfig.cc dfchunksrv.cc dfmaster.cc testchunk.cc
DFS_OBJS = $(DFS_SRC:.cc=.o)

# Source files for dfc
DFC_SRC = dfcomm.cc dfconfig.cc dfclient.cc
DFC_OBJS = $(DFC_SRC:.cc=.o)

# Source files for Test
TEST_SRC = dfcomm.cc dfchunksrv.cc testchunk.cc 
TEST_OBJS = $(TEST_SRC:.cc=.o)

OBJS = $(DFC_OBJS) $(DFS_OBJS) $(TEST_OBJS)

# SRC = $(\wildcard df*.cc)
# OBJS = $(\SRC:.cc=.o)

TARBALL = dfs.tar.gz

ifeq ($(DEBUG), 1)
	DBUG += -DDEBUG
endif

all: $(DFS) $(DFC) $(TEST)

$(TEST): $(TEST_OBJS)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

$(DFC): $(DFC_OBJS)
	$(CC) $(LDFLAGS) $(DFC_OBJS) -o $@

$(DFS): $(DFS_OBJS)
	$(CC) $(LDFLAGS) $(DFS_OBJS) -o $@

%.o: %.cc
	$(CC) -c -o $@ $(CCFLAGS) $<

.PHONY: clean
clean:
	@rm $(OBJS) $(DFS) $(DFC) $(TEST) *~ include/*~ $(TARBALL)

# create a source tar ball 
tar:
	tar -cvzf $(TARBALL) *.* include/*.* README Makefile
