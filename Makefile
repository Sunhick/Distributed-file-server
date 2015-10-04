###############################################
#
# Make file for Distributed file server
# Author: Sunil bn <sunhick@gmail.com>
#
# ############################################

CC = g++
DEBUG = -g -O0
LDFLAGS = -pthread
CCFLAGS = -Wall -std=c++11 $(DEBUG) -I $(IDIR)

IDIR = ./include
Exe = dfs

# Discover all .cc files in src/ and try to compile them
SRC = $(wildcard *.cc)
OBJS = $(SRC:.cc=.o)

TARBALL = dfs.tar.gz


all: $(Exe)

$(Exe): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(Exe)

%.o: %.cc
	$(CC) -c -o $@ $(CCFLAGS) $<

.PHONY: clean
clean:
	@rm $(OBJS) $(Exe) *~ include/*~ $(TARBALL)

# create a web server source tar ball 
tar:
	tar -cvzf $(TARBALL) *.* include/*.* README Makefile
