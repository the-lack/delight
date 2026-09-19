# --- c++ config
COMPILER              = g++
FLAG_VERSION          = -std=c++23
FLAG_WARNINGS         = -Wall -Wextra -Wunused-variable -Wpedantic -Wshadow -Wunused -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wformat-security
FLAG_MAX_OPTIMIZATION = -O3
FLAG_OUTPUT_ASM       = -S -masm=intel -fverbose-asm
FLAG_SAVE_INTERMEDIATE_COMPILER_REPRESENTATIONS = -save-temps

# --- build output config
BUILD_DIRECTORY_NAME = build_output
EXECUTABLE_PATH      = $(BUILD_DIRECTORY_NAME)/main.output

# --- entry point, run `make` in terminal
run: clean-build-output compile
	@ ./$(EXECUTABLE_PATH)

# substeps - irrelevant
compile: prepare_build_directory main.cpp
	 $(COMPILER) $(FLAG_VERSION) $(FLAG_WARNINGS) $(FLAG_MAX_OPTIMIZATION) $(FLAG_SAVE_INTERMEDIATE_COMPILER_REPRESENTATIONS) -o $(EXECUTABLE_PATH) main.cpp

prepare_build_directory:
	@ mkdir -p $(BUILD_DIRECTORY_NAME)

clean-build-output:
	@ rm -r $(BUILD_DIRECTORY_NAME) 2>/dev/null || true # ignore errors if build dir does not exist
