#
# Copyright (C) 2024 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## init
####

# build system variables
scripts_dir := scripts/build
project_type := c
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Pconfig
use_coverage_sys = n
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/
src_dirs := dtsls/ wiggler/

# include build system Makefile
include $(scripts_dir)/main.make


####
## flags
####

warnflags := \
	-Wall \
	-Wno-deprecated-declarations \
	-Wno-unused-function

cflags := \
	$(CFLAGS) \
	$(CONFIG_CFLAGS) \
	$(warnflags) \
	-std=c2x \
	-flto=auto \
	-O2

# some library functions are reported as undefined when
# compiling with -std=c2x and without the following flags
cflags += \
	-D_XOPEN_SOURCE=500 \
	-D_POSIX_C_SOURCE=200809L

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	-Iinclude \
	-I$(build_tree)

ldflags := \
	$(LDFLAGS) \
	$(CONFIG_LDFLAGS)

ldlibs := \
	$(LDLIOBSFLAGS) \
	$(CONFIG_LDLIBS)

hostcflags := \
	-flto=auto


####
## targets
####

## build
.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g -O0
all: hostcflags += -g -O0
all: hostcxxflags += -g -O0
endif

all: $(lib) $(bin) $(hostlib) $(hostbin)

## cleanup
.PHONY: clean
clean:
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(build_tree)

## install
include $(scripts_dir)/install.make

.PHONY: install
install: all
	$(call install,$(build_tree)/dtsls/dtsls)

.PHONY: uninstall
uninstall:
	$(call uninstall,$(PREFIX)/dtsls)
