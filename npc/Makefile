TOPNAME = antpc

CPP_CODE=$(wildcard csrc/*.cpp csrc/*.c)
VLOG_CODE=$(wildcard vsrc/*.v)
WAVEFORM_FILE=$(wildcard *.fst *.vcd)
VERILATOR = verilator
VERILATOR_CFLAGS += --cc --MMD --build -j 0 \
										--x-assign fast \
										--x-initial fast \
										-O3 --noassert \
										--trace-fst
										#-Wall 

BUILD_DIR = ./obj_dir
OBJ_DIR = $(BUILD_DIR)
BIN = $(BUILD_DIR)/V$(TOPNAME)

CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""

all: $(BIN)
	#@echo "Write this Makefile by your self."
	$(shell mkdir -p $(BUILD_DIR))

$(BIN): $(VLOG_CODE) $(CPP_CODE) 
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		--exe

gdb: $(BIN)
	gdb $(BIN)

run: $(BIN)
	./obj_dir/V$(TOPNAME)

check_waveform: $(WAVEFORM_FILE)
	@gtkwave $(WAVEFORM_FILE)

file_comfirm:
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"
	@echo "waveform file is $(WAVEFORM_FILE)"
	@verilator --lint-only -Wall $(VLOG_CODE)

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

.PYONY: clean

clean:
	@rm -rf $(BUILD_DIR) obj_dir *.fst*

include ../Makefile
