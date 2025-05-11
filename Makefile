# Define the build directory
BUILD_DIR = build

# Default target
all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

# Generate Makefiles using CMake (only if not already done)
$(BUILD_DIR)/Makefile:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ../src

# Clean build files
clean:
	rm -rf $(BUILD_DIR)