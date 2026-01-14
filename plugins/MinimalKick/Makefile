# Set RACK_DIR if not set as environment variable
ifndef RACK_DIR
RACK_DIR ?= $(HOME)/Rack-SDK
endif

# Specify source files
SOURCES += src/plugin.cpp
SOURCES += src/MinimalKick.cpp

# Include resources in distribution
DISTRIBUTABLES += res

# Include Rack SDK makefile
include $(RACK_DIR)/plugin.mk
