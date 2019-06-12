PREFIX=/usr/local

CC=cc
AR=ar
RANLIB=ranlib

CFLAGS=-Wall -Wextra -std=c99 -pedantic -g
LDFLAGS=-lX11

BIN=awitt2
OBJ=$(BIN:=.o)
MAN=$(BIN:=.1)
CONF=config.mk
