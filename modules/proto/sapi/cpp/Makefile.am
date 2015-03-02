# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

BUILT_SOURCES = southapi.pb.cc \
		southapi.pb.h \
		common.pb.cc \
		common.pb.h \
		releasepackage.pb.cc \
		releasepackage.pb.h

%.pb.cc %.pb.h: $(top_srcdir)/proto/%.proto
	$(PROTOC_CPP) --cpp_out=$(PWD) -I$(top_srcdir)/proto $^

lib_LTLIBRARIES = libcarsync-hu-proto-cpp.la

includecarsyncdir = $(includedir)/carsync
includecarsyncprotodir = $(includecarsyncdir)/proto

includecarsyncproto_HEADERS = $(filter %.h, $(BUILT_SOURCES))

includecarsync_HEADERS = carsync-hu-south-proto.hpp \
			 carsync-hu-south-proto-utils.hpp


libcarsync_hu_proto_cpp_la_SOURCES = $(BUILT_SOURCES) \
					utils.cpp
libcarsync_hu_proto_cpp_la_LDFLAGS =

AM_CPPFLAGS = @CARSYNC_COMMON_CFLAGS@

libcarsync_hu_proto_cpp_la_LDFLAGS += @CARSYNC_COMMON_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = carsync-hu-proto-cpp.pc

CLEANFILES = $(BUILT_SOURCES)
