# --- c++ config
COMPILER              = gcc
FLAG_VERSION          = -std=gnu89
FLAG_WARNINGS         = -Wall -Wextra -Wunused-variable -Wpedantic -Wshadow -Wunused -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wformat-security
FLAG_MAX_OPTIMIZATION =
# -O3
LLVM_FLAGS            =
# llvm-config --ldflags --system-libs --libs core support
FLAG_OUTPUT_ASM       =
# -S -masm=intel -fverbose-asm
FLAG_SAVE_INTERMEDIATE_COMPILER_REPRESENTATIONS =
# -save-temps

# --- build output config
BUILD_DIRECTORY_NAME = build_output
EXECUTABLE_PATH      = $(BUILD_DIRECTORY_NAME)/main.output

# --- entry point, run `make` in terminal
run: clean-build-output compile
	@ ./$(EXECUTABLE_PATH) testfile.ts

# substeps - irrelevant
compile: prepare_build_directory main.c
	 $(COMPILER) $(FLAG_VERSION) $(FLAG_WARNINGS) $(FLAG_MAX_OPTIMIZATION) $(FLAG_SAVE_INTERMEDIATE_COMPILER_REPRESENTATIONS) main.c `$(LLVM_FLAGS)` -o $(EXECUTABLE_PATH)
	 
prepare_build_directory:
	@ mkdir -p $(BUILD_DIRECTORY_NAME)

clean-build-output:
	@ rm -r $(BUILD_DIRECTORY_NAME) 2>/dev/null || true # ignore errors if build dir does not exist
