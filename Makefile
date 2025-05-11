# Define the build directory
BUILD_DIR = build

SRC_FILES = src/*.cpp src/*.hpp src/*.h src/CMakeLists.txt \
			src/interpret_generator.* \
			src/spec_parser/* \
			src/interpret/fsm_core/* \
			src/PortAddRemoveWidget.* \
			src/DynamicPortsModel.* \
			README.md Doxyfile

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

pack:
	zip -r xambruj00-xkovarj00-xcapikm00.zip $(SRC_FILES)