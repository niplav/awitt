PREFIX=/usr/local

CC=cc
AR=ar
RANLIB=ranlib

CFLAGS=-Wall -Wextra -s -std=c89 -pedantic -O2
LDFLAGS=-lxcb -lxcb-util

BIN=awitt
OBJ=$(BIN:=.o)
MAN=$(BIN:=.1)
CONF=config.mk
