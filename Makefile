#This is a hack to pass arguments to the run command and probably only 
#works with gnu make. 
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif

# Makefile based on https://stackoverflow.com/questions/20090616/makefiles-to-include-multiple-headers-in-c
#The following lines contain the generic build options
BIN_NAME=CSftp
SRC_DIR=src
INC_DIR=include
OBJ_DIR=obj
CC=gcc
CPPFLAGS=-I$(INC_DIR)
CFLAGS=-g -Werror-implicit-function-declaration
LIBS=-lpthread

#List all the .o files here that need to be linked
SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN_NAME)

$(BIN_NAME): $(OBJS) 
	$(CC) -o $@ $^

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_NAME)

.PHONY: run
run: $(BIN_NAME)  
	./$(BIN_NAME) $(RUN_ARGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LIBS) -c $< -o $@

# usage.o: $(SRC_DIR)/usage.c $(INC_DIR)/usage.h
# dir.o: $(SRC_DIR)/dir.c $(INC_DIR)/dir.h
# CSftp.o: $(SRC_DIR)/CSftp.c $(INC_DIR)/dir.h $(INC_DIR)/usage.h
