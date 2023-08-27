CXX ?= g++
CXXFLAGS ?= -std=c++14

OUTPUT_DIR = out


INCLUDE ?= -I../include/asio/asio/include/ \
			-I../include/kqlib/include/ \
			-I../include/ \
			-Icommon/