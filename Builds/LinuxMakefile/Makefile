# UltraDYN Linux Makefile
# This is a simplified Makefile for CI builds

CONFIG ?= Release
VST3DIR ?= build/$(CONFIG)
PLUGIN_NAME = ultraDYN

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -fPIC -Wall -Wextra
INCLUDES = -I../../Source

# Add JuceLibraryCode if it exists
ifneq ($(wildcard ../../JuceLibraryCode),)
    INCLUDES += -I../../JuceLibraryCode
endif

# JUCE path (will be set by CI)
JUCE_PATH ?= ~/JUCE

# Check for JUCE modules in multiple locations
ifneq ($(wildcard ../../modules),)
    # Local modules directory (created by CI)
    INCLUDES += -I../../modules
    INCLUDES += -I../../modules/juce_core/system
    INCLUDES += -I../../modules/juce_audio_basics
    INCLUDES += -I../../modules/juce_audio_devices
    INCLUDES += -I../../modules/juce_audio_formats
    INCLUDES += -I../../modules/juce_audio_plugin_client
    INCLUDES += -I../../modules/juce_audio_processors
    INCLUDES += -I../../modules/juce_audio_utils
    INCLUDES += -I../../modules/juce_core
    INCLUDES += -I../../modules/juce_data_structures
    INCLUDES += -I../../modules/juce_dsp
    INCLUDES += -I../../modules/juce_events
    INCLUDES += -I../../modules/juce_graphics
    INCLUDES += -I../../modules/juce_gui_basics
    INCLUDES += -I../../modules/juce_gui_extra
else ifneq ($(wildcard $(JUCE_PATH)/modules),)
    # JUCE_PATH modules directory
    INCLUDES += -I$(JUCE_PATH)/modules
    INCLUDES += -I$(JUCE_PATH)/modules/juce_core/system
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_basics
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_devices
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_formats
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_plugin_client
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_processors
    INCLUDES += -I$(JUCE_PATH)/modules/juce_audio_utils
    INCLUDES += -I$(JUCE_PATH)/modules/juce_core
    INCLUDES += -I$(JUCE_PATH)/modules/juce_data_structures
    INCLUDES += -I$(JUCE_PATH)/modules/juce_dsp
    INCLUDES += -I$(JUCE_PATH)/modules/juce_events
    INCLUDES += -I$(JUCE_PATH)/modules/juce_graphics
    INCLUDES += -I$(JUCE_PATH)/modules/juce_gui_basics
    INCLUDES += -I$(JUCE_PATH)/modules/juce_gui_extra
else
    $(error JUCE modules not found. Please ensure JUCE is properly downloaded.)
endif

# Add global header define
CXXFLAGS += -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_DISPLAY_SPLASH_SCREEN=0 -DJUCE_REPORT_APP_USAGE=0 -DJUCE_STRICT_REFCOUNTEDPOINTER=1 -DJUCE_USER_MODULE_SETTINGS_INCLUDED=1

# Configuration-specific flags
ifeq ($(CONFIG),Debug)
    CXXFLAGS += -g -O0 -DDEBUG
else
    CXXFLAGS += -O3 -DNDEBUG
endif

# Source files
SOURCES = ../../Source/PluginProcessor.cpp ../../Source/PluginEditor.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Targets
VST3_TARGET = $(VST3DIR)/$(PLUGIN_NAME).so
STANDALONE_TARGET = $(VST3DIR)/$(PLUGIN_NAME)

# Default target
all: $(VST3_TARGET) $(STANDALONE_TARGET)

# Create build directory
$(VST3DIR):
	mkdir -p $(VST3DIR)

# Build VST3 plugin
$(VST3_TARGET): $(VST3DIR) $(OBJECTS)
	$(CXX) -shared -o $@ $(OBJECTS) $(LDFLAGS) -lasound -lpthread -ldl
	@echo "Built VST3 plugin: $@"

# Build standalone application
$(STANDALONE_TARGET): $(VST3DIR) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) -lX11 -lXext -lXinerama -lasound -lpthread -ldl
	@echo "Built standalone app: $@"

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# VST3 only target
vst3: $(VST3_TARGET)

# Standalone only target
standalone: $(STANDALONE_TARGET)

# Clean target
clean:
	rm -rf build/
	rm -f ../../Source/*.o

# Install target (placeholder)
install:
	@echo "Install target not implemented for CI"

# Debug target
debug:
	@echo "Configuration: $(CONFIG)"
	@echo "VST3DIR: $(VST3DIR)"
	@echo "JUCE_PATH: $(JUCE_PATH)"
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Targets: $(VST3_TARGET) $(STANDALONE_TARGET)"

.PHONY: all vst3 standalone clean install debug
