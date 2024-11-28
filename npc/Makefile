CPP_CODE=$(wildcard csrc/*.cpp csrc/*.c)
VLOG_CODE=$(wildcard vsrc/*.v)

file_confirm:
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"	

all:
	@echo "Write this Makefile by your self."
	verilator --cc --exe --trace --build -j 0 -Wall $(CPP_CODE) $(VLOG_CODE)

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."

include ../Makefile
