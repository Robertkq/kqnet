ifeq ($(OS),Windows_NT)
    SHELL := powershell
endif
.SHELLFLAGS := -NoProfile -Command 

CXX ?= g++
CXXFLAGS ?= -std=c++14 -m64

OUTPUT_DIR = out

LIB_DIR = -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64"
LIB_LINK = -lws2_32 -lwldap32 -lCrypt32 -lwsock32


INCLUDE ?= -I../include/asio/asio/include/ \
			-I../include/kqlib/include/ \
			-I../include/ \
			-Icommon/