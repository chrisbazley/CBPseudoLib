# Project:   CBPseudoLib
include MakeCommon

CC = gcc
LibFile = ar

CCFlags = -I. -g -c -Wall -Wextra -pedantic -std=c99 -MMD -MP \
          -DFORTIFY -DDEBUG_OUTPUT -o $@
LibFileFlags = -rcs $@

Objects = $(addsuffix .o,$(ObjectList))

all: lib$(LibName).a

lib$(LibName).a: $(Objects)
	$(LibFile) $(LibFileFlags) $(Objects)

.SUFFIXES: .o .c
.c.o:
	${CC} $(CCFlags) -MF $*.d $<

-include $(addsuffix .d,$(ObjectList))
