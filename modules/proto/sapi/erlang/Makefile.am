BUILT_SOURCES = southapi_pb.erl \
		southapi_pb.hrl

$(BUILT_SOURCES):
	$(PROTOC_ERLANG) $(top_srcdir)/proto/southapi.proto

CLEANFILES = $(BUILT_SOURCES)
