#
# Copyright (C) 2024 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



obj-y := \
	lexer.lex.o \
	parser.tab.o \
	symtab.o


$(loc_build_tree)/parser.tab.o: $(loc_build_tree)/lexer.lex.o
$(loc_build_tree)/lexer.lex.o: $(loc_build_tree)/parser.tab.c
$(loc_build_tree)/symtab.o: $(loc_build_tree)/parser.tab.c
