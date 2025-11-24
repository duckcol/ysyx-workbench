TOPNAME = antpc

CPP_CODE=$(wildcard csrc/*.cpp csrc/*.c)
VLOG_CODE=$(wildcard vsrc/*.v)
WAVEFORM_FILE=$(wildcard *.fst *.vcd)

IMG_PATH := $(YSYX_HOME)/am-kernels/tests/cpu-tests/build
IMG := $(wildcard image/*.bin)

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
	$(shell mkdir -p $(BUILD_DIR))

$(BIN): $(VLOG_CODE) $(CPP_CODE)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $(VLOG_CODE) $(CPP_CODE)\
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		--exe

gdb: $(BIN)
	gdb $(BIN)

run: $(BIN)
	$(shell realpath ./obj_dir/V$(TOPNAME)) $(shell realpath $(IMG))

check_waveform: $(WAVEFORM_FILE)
	@gtkwave $(WAVEFORM_FILE)

file_comfirm: $(IMG)
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"
	@echo "waveform file is $(WAVEFORM_FILE)"
	@echo "img src path is $(IMG_PATH)"
	@echo "img: $(IMG)"
	@verilator --lint-only -Wall $(VLOG_CODE)

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

.PYONY: clean sim file_comfirm run gdb get-img

clean:
	@rm -rf $(BUILD_DIR) obj_dir *.fst* image

include ../Makefile
