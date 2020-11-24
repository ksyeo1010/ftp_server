#This is a hack to pass arguments to the run command and probably only 
#works with gnu make. 
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif


MAKE = make
COMPILE=compile
EXE=CSftp

all:
	cd $(COMPILE); $(MAKE) all

clean:
	cd $(COMPILE); $(MAKE) clean

run:
	cd $(COMPILE); ./$(EXE) $(RUN_ARGS)