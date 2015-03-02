# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential

BUILT_SOURCES = southapi.pb-c.c \
		southapi.pb-c.h \
		common.pb-c.c \
		common.pb-c.h \
		releasepackage.pb-c.c \
		releasepackage.pb-c.h

%.pb-c.h %.pb-c.c: $(top_srcdir)/proto/%.proto
	$(PROTOC_C) --c_out=$(PWD) -I$(top_srcdir)/proto $^

lib_LTLIBRARIES = libcarsync-hu-proto-c.la

includecarsyncdir = $(includedir)/carsync
includecarsyncprotodir = $(includecarsyncdir)/proto

includecarsyncproto_HEADERS = $(filter %.h, $(BUILT_SOURCES))

includecarsync_HEADERS = carsync-hu-south-proto.h \
                         carsync-hu-south-proto-utils.h

libcarsync_hu_proto_c_la_SOURCES = $(BUILT_SOURCES) \
                                        utils.c

AM_CFLAGS = @CARSYNC_COMMON_CFLAGS@

libcarsync_hu_proto_c_la_LDFLAGS = -lz
libcarsync_hu_proto_c_la_LDFLAGS += @CARSYNC_COMMON_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = carsync-hu-proto-c.pc

CLEANFILES = $(BUILT_SOURCES)
