PREFIX=/usr/local

CC=cc
AR=ar
RANLIB=ranlib

CFLAGS=-Wall -Wextra -std=c89 -pedantic -O2
LDFLAGS=-s -lxcb -lxcb-util

BIN=awitt
OBJ=$(BIN:=.o)
MAN=$(BIN:=.1)
CONF=config.mk
