#
# Copyright (C) 2024 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



bin-y := wiggler


wiggler-y := \
	main.o \
	cmds.o \
	cmds.hash.o \
	lsp/ \
	utils/

$(loc_build_tree)/main.o: $(loc_build_tree)/cmds.hash.h
$(loc_build_tree)/cmds.o: $(loc_build_tree)/cmds.hash.h

cmds.hash-cppflags-y := -Wno-old-style-definition

wiggler-ldlibs := \
	-ljson-c \
	-lreadline

