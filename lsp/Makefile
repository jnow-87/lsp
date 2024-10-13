#
# Copyright (C) 2024 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := \
	cmds.hash.o \
	datatypes.o \
	protocol.o


$(loc_build_tree)/protocol.o: $(loc_build_tree)/cmds.hash.h

cmds.hash-cppflags-y := -Wno-old-style-definition
