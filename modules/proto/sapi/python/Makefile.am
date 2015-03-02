BUILT_SOURCES = southapi_pb2.py \
		common_pb2.py \
		releasepackage_pb2.py

%_pb2.py: $(top_srcdir)/proto/%.proto
	$(PROTOC_PYTHON) --python_out=$(PWD) -I$(top_srcdir)/proto $^

CLEANFILES = $(BUILT_SOURCES)
