# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <opentracer_gmail...> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return.         Josef Kubin
# ----------------------------------------------------------------------------

CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -g -c
INCLUDE = .
MACROS =
CSRC = $(wildcard *.c)
#OBJS = $(patsubst %.c, %.o, $(CSRC))
PROJECT = $(notdir $(shell pwd))

.SUFFIXES:


#:all	build all
.PHONY: all
all: $(PROJECT)

$(PROJECT): $(patsubst %.c, %.o, $(CSRC))
	$(LD) $(LDFLAGS) $^ -o $@
	@ctags --extra=+f $(CSRC) $(shell sed -n '/:$$/{s///;p}' *.d | sort -u)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -I$(INCLUDE) $(MACROS) -o $@ $<
	@gcc -I$(INCLUDE) $(MACROS) -MM -MP -MF $*.d $<

-include $(CSRC:.c=.d)


#:cl/clean	removes generated files
.PHONY: clean cl
clean cl:
	$(RM) $(PROJECT) tags *.{d,o,bin,hex}


# how to print color text on a terminal
# $ man 4 console_codes
# $ echo -e '\e[1;40;38;5;82mColored!\e[m'
# $ echo -e '\x1b[1;40;38;5;82mColored!\x1b[m'		<--- use for sed
#:h/help	prints this text
.PHONY: help h
help h:
	@sed -n '/^#:/{s//\x1b[1;40;38;5;82mmk /;s/\t/\x1b[m /;p}' Makefile | sort

