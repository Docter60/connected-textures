SHELL := /bin/sh
CFLAGS := -g -rdynamic -std=c++17 -Wall -MMD -c
LDLIBS := -lpng -lz -lm -lpthread
SRCDIR := src
OBJDIR := obj
INCDIR := -I$(SRCDIR)
SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
OBJECTS := $(SOURCES:$(SRCDIR)%.cpp=$(OBJDIR)%.o)
DEPS := $(OBJECTS:%.o=%.d)
EXECUTABLE := ct
HIDE := @

.PHONY: all clean test

all: $(EXECUTABLE)
	$(HIDE)echo Done.

$(EXECUTABLE): $(OBJECTS)
	$(HIDE)echo Linking $@
	$(HIDE)$(CXX) $(OBJECTS) $(LDLIBS) -o $@

-include $(DEPS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(HIDE)echo Building $(notdir $@)
	$(HIDE)mkdir -p $(dir $@)
	$(HIDE)$(CXX) $(CFLAGS) $(INCDIR) $< -o $@

clean:
	$(HIDE)echo Deleting executable
	$(HIDE)rm -f $(EXECUTABLE)
	$(HIDE)echo Deleting $(OBJDIR) directory
	$(HIDE)rm -rf $(OBJDIR)
	$(HIDE)echo Deleting default settings.txt
	$(HIDE)rm -f settings.txt
	$(HIDE)echo Done.

test:
	$(shell ./ct -t test/white128.png -b test/black128.png -o test/out128.png)