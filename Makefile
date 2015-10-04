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

DFS_SRC = dfcomm.cc dfconfig.cc dfchunksrv.cc dfmaster.cc
DFS_OBJS = $(DFS_SRC:.cc=.o)

DFC_SRC = dfclient.cc dfcomm.cc dfconfig.cc
DFC_OBJS = $(DFC_SRC:.cc=.o)

TEST_SRC = testserver.cc 
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
	$(CC) $(LDFLAGS) $< -o $@

$(DFC): $(DFC_OBJS)
	$(CC) $(LDFLAGS) $< -o $@

$(DFS): $(DFS_OBJS)
	$(CC) $(LDFLAGS) $< -o $@

%.o: %.cc
	$(CC) -c -o $@ $(CCFLAGS) $<

.PHONY: clean
clean:
	@rm $(OBJS) $(DFS) $(DFC) $(TEST) *~ include/*~ $(TARBALL)

# create a web server source tar ball 
tar:
	tar -cvzf $(TARBALL) *.* include/*.* README Makefile
