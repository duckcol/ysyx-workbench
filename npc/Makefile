CPP_CODE=$(wildcard csrc/*.cpp csrc/*.c)
VLOG_CODE=$(wildcard vsrc/*.v)

all:
	@echo "Write this Makefile by your self."
	verilator --cc --exe --trace --build -j 0 -Wall $(CPP_CODE) $(VLOG_CODE)

file_comfirm:
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator --cc --exe --trace --build -j 0 -Wall $(CPP_CODE) $(VLOG_CODE)
	

.PYONY: clean

clean:
	rm -rf obj_dir

include ../Makefile
