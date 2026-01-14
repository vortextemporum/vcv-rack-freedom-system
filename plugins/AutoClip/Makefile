# Set RACK_DIR if not set as environment variable
ifndef RACK_DIR
RACK_DIR ?= $(HOME)/Rack-SDK
endif

SOURCES += src/plugin.cpp
SOURCES += src/AutoClip.cpp

DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk
