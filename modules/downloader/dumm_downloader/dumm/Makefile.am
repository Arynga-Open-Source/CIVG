###########################################################################
# Copyright 2013 Open-RnD Sp. zo.o.
#
# __OPEN-RND_COPYRIGHTS_HERE__
#
#
# AUTHOR      : Bartlomiej Jozwiak (bj@open-rnd.pl)
# DATE        : 22-03-2013
# BRIEF       : __BRIEF__
# DESCRIPTION :
#              __DESCRIPTION__
#
###########################################################################
SUBDIRS = . tests

include Includes.am

bin_PROGRAMS = dumm dummget

LDADD =

BUILT_SOURCES = include/dumm-dm-c-stub.h include/dumm-download-c-stub.h

if !QNX_BUILD
LDADD += -lrt
LDADD += -lpthread
endif
LDADD += @GLIB_LIBS@
LDADD += @DBUS_GLIB_LIBS@
LDADD += @CURL_LIBS@
LDADD += @SQLITE3_LIBS@

gdbus_srcs = gdbus/mainloop.c  gdbus/object.c  gdbus/polkit.c  gdbus/watch.c gdbus/client.c
gdbus_headers = gdbus/gdbus.h

headers =
headers += src/dumm-cm.h
headers += src/dumm-dm.h
headers += src/dumm-um.h

headers += src/dumm.h

headers += src/dumm-conf.h
headers += src/dumm-errors.h
headers += src/dumm-logger.h
headers += src/dumm-dbus.h
headers += src/dumm-proxy.h
headers += src/impl_db.h
headers += src/files.h

headers += src/dumm-db-common.h
headers += src/dumm-db-dm.h
headers += src/dumm-db-um.h

headers += src/openrnd-mutex.h
headers += src/openrnd-exceptions.h

headers += src/mgr-commoni.h
headers += src/mgr-downloadi.h
headers += src/mgr-uploadi.h
headers += src/mgr-download.h
headers += src/mgr-upload.h

headers += src/obj-common.h
headers += src/obj-download.h
headers += src/obj-upload.h

headers += src/curl-httpheaders.h
headers += src/curl-wrapper.h

headers += src/thbc.h

headers += $(gdbus_headers)

dumm_SOURCES =  $(headers)
dumm_SOURCES += src/main.cpp

dumm_SOURCES += src/dumm-cm.cpp
dumm_SOURCES += src/dumm-dm.cpp
dumm_SOURCES += src/dumm-um.cpp

dumm_SOURCES += src/dumm-conf.cpp
dumm_SOURCES += src/dumm-errors.cpp
dumm_SOURCES += src/dumm-logger.cpp
dumm_SOURCES += src/dumm-dbus.cpp
dumm_SOURCES += src/dumm-proxy.cpp
dumm_SOURCES += src/impl_db.cpp
dumm_SOURCES += src/files.cpp

dumm_SOURCES += src/openrnd-mutex.cpp
dumm_SOURCES += src/openrnd-exceptions.cpp

dumm_SOURCES += src/dumm-db-common.cpp
dumm_SOURCES += src/dumm-db-dm.cpp
dumm_SOURCES += src/dumm-db-um.cpp

dumm_SOURCES += src/mgr-download.cpp
dumm_SOURCES += src/mgr-upload.cpp

dumm_SOURCES += src/obj-common.cpp
dumm_SOURCES += src/obj-download.cpp
dumm_SOURCES += src/obj-upload.cpp

dumm_SOURCES += src/curl-httpheaders.cpp
dumm_SOURCES += src/curl-wrapper.cpp
dumm_SOURCES += src/thbc.cpp
dumm_SOURCES += $(gdbus_srcs)

dummget_SOURCES = client/client-main.c \
		  client/client-dumm-bus.c \
		  include/dumm_dm-c-stub.h
dummget_CFLAGS = -std=gnu99 -D_GNU_SOURCE -I$(top_srcdir)/xml

include_HEADERS = $(BUILT_SOURCES)

include/dumm-dm-c-stub.h: xml/net.dumm.download.dm.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=dumm_dm_object --mode=glib-client --output=$@ $(srcdir)/$<
include/dumm-download-c-stub.h: xml/net.dumm.download.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=dumm_dm_object --mode=glib-client --output=$@ $(srcdir)/$<

GIT_REV = $(shell (git describe --long --tags 2>/dev/null || echo "unknown") | cut -d/ -f2)
BUILD_DATE = $(shell date)
AM_CPPFLAGS += -DRCS_VER="\"$(GIT_REV)\""
AM_CPPFLAGS += -DBUILD_DATE="\"$(BUILD_DATE)\""


AM_CPPFLAGS += @GLIB_CFLAGS@
AM_CPPFLAGS += @DBUS_GLIB_CFLAGS@
AM_CPPFLAGS += @CURL_CFLAGS@
AM_CPPFLAGS += @SQLITE3_CFLAGS@

if WITH_SYSTEMD
systemdsystemunitdir = @SYSTEMDSYSTEMUNITDIR@
systemdsystemunit_DATA = others/genivi-dumm.service
endif

dbusdir = $(sysconfdir)/dbus-1/system.d
dbus_DATA = others/net.dumm.download.conf \
	    others/net.dumm.conf

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = $(BUILT_SOURCES)

