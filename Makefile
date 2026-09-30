all := libnd-core

LDLIBS-libnd-core := -lxylem

CFLAGS += -I$(shell cd .. && pwd)/axil-nd/include

FOLDER := nd

-include ./../mk/include.mk