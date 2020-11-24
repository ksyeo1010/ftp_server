#The following lines contain the generic build options
SRC_DIR=../c
INC_DIR=../h
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration -I$(INC_DIR)

#List all the .o files here that need to be linked 
OBJS=CSftp.o usage.o dir.o

all: CSftp

CSftp: $(OBJS) 
	$(CC) -o CSftp $(OBJS)

clean:
	rm -f *.o
	rm -f CSftp

.PHONY: run
run: CSftp  
	./CSftp $(RUN_ARGS)

usage.o: $(SRC_DIR)/usage.c $(INC_DIR)/usage.h
dir.o: $(SRC_DIR)/dir.c $(INC_DIR)/dir.h
CSftp.o: $(SRC_DIR)/CSftp.c $(INC_DIR)/dir.h $(INC_DIR)/usage.h

$(OBJS):
	$(CC) $(CFLAGS) -c $< -o $@