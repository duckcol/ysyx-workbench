TOPNAME = antpc

CPP_CODE=$(wildcard csrc/*.cpp csrc/*.c)
VLOG_CODE := $(sort $(shell find vsrc -type f -name "*.v"))
WAVEFORM_FILE=$(wildcard *.fst *.vcd)

# IMG_PATH := $(YSYX_HOME)/am-kernels/tests/cpu-tests/build
# IMG := $(wildcard image/*.bin)

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

DEFAULT_IMG = ./dummy-riscv32e-npc.bin
IMG ?= ./dummy-riscv32e-npc.bin

CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""

all: $(BIN)
	$(shell mkdir -p $(BUILD_DIR))

$(BUILD_DIR):
	$(shell mkdir -p $@)

$(BIN): $(VLOG_CODE) $(CPP_CODE) | $(BUILD_DIR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $(VLOG_CODE) $(CPP_CODE)\
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		--exe

gdb: $(BIN)
	gdb $(BIN)

run: $(BIN)
	@echo exe: $(shell realpath $(BIN))
	@echo img: $(shell realpath $(IMG))
	@if [ "$(IMG)" = "$(DEFAULT_IMG)" ]; then \
		echo USING DEFAULT_IMG; \
	fi
	$(shell realpath $(BIN)) $(shell realpath $(IMG))

check_waveform: $(WAVEFORM_FILE)
	@gtkwave $(WAVEFORM_FILE)

file_comfirm: $(IMG)
	@echo "cpp code is $(CPP_CODE)"
	@echo "verilog code is $(VLOG_CODE)"
	@echo "waveform file is $(WAVEFORM_FILE)"
	@echo "img src path is $(IMG_PATH)"
	@echo "img: $(IMG)"
	@echo "Verlator Linter Result:"
	@verilator --lint-only -Wall $(VLOG_CODE)

sim: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

.PYONY: clean sim file_comfirm run gdb get-img

clean:
	@rm -rf $(BUILD_DIR) obj_dir *.fst* image

include ../Makefile
