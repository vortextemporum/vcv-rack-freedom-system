# Set RACK_DIR if not set as environment variable
ifndef RACK_DIR
RACK_DIR ?= $(HOME)/Rack-SDK
endif

# Source files
SOURCES += src/plugin.cpp
SOURCES += src/GenesisMono.cpp
SOURCES += src/GenesisPoly.cpp

# Include resources in distribution
DISTRIBUTABLES += res

# Include Rack SDK makefile
include $(RACK_DIR)/plugin.mk
