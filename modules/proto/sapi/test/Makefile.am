noinst_PROGRAMS = tar-test

AM_CPPFLAGS = -I$(top_srcdir)/c \
		-D_GNU_SOURCE
AM_CPPFLAGS += @CARSYNC_COMMON_CFLAGS@
AM_CPPFLAGS += @PROTOBUF_CFLAGS@

AM_CXXFLAGS += -std=c++0x

tar_test_SOURCES = tar-test.cpp
tar_test_LDFLAGS = @PROTOBUF_LIBS@
tar_test_LDFLAGS += @CARSYNC_COMMON_LIBS@
tar_test_LDFLAGS += ../c/libcarsync-hu-proto-c.la

